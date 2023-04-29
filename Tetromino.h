#pragma once
#include "TetrisConstants.h"
#include <vector>
#include <SFML/Graphics.hpp>


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
	int pieceCode; // Between 0 and 6

	Tetromino(int pieceCode = -1) {
		centerPos = sf::Vector2i(1, 4); // Initial starting position for center
		orientation = 0;
		this->pieceCode = pieceCode;
		somethingBelow = false, somethingLeft = false, somethingRight = false;
		positions = { &block1Pos, &block2Pos, &block3Pos, &centerPos };
	}
	virtual ~Tetromino() {}
	virtual void setPieceCode(int num) {
		pieceCode = num;
	}
	virtual int getPieceCode() {
		return pieceCode;
	}
	virtual Tetromino* getNewPiece() = 0;
	vector<sf::Sprite> getPieceSprite(sf::Texture& texture, int xPos, int yPos, float scaleFactor) {
		vector<sf::Sprite> sprites;
		for (sf::Vector2i* pos : positions) {
			sf::Sprite sprite;
			sprite.setTexture(texture);
			sprite.setPosition(xPos + (pos->y - 3) * TILESIZE * scaleFactor, yPos + pos->x * TILESIZE * scaleFactor);
			sprite.setScale(scaleFactor, scaleFactor);
			sprite.setColor(color);
			sprites.push_back(sprite);
		}
		return sprites;
	}
	sf::Color getColor() {
		return color;
	}
	// Set absulte positions (row, col) for the piece
	void setPositions(vector<sf::Vector2i>& newPositions) {
		for (int i = 0; i < 4; i++)
			*positions[i] = newPositions[i];
	}

	// Return a vector copy of the four block coordinates [row][col]
	vector<sf::Vector2i> getPositions() { 
		return vector<sf::Vector2i>{block1Pos, block2Pos, block3Pos, centerPos};
	}
	// Spins clockwise and returns kick test positions for SZTLJ pieces
	virtual vector<vector<sf::Vector2i>> spinCW() {
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
		// checkBounds();
		vector<sf::Vector2i> shiftValues; // There are in the format (col, row). Remember to flip later. (1, 2) means one right two down
		if (orientation == 0)
			shiftValues = { sf::Vector2i(0, 0), sf::Vector2i(-1, 0), sf::Vector2i(-1, -1), sf::Vector2i(0, 2), sf::Vector2i(-1, 2) };
		else if (orientation == 1)
			shiftValues = { sf::Vector2i(0, 0), sf::Vector2i(1, 0), sf::Vector2i(1, 1), sf::Vector2i(0, -2), sf::Vector2i(1, -2) };
		else if (orientation == 2)
			shiftValues = { sf::Vector2i(0, 0), sf::Vector2i(1, 0), sf::Vector2i(1, -1), sf::Vector2i(0, 2), sf::Vector2i(1, 2) };
		orientation++;
		if (orientation >= 4) {
			shiftValues = { sf::Vector2i(0, 0), sf::Vector2i(-1, 0), sf::Vector2i(-1, 1), sf::Vector2i(0, -2), sf::Vector2i(-1, -2) };
			orientation = 0;
		}
		vector<vector<sf::Vector2i>> shiftedPositions;
		for (sf::Vector2i shift : shiftValues)
			shiftedPositions.push_back(getShiftedPositions(shift.y, shift.x));
		return shiftedPositions;
	}
	virtual vector<vector<sf::Vector2i>> spinCCW() { // Spins counterclockwise
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
		// checkBounds();
		vector<sf::Vector2i> shiftValues; // There are in the format (col, row). Remember to flip later.
		if (orientation == 1)
			shiftValues = { sf::Vector2i(0, 0), sf::Vector2i(1, 0), sf::Vector2i(1, 1), sf::Vector2i(0, -2), sf::Vector2i(1, -2) };
		else if (orientation == 2)
			shiftValues = { sf::Vector2i(0, 0), sf::Vector2i(-1, 0), sf::Vector2i(-1, -1), sf::Vector2i(0, 2), sf::Vector2i(-1, 2) };
		else if (orientation == 3)
			shiftValues = { sf::Vector2i(0, 0), sf::Vector2i(-1, 0), sf::Vector2i(-1, 1), sf::Vector2i(0, -2), sf::Vector2i(-1, -2) };
		orientation--;
		if (orientation <= -1) {
			shiftValues = { sf::Vector2i(0, 0), sf::Vector2i(1, 0), sf::Vector2i(1, -1), sf::Vector2i(0, 2), sf::Vector2i(1, 2) };
			orientation = 3;
		}
		vector<vector<sf::Vector2i>> shiftedPositions;
		for (sf::Vector2i shift : shiftValues)
			shiftedPositions.push_back(getShiftedPositions(shift.y, shift.x));
		return shiftedPositions;
	}

	vector<sf::Vector2i> getShiftedPositions(int row, int col) {
		vector<sf::Vector2i> newPositions;
		for (sf::Vector2i* pos : positions)
			newPositions.push_back(sf::Vector2i(pos->x + row, pos->y + col));
		return newPositions;
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

};

