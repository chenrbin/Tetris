#pragma once
#include "TetrisConstants.h"
#include <vector>;


using namespace std;
using namespace TetrisVariables;

struct Tetromino {
	// Position notation: block numbers 1/2/3/C each followed by directions </>/^/v,
	// For example, an upright L piece has center at the middle of the long bar
	// with notation 1 v C v 2 > 3.
	// Default Z block has notation 1 > 2 v C > 3, turn clockwise and it's 1 v 2 < C v 3

	// NOTE: The vector's x is the row and y is the column. 
	sf::Vector2i block1Pos, block2Pos, block3Pos, centerPos;
	vector<sf::Vector2i*> positions; // Pointers for easy iteration
	sf::Color color;
	bool somethingBelow, somethingLeft, somethingRight;
	int orientation; // Between 0 and 3

	Tetromino() {
		centerPos = sf::Vector2i(1, 4); // Initial starting position for center
		orientation = 0;
		somethingBelow = false, somethingLeft = false, somethingRight = false;
		positions.push_back(&block1Pos);
		positions.push_back(&block2Pos);
		positions.push_back(&block3Pos);
		positions.push_back(&centerPos);
	}
	virtual ~Tetromino() {}
	virtual void spinCW() { // Spins clockwise
		int& row = centerPos.x, & col = centerPos.y;
		for (sf::Vector2i* pos : positions) {
			if (pos->x == row - 1 && pos->y == col - 1) // Top left -> top right
				*pos = sf::Vector2i(row - 1, col + 1);
			else if (pos->x == row - 1 && pos->y == col) // Top middle -> middle right
				*pos = sf::Vector2i(row, col + 1);
			else if (pos->x == row - 1 && pos->y == col + 1) // Top right, -> bottom right
				*pos = sf::Vector2i(row + 1, col + 1);
			else if (pos->x == row && pos->y == col - 1) // Middle left -> top middle
				*pos = sf::Vector2i(row - 1, col);
			else if (pos->x == row && pos->y == col + 1) // Middle right -> bottom middle
				*pos = sf::Vector2i(row + 1, col);
			else if (pos->x == row + 1 && pos->y == col - 1) // Bottom left -> top left
				*pos = sf::Vector2i(row - 1, col - 1);
			else if (pos->x == row + 1 && pos->y == col) // Bottom middle -> middle left
				*pos = sf::Vector2i(row, col - 1);
			else if (pos->x == row + 1 && pos->y == col + 1) // Bottom right -> bottom left
				*pos = sf::Vector2i(row + 1, col - 1);		
		}
		checkBounds();
		orientation++;
		if (orientation >= 4)
			orientation = 0;
	}
	virtual void spinCCW() { // Spins counterclockwise
		int &row = centerPos.x, &col = centerPos.y;
		for (sf::Vector2i* pos : positions) {
			if (pos->x == row - 1 && pos->y == col - 1) // Top left -> bottom left
				*pos = sf::Vector2i(row + 1, col - 1);
			else if (pos->x == row - 1 && pos->y == col) // Top middle -> middle left
				*pos = sf::Vector2i(row, col - 1);
			else if (pos->x == row - 1 && pos->y == col + 1) // Top right -> top left
				*pos = sf::Vector2i(row - 1, col - 1);
			else if (pos->x == row && pos->y == col - 1) // Middle left -> bottom middle
				*pos = sf::Vector2i(row + 1, col);
			else if (pos->x == row && pos->y == col + 1) // Middle right -> top middle
				*pos = sf::Vector2i(row - 1, col);
			else if (pos->x == row + 1 && pos->y == col - 1) // Bottom left -> bottom right
				*pos = sf::Vector2i(row + 1, col + 1);
			else if (pos->x == row + 1 && pos->y == col) // Bottom middle -> middle right
				*pos = sf::Vector2i(row, col + 1);
			else if (pos->x == row + 1 && pos->y == col + 1) // Bottom right -> top right
				*pos = sf::Vector2i(row - 1, col + 1);
		}
		checkBounds();
		orientation--;
		if (orientation <= -1)
			orientation = 3;
	}

