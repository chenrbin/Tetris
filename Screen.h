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
	const int REALNUMROWS = NUMROWS + 2; // Actual number of rows. NUMROWS is the rows visible.
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
		tetrominos = { new IPiece, new JPiece, new LPiece, new OPiece, new SPiece, new ZPiece, new TPiece };
		for (int i = 0; i < tetrominos.size(); i++) {
			tetrominos[i]->setPieceCode(i); // This piece code mess ensures proper holding and that the tetrominos vector element order does not matter.
			pieceSprites.push_back(tetrominos[i]->getPieceSprite(blockTexture, 0, 0, 0.8));	
		}
		for (int i = 0; i < NEXTPIECECOUNT; i++) // Initialize next pieces sprites
			nextPieceSprites.push_back(tetrominos[i]->getPieceSprite(blockTexture, 0, 0, 1));
		srand(time(NULL));

		// Generate board
		for (int i = 0; i < REALNUMROWS; i++) {
			vector<Tile> row;
			for (int j = 0; j < NUMCOLS; j++) {
				row.push_back(Tile(blockTexture, gameBounds.left + j * TILESIZE, gameBounds.top + (i - 2) * TILESIZE));
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
		// TODO - reset gravity timer when piece leaves ground
		// Handles gravity
		if (gravityTimer.getElapsedTime().asSeconds() >= DEFAULTGRAVITY) {
			movePiece(1);
			gravityTimer.restart();
		}

	}

	void drawScreen() {
		for (int i = 1; i < REALNUMROWS; i++) {
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
			if (nextPieceQueue.size() < 7) { // Pre-generate next pieces
				vector<int> queueBatch{ 0, 1, 2, 3, 4, 5, 6 };
				random_shuffle(queueBatch.begin(), queueBatch.end());
				for (int num : queueBatch)
					nextPieceQueue.push_back(num);
			}
			pieceCode = nextPieceQueue[0];
			nextPieceQueue.erase(nextPieceQueue.begin());
			for (int i = 0; i < nextPieceSprites.size(); i++) {
				nextPieceSprites[i] = tetrominos[nextPieceQueue[i]]->getPieceSprite(blockTexture, 
					queueBounds.left + TILESIZE * SCALE, 
					queueBounds.top + i * 2.5 * TILESIZE * SCALE, SCALE);
			}
		}
		currentPiece = tetrominos[pieceCode]->getNewPiece();
		updateBlocks();
		for (sf::Vector2i& pos : currentPiece->getPositions()) {
			if (board[pos.x][pos.y].getHasBlock()) { // Gave over is spawn position is occupied
				resetBoard();
				return;
			}
		}
		currentPiece->setPieceCode(pieceCode);
		movePiece(1);
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
			heldSprite = heldPiece->getPieceSprite(blockTexture, holdBounds.left + TILESIZE * SCALE, holdBounds.top + TILESIZE / 2 * SCALE, SCALE);
			hasHeld = true;
	}

	// Hide current moving tiles, update position, set new tiles
	void updateBlocks() { 
		if (currentPositions.size() != 0)
			setMovingBlocks(currentPositions, false);
		currentPositions = currentPiece->getPositions();
		updatePreview();
		setMovingBlocks(currentPositions, true, currentPiece->color);
	}
	void updatePreview() {
		if (previewPositions.size() != 0) 
			setPreviewBlocks(previewPositions, false); // Clear preview
		previewPositions = currentPositions;
		bool canGoDown = true;
		while (canGoDown) {
			for (sf::Vector2i& pos : previewPositions) 
				if (pos.x >= REALNUMROWS - 1 || board[pos.x + 1][pos.y].getHasBlock())
					canGoDown = false;
			if(canGoDown)
				for (sf::Vector2i& pos : previewPositions)
						pos.x++;
		}
		sf::Color previewColor = currentPiece->color;
		previewColor.a = PREVIEWTRANSPARENCY;
		setPreviewBlocks(previewPositions, true, previewColor);
	}
	
	void setPiece() {
		setBlocks(currentPositions, true);
		updateBlocks();
		clearLines();
		hasHeld = false;
		gravityTimer.restart();
		spawnPiece();
	}
	void clearLines() {
		bool hasCleared = false;
		for (int i = 0; i < REALNUMROWS; i++) {
			if (checkLine(board[i])) {
				// Move all rows above cleared row down
				for (int j = 0; j < i; j++)
					for (Tile& tile : board[j])
						tile.moveDown();
				board.erase(board.begin() + i);

				// Insert new row at top
				vector<Tile> newRow;
				for (int j = 0; j < NUMCOLS; j++) {
					newRow.push_back(Tile(blockTexture, gameBounds.left + j * TILESIZE, gameBounds.top - 2 * TILESIZE));
				}
				board.insert(board.begin(), newRow);
				hasCleared = true;
			}
		}
		if (hasCleared) {
			clearMovingSprites();
		}
	}
	// If a row is filled
	bool checkLine(vector<Tile>& row) { 
		for (Tile& tile : row)
			if (!tile.getHasBlock())
				return false;
		return true;
	}

	void setBlocks(vector<sf::Vector2i>& positions, bool value) {
		for (sf::Vector2i& pos : positions)
			board[pos.x][pos.y].setBlock(value);
	}
	void setBlocks(vector<sf::Vector2i>& positions, bool value, sf::Color& color) {
		for (sf::Vector2i& pos : positions)
			board[pos.x][pos.y].setBlock(value, color);
	}
	void setMovingBlocks(vector<sf::Vector2i>& positions, bool value) {
		for (sf::Vector2i& pos : positions)
			board[pos.x][pos.y].setMovingBlock(value);
	}
	void setMovingBlocks(vector<sf::Vector2i>& positions, bool value, sf::Color& color) {
		for (sf::Vector2i& pos : positions)
			board[pos.x][pos.y].setMovingBlock(value, color);
	}
	void setPreviewBlocks(vector<sf::Vector2i>& positions, bool value) {
		for (sf::Vector2i& pos : positions)
			board[pos.x][pos.y].setPreviewBlock(value);
	}
	void setPreviewBlocks(vector<sf::Vector2i>& positions, bool value, sf::Color& color) {
		for (sf::Vector2i& pos : positions)
			board[pos.x][pos.y].setPreviewBlock(value, color);
	}
	// Hard reset moving and preview block sprites to avoid bugs.
	void clearMovingSprites() {
		for (int i = 1; i < REALNUMROWS; i++) {
			for (int j = 0; j < NUMCOLS; j++) {
				board[i][j].setMovingBlock(false);
				board[i][j].setPreviewBlock(false);
			}
		}
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
			if (pos.x >= REALNUMROWS - 1 || board[pos.x + 1][pos.y].getHasBlock())
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
				if (pos.y < 0 || pos.y >= NUMCOLS || pos.x >= REALNUMROWS || pos.x < 0 || board[pos.x][pos.y].getHasBlock()) {
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
	void resetBoard() {
		board.clear();
		for (int i = 0; i < REALNUMROWS; i++) {
			vector<Tile> row;
			for (int j = 0; j < NUMCOLS; j++) {
				row.push_back(Tile(blockTexture, gameBounds.left + j * TILESIZE, gameBounds.top + (i - 2) * TILESIZE));
			}
			board.push_back(row);
		}
		clearMovingSprites();
		spawnPiece();
		gravityTimer.restart();
	}
	// Pause for a specified amount of time. Has hard limit of 10 seconds to avoid possible bugs
	void wait(float seconds) {
		sf::Clock cooldown;
		while (cooldown.getElapsedTime().asSeconds() < seconds && cooldown.getElapsedTime().asSeconds() < 10)
			continue;
	}
};