class IPiece : public Tetromino {
public:
	IPiece() { // 1 > C > 2 > 3
		block1Pos = sf::Vector2i(centerPos.x, centerPos.y - 1);
		block2Pos = sf::Vector2i(centerPos.x, centerPos.y + 1);
		block3Pos = sf::Vector2i(centerPos.x, centerPos.y + 2);
		color = CYAN;
	}
	Tetromino* getNewPiece() {
		return new IPiece;
	}
	vector<vector<sf::Vector2i>> spinCW() { // Special case, center changes. Notation document which orientation to change to
		vector<sf::Vector2i> shiftValues; // There are in the format (col, row). Remember to flip later.
		if (orientation == 0) { // 1 v C v 2 v 3
			centerPos.y++;
			block1Pos = sf::Vector2i(centerPos.x - 1, centerPos.y);
			block2Pos = sf::Vector2i(centerPos.x + 1, centerPos.y);
			block3Pos = sf::Vector2i(centerPos.x + 2, centerPos.y);
			orientation++;
			shiftValues = { sf::Vector2i(0, 0), sf::Vector2i(-2, 0), sf::Vector2i(1, 0), sf::Vector2i(-2, 1), sf::Vector2i(1, -2) };
		}
		else if (orientation == 1) { // 1 < C < 2 < 3
			centerPos.x++;
			block1Pos = sf::Vector2i(centerPos.x, centerPos.y + 1);
			block2Pos = sf::Vector2i(centerPos.x, centerPos.y - 1);
			block3Pos = sf::Vector2i(centerPos.x, centerPos.y - 2);
			orientation++;
			shiftValues = { sf::Vector2i(0, 0), sf::Vector2i(-1, 0), sf::Vector2i(2, 0), sf::Vector2i(-1, -2), sf::Vector2i(2, 1) };

		}
		else if (orientation == 2) { // 1 ^ C ^ 2 ^ 3
			centerPos.y--;
			block1Pos = sf::Vector2i(centerPos.x + 1, centerPos.y);
			block2Pos = sf::Vector2i(centerPos.x - 1, centerPos.y);
			block3Pos = sf::Vector2i(centerPos.x - 2, centerPos.y);
			orientation++;
			shiftValues = { sf::Vector2i(0, 0), sf::Vector2i(2, 0), sf::Vector2i(-1, 0), sf::Vector2i(2, -1), sf::Vector2i(-1, 2) };

		}
		else if (orientation == 3) { // Back to default
			centerPos.x--;
			block1Pos = sf::Vector2i(centerPos.x, centerPos.y - 1);
			block2Pos = sf::Vector2i(centerPos.x, centerPos.y + 1);
			block3Pos = sf::Vector2i(centerPos.x, centerPos.y + 2);
			orientation = 0;
			shiftValues = { sf::Vector2i(0, 0), sf::Vector2i(1, 0), sf::Vector2i(-2, 0), sf::Vector2i(1, 2), sf::Vector2i(-2, -1) };

		}
		vector<vector<sf::Vector2i>> shiftedPositions;
		for (sf::Vector2i shift : shiftValues)
			shiftedPositions.push_back(getShiftedPositions(shift.y, shift.x));
		return shiftedPositions;
	}
	vector<vector<sf::Vector2i>> spinCCW() {
		vector<sf::Vector2i> shiftValues; // There are in the format (col, row). Remember to flip later.
		if (orientation == 0) { // 1 ^ C ^ 2 ^ 3
			centerPos.x++;
			block1Pos = sf::Vector2i(centerPos.x + 1, centerPos.y);
			block2Pos = sf::Vector2i(centerPos.x - 1, centerPos.y);
			block3Pos = sf::Vector2i(centerPos.x - 2, centerPos.y);
			orientation = 3;
			shiftValues = { sf::Vector2i(0, 0), sf::Vector2i(-1, 0), sf::Vector2i(2, 0), sf::Vector2i(-1, -2), sf::Vector2i(2, 1) };

		}
		else if (orientation == 1) { // To default
			centerPos.y--;
			block1Pos = sf::Vector2i(centerPos.x, centerPos.y - 1);
			block2Pos = sf::Vector2i(centerPos.x, centerPos.y + 1);
			block3Pos = sf::Vector2i(centerPos.x, centerPos.y + 2);
			orientation--;
			shiftValues = { sf::Vector2i(0, 0), sf::Vector2i(2, 0), sf::Vector2i(-1, 0), sf::Vector2i(2, -1), sf::Vector2i(-1, 2) };
		}
		else if (orientation == 2) { // 1 v C v 2 v 3
			centerPos.x--;
			block1Pos = sf::Vector2i(centerPos.x - 1, centerPos.y);
			block2Pos = sf::Vector2i(centerPos.x + 1, centerPos.y);
			block3Pos = sf::Vector2i(centerPos.x + 2, centerPos.y);
			orientation--;	
			shiftValues = { sf::Vector2i(0, 0), sf::Vector2i(1, 0), sf::Vector2i(-2, 0), sf::Vector2i(1, 2), sf::Vector2i(-2, -1) };
		}
		else if (orientation == 3) { // 1 < C < 2 < 3
			centerPos.y++;
			block1Pos = sf::Vector2i(centerPos.x, centerPos.y + 1);
			block2Pos = sf::Vector2i(centerPos.x, centerPos.y - 1);
			block3Pos = sf::Vector2i(centerPos.x, centerPos.y - 2);
			orientation--;
			shiftValues = { sf::Vector2i(0, 0), sf::Vector2i(-2, 0), sf::Vector2i(1, 0), sf::Vector2i(-2, 1), sf::Vector2i(1, -2) };

		}
		vector<vector<sf::Vector2i>> shiftedPositions;
		for (sf::Vector2i shift : shiftValues)
			shiftedPositions.push_back(getShiftedPositions(shift.y, shift.x));
		return shiftedPositions;
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
	Tetromino* getNewPiece() {
		return new JPiece;
	}
};

class LPiece : public Tetromino { // 1 > C > 2 ^ 3
public:
	LPiece() {
		block1Pos = sf::Vector2i(centerPos.x, centerPos.y - 1);
		block2Pos = sf::Vector2i(centerPos.x, centerPos.y + 1);
		block3Pos = sf::Vector2i(centerPos.x - 1, centerPos.y + 1);
		color = ORANGE;
	}
	Tetromino* getNewPiece() {
		return new LPiece;
	}
};

class OPiece : public Tetromino {
public:
	OPiece() { // C ^ 1 > 2 > 3. O pieces do not rotate so this is arbitrary
		block1Pos = sf::Vector2i(centerPos.x - 1, centerPos.y);
		block2Pos = sf::Vector2i(centerPos.x - 1, centerPos.y + 1);
		block3Pos = sf::Vector2i(centerPos.x, centerPos.y + 1);
		color = YELLOW;
	}
	Tetromino* getNewPiece() {
		return new OPiece;
	}
	vector<vector<sf::Vector2i>> spinCW() {
		return vector<vector<sf::Vector2i>>{};
	} // Does not do anything
	vector<vector<sf::Vector2i>> spinCCW() {
		return vector<vector<sf::Vector2i>> {};
	}
};

class SPiece : public Tetromino {
public:
	SPiece() { // 1 > C ^ 2 > 3
		block1Pos = sf::Vector2i(centerPos.x, centerPos.y - 1);
		block2Pos = sf::Vector2i(centerPos.x - 1, centerPos.y);
		block3Pos = sf::Vector2i(centerPos.x - 1, centerPos.y + 1);
		color = GREEN;
	}
	Tetromino* getNewPiece() {
		return new SPiece;
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
	ZPiece* getNewPiece() {
		return new ZPiece;
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
	TPiece* getNewPiece() {
		return new TPiece;
	}
};