	void checkBounds() { 
		// Check if the piece has rotated into the screen edge, push it back out
		for (sf::Vector2i* pos : positions) {
			if (pos->y < 0)
				moveRight();
			else if (pos->y >= NUMCOLS)
				moveLeft();
			if (pos->x >= NUMROWS)
				moveUp();
		}
	}
	void moveUp() {
		for (sf::Vector2i* pos : positions)
			pos->x--;
	}
	void moveDown() {
		for (sf::Vector2i* pos : positions)
			pos->x++;
	}
	void moveLeft() {
		for (sf::Vector2i* pos : positions)
			pos->y--;
	}
	void moveRight() {
		for (sf::Vector2i* pos : positions)
			pos->y++;
	}
	bool checkLeft() { // True if the piece can move left
		for (sf::Vector2i* pos : positions)
			if (pos->y <= 0)
				return false;
		return true;
	}
	bool checkBelow() { // True if the piece can move down
		for (sf::Vector2i* pos : positions)
			if (pos->x >= NUMROWS - 1)
				return false;
		return true;
	}
	bool checkRight() { // True if the piece can move right
		for (sf::Vector2i* pos : positions)
			if (pos->y >= NUMCOLS - 1)
				return false;
		return true;
	}

	sf::Color getColor() {
		return color;
	}

	vector<sf::Vector2i> getPositions() { // Return a vector copy of the four block coordinates [row][col]
		return vector<sf::Vector2i>{block1Pos, block2Pos, block3Pos, centerPos};
	}
};

class IPiece : public Tetromino {
public:
	IPiece() { // 1 > C > 2 > 3
		block1Pos = sf::Vector2i(centerPos.x, centerPos.y - 1);
		block2Pos = sf::Vector2i(centerPos.x, centerPos.y + 1);
		block3Pos = sf::Vector2i(centerPos.x, centerPos.y + 2);
		color = CYAN;
	}
	IPiece getNewPiece() {
		return IPiece();
	}
	void spinCW() { // Special case, center changes. Notation document which orientation to change to
		if (orientation == 0) { // 1 v C v 2 v 3
			centerPos.y++;
			block1Pos = sf::Vector2i(centerPos.x - 1, centerPos.y);
			block2Pos = sf::Vector2i(centerPos.x + 1, centerPos.y);
			block3Pos = sf::Vector2i(centerPos.x + 2, centerPos.y);
			orientation++;

		}
		else if (orientation == 1) { // 1 < C < 2 < 3
			centerPos.x++;
			block1Pos = sf::Vector2i(centerPos.x, centerPos.y + 1);
			block2Pos = sf::Vector2i(centerPos.x, centerPos.y - 1);
			block3Pos = sf::Vector2i(centerPos.x, centerPos.y - 2);
			orientation++;
			
		}
		else if (orientation == 2) { // 1 ^ C ^ 2 ^ 3
			centerPos.y--;
			block1Pos = sf::Vector2i(centerPos.x + 1, centerPos.y);
			block2Pos = sf::Vector2i(centerPos.x - 1, centerPos.y);
			block3Pos = sf::Vector2i(centerPos.x - 2, centerPos.y);
			orientation++;

		}
		else if (orientation == 3) { // Back to default
			centerPos.x--;
			block1Pos = sf::Vector2i(centerPos.x, centerPos.y - 1);
			block2Pos = sf::Vector2i(centerPos.x, centerPos.y + 1);
			block3Pos = sf::Vector2i(centerPos.x, centerPos.y + 2);
			orientation = 0;
		}
		// Check bounds twice
		checkBounds();
		checkBounds();
	}
	void spinCCW() {
		if (orientation == 0) { // 1 ^ C ^ 2 ^ 3
			centerPos.x++;
			block1Pos = sf::Vector2i(centerPos.x + 1, centerPos.y);
			block2Pos = sf::Vector2i(centerPos.x - 1, centerPos.y);
			block3Pos = sf::Vector2i(centerPos.x - 2, centerPos.y);
			orientation = 3;

		}
		else if (orientation == 1) { // To default
			centerPos.y--;
			block1Pos = sf::Vector2i(centerPos.x, centerPos.y - 1);
			block2Pos = sf::Vector2i(centerPos.x, centerPos.y + 1);
			block3Pos = sf::Vector2i(centerPos.x, centerPos.y + 2);
			orientation--;
		}
		else if (orientation == 2) { // 1 v C v 2 v 3
			centerPos.x--;
			block1Pos = sf::Vector2i(centerPos.x - 1, centerPos.y);
			block2Pos = sf::Vector2i(centerPos.x + 1, centerPos.y);
			block3Pos = sf::Vector2i(centerPos.x + 2, centerPos.y);
			orientation--;	
		}
		else if (orientation == 3) { // 1 < C < 2 < 3
			centerPos.y++;
			block1Pos = sf::Vector2i(centerPos.x, centerPos.y + 1);
			block2Pos = sf::Vector2i(centerPos.x, centerPos.y - 1);
			block3Pos = sf::Vector2i(centerPos.x, centerPos.y - 2);
			orientation--;
		}
		// Check bounds twice
		checkBounds();
		checkBounds();
	}
};

