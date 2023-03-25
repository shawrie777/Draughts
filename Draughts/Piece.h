#pragma once

#include <QGraphics.h>
#include <algorithm>

extern std::shared_ptr<QG::Material> lightBaseMat;
extern std::shared_ptr<QG::Material> lightSelectMat;
extern std::shared_ptr<QG::Material> darkBaseMat;
extern std::shared_ptr<QG::Material> darkSelectMat;


enum class col
{
	light,
	dark
};

void initMaterials();

class Piece : public QG::prism<6>
{
private:
	const int ID;
	bool king = false;
	col m_col;

public:
	Piece(col colour, int xPos, int yPos);
	const int getID() const;
	QM::vector<2, int> boardPosition();
	void unselect();
	bool isKing() { return king; };
	col getCol() { return m_col; };
	void kingMe();
	int x;
	int y;

	std::vector <std::pair<QM::vector<2, int>, QM::vector<2, int>>> createMoveList(bool captureOnly = false);
};

extern std::vector<Piece*> white;
extern std::vector<Piece*> black;
extern Piece* selected;
