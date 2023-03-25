#include "Piece.h"

int nextID = 0;
Piece* selected = NULL;
extern bool moving;
extern col turn;


std::shared_ptr<QG::Material> lightBaseMat;// = std::make_shared<QG::Material>("../LightPiece.png", "../black.png", 1.0f);
std::shared_ptr<QG::Material> lightSelectMat;// = std::make_shared<QG::Material>("../LightPieceSelected.png", "../white.png", 0.75f);
std::shared_ptr<QG::Material> darkBaseMat;// = std::make_shared<QG::Material>("../darkPiece.png", "../black.png", 1.0f);
std::shared_ptr<QG::Material> darkSelectMat;// = std::make_shared<QG::Material>("../darkPieceSelected.png", "../white.png", 0.75f);

std::shared_ptr<QG::Material> lightBaseMatKing;
std::shared_ptr<QG::Material> lightSelectMatKing;
std::shared_ptr<QG::Material> darkBaseMatKing;
std::shared_ptr<QG::Material> darkSelectMatKing;

void selectPiece(QG::Asset* asset, mouseAction action)
{
	if (moving)
		return;
	if (action.button == mouseCode::LEFT && action.state == keyState::press)
	{
		Piece* P = dynamic_cast<Piece*>(asset);
		if (P->getCol() != turn)
			return;

		if (selected == P)
			P->unselect();
		else
		{
			for (auto& x : white)
				x->unselect();
			for (auto& x : black)
				x->unselect();

			if (P->getMaterial() == lightBaseMat.get())
				P->setMaterial(lightSelectMat.get());
			else if (P->getMaterial() == darkBaseMat.get())
				P->setMaterial(darkSelectMat.get());
			else if (P->getMaterial() == lightBaseMatKing.get())
				P->setMaterial(lightSelectMatKing.get());
			else if (P->getMaterial() == darkBaseMatKing.get())
				P->setMaterial(darkSelectMatKing.get());
			selected = P;
		}

		P->createMoveList();

	}
}

void initMaterials()
{
	lightBaseMat = std::make_shared<QG::Material>("../LightPiece.png", "../black.png", 1.0f);
	lightSelectMat = std::make_shared<QG::Material>("../LightPieceSelected.png", "../white.png", 0.75f);
	darkBaseMat = std::make_shared<QG::Material>("../darkPiece.png", "../black.png", 1.0f);
	darkSelectMat = std::make_shared<QG::Material>("../darkPieceSelected.png", "../white.png", 0.75f);

	lightBaseMatKing = std::make_shared<QG::Material>("../LightPieceKing.png", "../black.png", 1.0f);
	lightSelectMatKing = std::make_shared<QG::Material>("../LightPieceSelectedKing.png", "../white.png", 0.75f);
	darkBaseMatKing = std::make_shared<QG::Material>("../DarkPieceKing.png", "../black.png", 1.0f);
	darkSelectMatKing = std::make_shared<QG::Material>("../DarkPieceSelectedKing.png", "../white.png", 0.75f);
}

Piece::Piece(col colour, int xPos, int yPos) :x(xPos), y(yPos), m_col(colour), ID(nextID)
{
	nextID++;

	setRotation(90.0, 0.0, 0.0);
	setScale(QM::vector<3>(0.47f, 0.47f, 0.1f));
	QM::vector<3> position((float)x - 4.5f, -4.8f, -(float)y + 4.5f);
	setPosition(position);

	if (colour == col::light)
		setMaterial(lightBaseMat.get());
	else
		setMaterial(darkBaseMat.get());
	OnClick = selectPiece;

}

const int Piece::getID() const
{
	return ID;
}

QM::vector<2, int> Piece::boardPosition()
{
	return QM::vector<2, int>(x, y);
}

void Piece::unselect()
{
	if (getMaterial() == lightSelectMat.get())
		setMaterial(lightBaseMat.get());
	else if (getMaterial() == darkSelectMat.get())
		setMaterial(darkBaseMat.get());
	else if (getMaterial() == lightSelectMatKing.get())
		setMaterial(lightBaseMatKing.get());
	else if (getMaterial() == darkSelectMatKing.get())
		setMaterial(darkBaseMatKing.get());
	selected = nullptr;
}

void Piece::kingMe()
{
	king = true;
	if (getMaterial() == lightSelectMat.get() || getMaterial() == lightBaseMat.get())
		setMaterial(lightSelectMatKing.get());
	else if (getMaterial() == darkSelectMat.get() || getMaterial() == darkBaseMat.get())
		setMaterial(darkSelectMatKing.get());
}

std::vector<std::pair<QM::vector<2, int>, QM::vector<2, int>>> Piece::createMoveList(bool captureOnly)
{
	std::vector<std::pair<QM::vector<2, int>, QM::vector<2, int>>> moves;
	std::vector < QM::vector<2, int>> targets;
	
	//all potential one-step moves
	if (!captureOnly)
	{
		targets.push_back(QM::vector<2, int>(x - 1, y - 1));
		targets.push_back(QM::vector<2, int>(x - 1, y + 1));
		targets.push_back(QM::vector<2, int>(x + 1, y - 1));
		targets.push_back(QM::vector<2, int>(x + 1, y + 1));
	}
	
	//all potential two-step moves
	targets.push_back(QM::vector<2, int>(x - 2, y - 2));
	targets.push_back(QM::vector<2, int>(x - 2, y + 2));
	targets.push_back(QM::vector<2, int>(x + 2, y - 2));
	targets.push_back(QM::vector<2, int>(x + 2, y + 2));

	//remove any moves beyond the board
	targets.erase(std::remove_if(targets.begin(), targets.end(), [](QM::vector<2, int> V) {return V.get(1) < 1 || V.get(1) > 8 || V.get(2) < 1 || V.get(2) > 8; }),targets.end());

	//remove backwards moves if not king
	if (!king)
	{
		if (m_col == col::light)
			targets.erase(std::remove_if(targets.begin(), targets.end(), [this](QM::vector<2, int> V) {return y - V.get(2) < 0; }), targets.end());
		else if (m_col == col::dark)
			targets.erase(std::remove_if(targets.begin(), targets.end(), [this](QM::vector<2, int> V) {return y - V.get(2) > 0; }), targets.end());
	}
	//check if potential targets are occupied and jumps are over pieces
	auto it = targets.begin();
	while (it != targets.end())
	{
		auto itW = std::find_if(white.begin(), white.end(), [it](Piece* P) {return (P->x == it->get(1)) && (P->y == it->get(2)); });
		auto itB = std::find_if(black.begin(), black.end(), [it](Piece* P) {return (P->x == it->get(1)) && (P->y == it->get(2)); });

		bool captured = true;
		if (abs(it->get(1) - x) == 2)
		{
			int x_capture = (it->get(1) + x) / 2;
			int y_capture = (it->get(2) + y) / 2;
			if (m_col == col::light)
				captured = std::find_if(black.begin(), black.end(), [it, x_capture, y_capture](Piece* P) {return (P->x == x_capture) && (P->y == y_capture); }) != black.end();
			if (m_col == col::dark)
				captured = std::find_if(white.begin(), white.end(), [it, x_capture, y_capture](Piece* P) {return (P->x == x_capture) && (P->y == y_capture); }) != white.end();
		}

		if (itW != white.end() || itB != black.end() || !captured)
			it = targets.erase(it);
		else
			it++;
	}

	for (auto& V : targets)
		moves.push_back(std::make_pair(QM::vector<2, int>(x, y), V));

	return moves;
}

