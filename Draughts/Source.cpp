#include "Piece.h"
#include <random>

void createMove(QG::Asset* asset, mouseAction action);

class boardSquare : public QG::cube
{
public:
	int x, y;
	boardSquare(int X, int Y) : x(X), y(Y)
	{
		setScale(QM::vector<3>(0.5f, 0.2f, 0.5f));
		OnClick = createMove;
	}
};

struct move
{
	QM::vector<3> startPos;
	QM::vector<3> targetPos;
	float a = 0, b = 0, c = 0, d = 0, endTime = 0;
};
move CurrentMove;
bool moving = false;
bool mustTake = false;
Piece* captured = nullptr;
col turn = col::dark;

std::vector<boardSquare*> board;

std::vector<Piece*> white;
std::vector<Piece*> black;

void initBoard();
void destroyBoard();

void initPieces();
void destroyPieces();

void endGame();

void swapTurn();

void autoMove();

std::shared_ptr<QG::Material> lightWood;
std::shared_ptr<QG::Material> darkWood;

std::shared_ptr<QG::Font> font;
std::shared_ptr<QG::Text> message;
std::shared_ptr<QG::Text> message2;
float messageEndTime = 0.0f;

int main()
{
	QG::window win(/*1600, 900, */"Draughts");
	win.setBackColour(QG::Colour(0.0f,0.0f,0.3f));
	win.cam->setPosition(QM::vector<3>(0.0f, 1.5f, 3.15f));
	win.cam->setPitch(-70.0f);
	win.disableCamPan();

	QG::pointLight light(QM::vector<3>(0.0f, 0.0f, 0.0f));

	lightWood = std::make_shared<QG::Material>("../wood.jpg", "../black.png", 1.0f);
	darkWood = std::make_shared<QG::Material>("../wood2.png", "../black.png", 1.0f);

	font = std::make_shared<QG::Font>("C:/Windows/Fonts/ARLRDBD.TTF");
	message = std::make_shared<QG::Text>();
	message->setFont(font.get());
	message->hide();
	message->setColour(WHITE);
	message->setScale(0.7f);
	message->setPosition(QM::vector<2>(1250,700));
	message2 = std::make_shared<QG::Text>();
	message2->setFont(font.get());
	message2->hide();
	message2->setColour(WHITE);
	message2->setScale(0.7f);
	message2->setPosition(QM::vector<2>(1250,650));


	initBoard();

	initMaterials();
	initPieces();

	while (win.running())
	{
		if (moving)
		{
			if (win.runtime() >= CurrentMove.endTime)
			{
				bool repeatMove = false;
				selected->setPosition(CurrentMove.targetPos);
				if (selected->y == 1 || selected->y == 8)
					selected->kingMe();

				if (captured)
				{
					for (auto it = white.begin(); it != white.end(); it++)
						if (*it == captured)
						{
							delete* it;
							white.erase(it);
							break;
						}
					for (auto it = black.begin(); it != black.end(); it++)
						if (*it == captured)
						{
							delete* it;
							black.erase(it);
							break;
						}

					repeatMove = !selected->createMoveList(true).empty();

					captured = nullptr;
				}
				if (!repeatMove)
				{
					selected->unselect();
					swapTurn();
				}
				else
					mustTake = true;
				moving = false;

			}
			else
			{
				float time = 1.0f - (CurrentMove.endTime - win.runtime()) / 2.0f;
				QM::vector<3> D = time * (CurrentMove.targetPos - CurrentMove.startPos) + CurrentMove.startPos;
				float X = D.get(1);
				float Z = D.get(3);
				float Y = (-pow(X - CurrentMove.b, 2) - pow(Z - CurrentMove.c, 2) + CurrentMove.d) / CurrentMove.a;
				selected->setPosition(QM::vector<3>(X, Y, Z));
			}
		}
		if (turn == col::light && !moving)
			autoMove();

		if (win.runtime() > messageEndTime)
		{
			message->hide();
			message2->hide();
			message->setString("");
			message2->setString("");
		}
		else
		{
			message->show();
			message2->show();
		}
		win.endFrame();
	}

	destroyPieces();

	return 0;
}

void initBoard()
{
	QM::vector<3> position(-3.5f, -5.0f, -3.5f);
	int x = 1;
	int y = 1;
	for (int i = 0; i < 64; i++)
	{
		board.push_back(new boardSquare(x, 9 - y));
		board[i]->setPosition(position);

		position += QM::vector<3>(1.0f, 0.0f, 0.0f);
		if (position.get(1) > 3.5f)
			position += QM::vector<3>(-8.0f, 0.0f, 1.0f);
		x++;
		if (x > 8)
		{
			x = 1; y++;
		}
	}

	for (auto& sq : board)
		if ((sq->x + sq->y) % 2 == 0)
			sq->setMaterial(lightWood.get());
		else
			sq->setMaterial(darkWood.get());
}

void destroyBoard()
{
	for (auto& x : board)
		delete(x);
	board.clear();
}

