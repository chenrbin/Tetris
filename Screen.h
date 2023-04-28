#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include "Tetromino.h"
#include <cstdlib> 
#include <ctime>
#include "Tile.h"
#include <algorithm>
using namespace std;
using namespace TetrisVariables;

class Screen {
	sf::RenderWindow* window;
	sf::FloatRect gameBounds;
	sf::Texture blockTexture;
	vector<vector<Tile>> board; // Coordinates are [row][col]
	Tetromino* currentPiece;
	Tetromino* heldPiece;
	vector<Tetromino*> tetrominos;
	vector<sf::Vector2i> currentPositions;
	vector<sf::Vector2i> previewPositions;
	vector<vector<sf::Sprite>> pieceSprites;
	vector<int> nextPieceQueue; // Stores up to the next 14 pieces. The next 5 will be shown.
	vector<vector<sf::Sprite>> nextPieceSprites;
	vector<sf::Sprite> heldSprite;
	sf::FloatRect holdBounds, queueBounds;
	bool hasHeld, lockTimerStarted;
	sf::Clock gravityTimer;
	sf::Clock lockTimer;


public:
	Screen(sf::RenderWindow& window, sf::FloatRect& gameBounds, sf::Texture& blockTexture, sf::FloatRect& holdBounds, sf::FloatRect& queueBounds) {
		this->window = &window;
		this->gameBounds = gameBounds;
		this->blockTexture = blockTexture;
		heldPiece = nullptr;
		hasHeld = false;
		lockTimerStarted = false;
		this->holdBounds = holdBounds;
		this->queueBounds = queueBounds;
		tetrominos = { new TPiece, new JPiece, new LPiece, new OPiece, new SPiece, new ZPiece, new TPiece };
		for (int i = 0; i < tetrominos.size(); i++) {
			tetrominos[i]->setPieceCode(i); // This piece code mess ensures proper holding and that the tetrominos vector element order does not matter.
			pieceSprites.push_back(tetrominos[i]->getPieceSprite(blockTexture, 0, 0, 0.8));	
		}
		for (int i = 0; i < 5; i++)
			nextPieceSprites.push_back(tetrominos[i]->getPieceSprite(blockTexture, 0, 0, 1));
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

	// Checked every frame. Handles timer related events
	void doTimeStuff() {
		// Handles lock timer
		if (!checkBelow()) {
			if (!lockTimerStarted) {
				lockTimer.restart();
				lockTimerStarted = true;
			}
			else {
				if (lockTimer.getElapsedTime().asSeconds() >= LOCKDELAY) {
					lockTimer.restart();
					setPiece();
				}
			}
		}
		else {
			lockTimerStarted = false;
		}
		
		// Handles gravity
		if (gravityTimer.getElapsedTime().asSeconds() >= DEFAULTGRAVITY) {
			movePiece(1);
			gravityTimer.restart();
		}

	}

	void drawScreen() {
		for (int i = 0; i < NUMROWS; i++) {
			for (int j = 0; j < NUMCOLS; j++) {
				board[i][j].draw(window);
			}
		}
		for (sf::Sprite& sprite : heldSprite)
			window->draw(sprite);
		for(vector<sf::Sprite> pieceSprite: nextPieceSprites)
			for (sf::Sprite& sprite : pieceSprite)
				window->draw(sprite);
	}

	void spawnPiece(int pieceCode = -1) {
		if (currentPositions.size() != 0) {
			delete currentPiece;
		}
		if (pieceCode == -1) { // Generate random piece
			if (nextPieceQueue.size() < 7) {
				vector<int> queueBatch{ 0, 1, 2, 3, 4, 5, 6 };
				random_shuffle(queueBatch.begin(), queueBatch.end());
				for (int num : queueBatch)
					nextPieceQueue.push_back(num);
			}
			pieceCode = nextPieceQueue[0];
			nextPieceQueue.erase(nextPieceQueue.begin());
			for (int i = 0; i < 5; i++) {
				nextPieceSprites[i] = tetrominos[nextPieceQueue[i]]->getPieceSprite(blockTexture, 
					queueBounds.left + TILESIZE * SCALE, 
					queueBounds.top + i * 2.5 * TILESIZE * SCALE + TILESIZE / 2, SCALE);
			}
		}
		currentPiece = tetrominos[pieceCode]->getNewPiece();
		currentPiece->setPieceCode(pieceCode);
		updateBlocks();
	}
	void holdPiece() {
		if(!hasHeld)
			if (heldPiece == nullptr) { // If holding for the first time
				heldPiece = currentPiece->getNewPiece();
				heldPiece->setPieceCode(currentPiece->getPieceCode());
				spawnPiece();
			}
			else {
				int temp = heldPiece->getPieceCode();
				heldPiece = currentPiece->getNewPiece();
				heldPiece->setPieceCode(currentPiece->getPieceCode());
				spawnPiece(temp);	
			}
			heldSprite = heldPiece->getPieceSprite(blockTexture, holdBounds.left + TILESIZE * SCALE, holdBounds.top + TILESIZE * SCALE, SCALE);
			hasHeld = true;
	}

	// Hide current moving tiles, update position, set new tiles
	void updateBlocks() { 
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
		hasHeld = false;
		spawnPiece();
	}
	void clearlines() {
		for (int i = 0; i < NUMROWS; i++) {
			if (checkLine(board[i])) {
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
	// If a row is filled
	bool checkLine(vector<Tile>& row) { 
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

	// 0 to move left, 1 to move down, 2 to move right
	void movePiece(int direction){ 
		switch (direction)
		{
		case(0):
			if (checkLeft())
				currentPiece->moveLeft();
			break;
		case(1):
			if (checkBelow())
				currentPiece->moveDown();
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

	// True if the piece can move left
	bool checkLeft() { 
		for (sf::Vector2i& pos : currentPositions)
			if (pos.y <= 0 || board[pos.x][pos.y - 1].getHasBlock())
				return false;
		return true;
	}
	// True if the piece can move down
	bool checkBelow() { 
		for (sf::Vector2i& pos : currentPositions)
			if (pos.x >= NUMROWS - 1 || board[pos.x + 1][pos.y].getHasBlock())
				return false;
		return true;
	}
	// True if the piece can move right
	bool checkRight() { 
		for (sf::Vector2i& pos : currentPositions)
			if (pos.y >= NUMCOLS - 1 || board[pos.x][pos.y + 1].getHasBlock())
				return false;
		return true;
	}
	void spinPiece(bool clockwise) {
		vector<vector<sf::Vector2i>> kickTestPositions;
		if (clockwise) 
			kickTestPositions = currentPiece->spinCW();
		else
			kickTestPositions = currentPiece->spinCCW();
		if (kickTestPositions.size() == 0) // Returned by spinning o piece
		{
			updateBlocks();
			return;
		}
		bool spinSuccessful = false;
		for (vector<sf::Vector2i>& positions : kickTestPositions){
			bool validPosition = true;
			for (sf::Vector2i& pos : positions) {
				if (pos.y < 0 || pos.y >= NUMCOLS || pos.x >= NUMROWS || board[pos.x][pos.y].getHasBlock()) {
					print("Spin");
					validPosition = false;
					break;
				}
			}
			if (validPosition) {
				currentPiece->setPositions(positions);
				lockTimer.restart(); // Restart lock timer. Can go on infinitely without second timer
				spinSuccessful = true;
				break;
			}
		}
		if (!spinSuccessful) // If spin fails, return piece to original position
			if (clockwise)
				currentPiece->spinCCW();
			else
				currentPiece->spinCW();
		updateBlocks();
	}
	
};