class JPiece : public Tetromino { // 1 v 2 > C > 3
public:
	JPiece() {
		block1Pos = sf::Vector2i(centerPos.x - 1, centerPos.y - 1);
		block2Pos = sf::Vector2i(centerPos.x, centerPos.y - 1);
		block3Pos = sf::Vector2i(centerPos.x, centerPos.y + 1);
		color = BLUE;
	}
	JPiece getNewPiece() {
		return JPiece();
};

class LPiece : public Tetromino { // 1 > C > 2 ^ 3
public:
	LPiece() {
		block1Pos = sf::Vector2i(centerPos.x, centerPos.y - 1);
		block2Pos = sf::Vector2i(centerPos.x, centerPos.y + 1);
		block3Pos = sf::Vector2i(centerPos.x - 1, centerPos.y + 1);
		color = ORANGE;
	}
	LPiece getNewPiece() {
		return LPiece();
};

class OPiece : public Tetromino {
public:
	OPiece() { // C ^ 1 > 2 > 3. O pieces do not rotate so this is arbitrary
		block1Pos = sf::Vector2i(centerPos.x - 1, centerPos.y);
		block2Pos = sf::Vector2i(centerPos.x - 1, centerPos.y + 1);
		block3Pos = sf::Vector2i(centerPos.x, centerPos.y + 1);
		color = YELLOW;
	}
	void spinCW() {} // Does not do anything
	void spinCCW() {}
};

class SPiece : public Tetromino {
public:
	SPiece() { // 1 > C ^ 2 > 3
		block1Pos = sf::Vector2i(centerPos.x, centerPos.y - 1);
		block2Pos = sf::Vector2i(centerPos.x - 1, centerPos.y);
		block3Pos = sf::Vector2i(centerPos.x - 1, centerPos.y + 1);
		color = GREEN;
	}
};

class ZPiece : public Tetromino {
public:
	ZPiece() { // 1 > 2 v C > 3
		block1Pos = sf::Vector2i(centerPos.x - 1, centerPos.y - 1);
		block2Pos = sf::Vector2i(centerPos.x - 1, centerPos.y);
		block3Pos = sf::Vector2i(centerPos.x, centerPos.y + 1);
		color = RED;
	}
	
};

class TPiece : public Tetromino {
public:
	TPiece() { // Single symbol pointing with center branch
		// a literal T points down with notation v. Default orientation is ^
		block1Pos = sf::Vector2i(centerPos.x, centerPos.y - 1);
		block2Pos = sf::Vector2i(centerPos.x - 1, centerPos.y);
		block3Pos = sf::Vector2i(centerPos.x, centerPos.y + 1);
		color = VIOLET;
	}
};