#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include "Tetromino.h"
#include <cstdlib> 
#include <ctime>
#include "Tile.h"
using namespace std;
using namespace TetrisVariables;

class Screen {
	sf::RenderWindow* window;
	sf::FloatRect gameBounds;
	sf::Texture blockTexture;
	vector<vector<Tile>> board; // Coordinates are [row][col]
	Tetromino* currentPiece;
	Tetromino* heldPiece;
	vector<sf::Vector2i> currentPositions;
	vector<sf::Vector2i> previewPositions;
public:
	Screen(sf::RenderWindow& window, sf::FloatRect& gameBounds, sf::Texture& blockTexture) {
		this->window = &window;
		this->gameBounds = gameBounds;
		this->blockTexture = blockTexture;
		heldPiece = nullptr;
		srand(time(NULL));

		for (int i = 0; i < NUMROWS; i++) {
			vector<Tile> row;
			for (int j = 0; j < NUMCOLS; j++) {
				row.push_back(Tile(blockTexture, gameBounds.left + j * TILESIZE, gameBounds.top + i * TILESIZE));
			}
			board.push_back(row);
		}
		spawnPiece();
	}

	void drawScreen() {
		for (int i = 0; i < NUMROWS; i++) {
			for (int j = 0; j < NUMCOLS; j++) {
				board[i][j].draw(window);
			}
		}
	}

	void spawnPiece(bool hold = false) {
		if (currentPositions.size() != 0) {
			delete currentPiece;
		}
		// Generate random piece
		int num = rand() % 7;
		if (num == 0) 
			currentPiece = new IPiece;
		else if (num == 1) 
			currentPiece = new JPiece;
		else if (num == 2) 
			currentPiece = new LPiece;
		else if (num == 3) 
			currentPiece = new OPiece;
		else if (num == 4) 
			currentPiece = new SPiece;
		else if (num == 5) 
			currentPiece = new ZPiece;
		else if (num == 6) 
			currentPiece = new TPiece;
		updateBlocks();
	}
	void holdPiece() {
		if (heldPiece == nullptr) {
			heldPiece = currentPiece;
			spawnPiece();
		}
	}
	void updateBlocks() { // Hide current tiles, update position, set new tiles
		if (currentPositions.size() != 0)
			setMovingBlocks(currentPositions);
		currentPositions = currentPiece->getPositions();
		updatePreview();
		setMovingBlocks(currentPositions, currentPiece->color);
	}

	void updatePreview() {
		if (previewPositions.size() != 0) 
			setPreviewBlocks(previewPositions); // Clear preview
		previewPositions = currentPositions;
		bool canGoDown = true;
		while (canGoDown) {
			for (sf::Vector2i& pos : previewPositions)
				if (pos.x >= NUMROWS - 1 || board[pos.x + 1][pos.y].getHasBlock())
					canGoDown = false;
			if(canGoDown)
				for (sf::Vector2i& pos : previewPositions)
						pos.x++;
		}
		sf::Color previewColor = currentPiece->color;
		previewColor.a = 120;
		setPreviewBlocks(previewPositions, previewColor);
	}
	
	void setPiece() {
		setBlocks(currentPositions);
		setMovingBlocks(currentPositions);
		updateBlocks();
		clearlines();
		spawnPiece();
	}
	void clearlines() {
		for (int i = 0; i < NUMROWS; i++) {
			if (checkLine(board[i])) {
				printLine(i);
				clearLine(i);
				setMovingBlocks(currentPositions);
				setPreviewBlocks(currentPositions);
			}
		}
	}
	void clearLine(int row) {
		for (int i = 0; i < NUMROWS; i++) {
			if (checkLine(board[i])) {
				// Move all rows above cleared row down
				for (int j = 0; j < i; j++)
					for (Tile& tile : board[j])
						tile.moveDown();
				board.erase(board.begin() + i);

				// Insert new row at top
				vector<Tile> newRow;
				for (int j = 0; j < NUMCOLS; j++) {
					newRow.push_back(Tile(blockTexture, gameBounds.left + j * TILESIZE, gameBounds.top));
				}
				board.insert(board.begin(), newRow);
			}
		}
	}
	bool checkLine(vector<Tile>& row) { // If a line has all blocks
		for (Tile& tile : row)
			if (!tile.getHasBlock())
				return false;
		return true;
	}
	void setBlocks(vector<sf::Vector2i>& positions) {
		for (sf::Vector2i& pos : positions)
			board[pos.x][pos.y].setBlock();
	}

	void setBlocks(vector<sf::Vector2i>& positions, sf::Color& color) {
		for (sf::Vector2i& pos : positions)
			board[pos.x][pos.y].setBlock(color);
	}
	void setMovingBlocks(vector<sf::Vector2i>& positions) {
		for (sf::Vector2i& pos : positions)
			board[pos.x][pos.y].setMovingBlock();
	}
	void setMovingBlocks(vector<sf::Vector2i>& positions, sf::Color& color) {
		for (sf::Vector2i& pos : positions)
			board[pos.x][pos.y].setMovingBlock(color);
	}
	void setPreviewBlocks(vector<sf::Vector2i>& positions) {
		for (sf::Vector2i& pos : positions)
			board[pos.x][pos.y].setPreviewBlock();
	}
	void setPreviewBlocks(vector<sf::Vector2i>& positions, sf::Color& color) {
		for (sf::Vector2i& pos : positions)
			board[pos.x][pos.y].setPreviewBlock(color);
	}

	void movePiece(int direction){ // 0 to move left, 1 to move down, 2 to move right
		switch (direction)
		{
		case(0):
			if (checkLeft())
				currentPiece->moveLeft();
			break;
		case(1):
			if (checkBelow())
				currentPiece->moveDown();
			else
				setPiece();
			break;
		case(2):
			if (checkRight())
				currentPiece->moveRight();
			break;
		case(3):
			if (checkBelow()) {
				movePiece(1);
				movePiece(3);
			}
			else
				setPiece();
			break;
		default:
			break;
		}

		// Hide current tiles, update position, set new tiles
		updateBlocks();
	}

	bool checkLeft() { // True if the piece can move left
		for (sf::Vector2i pos : currentPositions)
			if (pos.y <= 0 || board[pos.x][pos.y - 1].getHasBlock())
				return false;
		return true;
	}
	bool checkBelow() { // True if the piece can move down
		for (sf::Vector2i pos : currentPositions)
			if (pos.x >= NUMROWS - 1 || board[pos.x + 1][pos.y].getHasBlock())
				return false;
		return true;
	}
	bool checkRight() { // True if the piece can move right
		for (sf::Vector2i pos : currentPositions)
			if (pos.y >= NUMCOLS - 1 || board[pos.x][pos.y + 1].getHasBlock())
				return false;
		return true;
	}
	void spinPiece(bool clockwise) {
		if (clockwise)
			currentPiece->spinCW();
		else 
			currentPiece->spinCCW();
		updateBlocks();
	}
	
};