void initPieces()
{
	int x = 1;
	int y = 1;
	for (int i = 0; i < 12; i++)
	{
		black.push_back(new Piece(col::dark, x, y));
		white.push_back(new Piece(col::light, 9 - x, 9 - y));

		x += 2;
		if (x == 9)
		{
			x = 2;
			y++;
		}
		if (x == 10)
		{
			x = 1;
			y++;
		}
	}
}

void destroyPieces()
{
	for (auto x : white)
		delete(x);
	white.clear();
	for (auto x : black)
		delete(x);
	black.clear();
}

void endGame()
{
	destroyPieces();
	destroyBoard();
	initBoard();
	initPieces();
	turn = col::dark;
}

void swapTurn()
{
	turn = turn == col::dark ? col::light : col::dark;
	mustTake = false;

	bool any_move = false;
	if (turn == col::dark)
	{
		for (auto& it : black)
		{
			if (!it->createMoveList(true).empty())
			{
				mustTake = true;
				any_move = true;
				break;
			}
			if (!it->createMoveList().empty())
			{
				any_move = true;
			}
		}
	}
	else
	{
		for (auto& it : white)
		{
			if (!it->createMoveList(true).empty())
			{
				mustTake = true;
				any_move = true;
				break;
			}
			if (!it->createMoveList().empty())
			{
				any_move = true;
			}
		}
	}

	if (!any_move)
	{
		if (turn == col::dark)
		{
			message->setString("Computer");
			message2->setString("wins!");			
		}
		else
		{
			message->setString("You win!");
		}
		message->show();
		message2->show();
		messageEndTime = QG::getWindow()->runtime() + 5.0f;
		endGame();
	}
}

void createMove(QG::Asset* asset, mouseAction action)
{
	if (action.button == mouseCode::LEFT && action.state == keyState::press && selected != NULL)
	{
		boardSquare* B = dynamic_cast<boardSquare*>(asset);

		if ((B->x + B->y) % 2 == 1)//must move to dark square
			return;
		if (abs(B->x - selected->x) != abs(B->y - selected->y))//must be diagonal
			return;
		if (abs(B->x - selected->x) > 2)//can't be further than 2 squares
			return;
		if (!(selected->isKing()) && B->y - selected->y < 0 && selected->getCol() == col::dark)//can only move forwards
			return;
		if (!(selected->isKing()) && B->y - selected->y > 0 && selected->getCol() == col::light)
			return;

		if (abs(B->x - selected->x) == 2)
		{
			int x_capture = (B->x + selected->x) / 2;
			int y_capture = (B->y + selected->y) / 2;
			if (selected->getCol() == col::light)
			{
				for (auto& it : black)
					if (it->x == x_capture && it->y == y_capture)
					{
						captured = it;
						break;
					}
			}
			if (selected->getCol() == col::dark)
			{
				for (auto& it : white)
					if (it->x == x_capture && it->y == y_capture)
					{
						captured = it;
						break;
					}
			}
			if (!captured)
				return;
		}
		if (mustTake && !captured)
		{
			message->setString("You must capture");
			message2->setString("when possible");
			message->show();
			message2->show();
			messageEndTime = QG::getWindow()->runtime() + 5.0f;
			return;
		}

		auto pos = B->getPosition();
		pos.set(2, -4.8f);
		auto start = selected->getPosition();

		CurrentMove.startPos = start;
		CurrentMove.targetPos = pos;
		CurrentMove.a = 0.5f * ((QM::vector<3>)(pos - start)).magnitude();
		QM::vector<3> temp = 0.5 * (pos + start);
		CurrentMove.b = temp.get(1);
		CurrentMove.c = temp.get(3);
		CurrentMove.d = (CurrentMove.a - 4.8f) * CurrentMove.a;

		CurrentMove.endTime = QG::getWindow()->runtime() + 2.0f;
		selected->x = B->x;
		selected->y = B->y;
		moving = true;
	}
}

void autoMove()
{
	std::vector <std::pair < QM::vector<2, int>, QM::vector<2, int>>> moveList;
	for (auto& P : white)
	{
		auto Pmoves = P->createMoveList(true);
		moveList.insert(moveList.end(), Pmoves.begin(), Pmoves.end());
	}
	if (moveList.empty())
	{
		for (auto& P : white)
		{
			auto Pmoves = P->createMoveList();
			moveList.insert(moveList.end(), Pmoves.begin(), Pmoves.end());
		}
	}

	std::default_random_engine generator;
	std::uniform_int_distribution<int> distribution(1, moveList.size());
	int index = distribution(generator) - 1;

	auto& chosenMove = moveList[index];
	auto piece = std::find_if(white.begin(), white.end(), [&chosenMove](Piece* P) {return (P->x == chosenMove.first.get(1)) && (P->y == chosenMove.first.get(2)); });
	selected = *piece;

	auto target = std::find_if(board.begin(), board.end(), [&chosenMove](boardSquare* B) {return (B->x == chosenMove.second.get(1)) && (B->y == chosenMove.second.get(2)); });
	createMove(*target, mouseAction(mouseCode::LEFT, keyState::press, modCode::NONE));
}