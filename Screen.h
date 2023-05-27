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
	float gravity, startingGravity; // Seconds between automatic movements. Smaller gravity falls faster. 0 disables gravity. 
	int linesCleared;
	vector<vector<Tile>> board; // Coordinates are [row][col]
	Tetromino* currentPiece;
	Tetromino* heldPiece;
	vector<Tetromino*> tetrominos;
	vector<sf::Vector2i> currentPositions, previewPositions;
	vector<vector<sf::Sprite>> pieceSprites, nextPieceSprites;
	vector<int> nextPieceQueue; // Stores up to the next 14 pieces. The next 5 will be shown.
	vector<sf::Sprite> heldSprite;
	sf::FloatRect holdBounds, queueBounds;
	bool hasHeld, lockTimerStarted, touchedGround, creativeMode;
	sf::Clock gravityTimer, lockTimer;

public:
	Screen(sf::RenderWindow& window, sf::FloatRect& gameBounds, sf::Texture& blockTexture, sf::FloatRect& holdBounds, sf::FloatRect& queueBounds) {
		this->window = &window;
		this->gameBounds = gameBounds;
		this->blockTexture = blockTexture;
		linesCleared = 0;
		heldPiece = nullptr;
		hasHeld = false;
		lockTimerStarted = false;
		touchedGround = false;
		creativeMode = false;
		startingGravity = DEFAULTGRAVITY;
		gravity = startingGravity;
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
		if (creativeMode) // Deactivates lock timer and gravity if creative mode is on
			return;
		// Handles lock timer
		if (!checkBelow()) {
			touchedGround = true;
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
			if (touchedGround) // Restarts gravity timer if piece leaves ground
				gravityTimer.restart();
			lockTimerStarted = false;
			touchedGround = false;
		}
		// Handles gravity
		if (gravityTimer.getElapsedTime().asSeconds() >= gravity && gravity > 0) {
			movePiece(1);
			gravityTimer.restart();
		}

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
					queueBounds.top + i * 2.5 * TILESIZE * SCALE + TILESIZE / 2, SCALE);
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
	
	
	// 0 to move left, 1 to move down, 2 to move right
	void movePiece(int direction) {
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
	// Spin piece either counterclockwise or clockwise
	void spinPiece(bool clockwise) {
		vector<vector<sf::Vector2i>> kickTestPositions;
		if (clockwise) // Attempt spin, get possible new positions
			kickTestPositions = currentPiece->spinCW();
		else
			kickTestPositions = currentPiece->spinCCW();
		bool spinSuccessful = false;
		for (vector<sf::Vector2i>& positions : kickTestPositions) {
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
	// Can hold piece once every time a piece is set
	void holdPiece() {
		if (!hasHeld)
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
	// Sets the current piece and spawns a new piece
	void setPiece() {
		setBlocks(currentPositions, true);
		updateBlocks();
		clearLines();
		hasHeld = false;
		touchedGround = false;
		gravityTimer.restart();
		spawnPiece();
	}
	// Check and clear any filled rows. Records number of lines cleared
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
				linesCleared++;
				hasCleared = true;
			}
		}
		if (hasCleared) { // Execute when lines have been cleared
			clearMovingSprites(); // Code for cleaning up flags
		}
	}
	// Clear board and restart game. Reset gravity and piece queue
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
		nextPieceQueue.clear();
		heldPiece = nullptr;
		heldSprite.clear();
		spawnPiece();
		setGravity(startingGravity);
		gravityTimer.restart();
	}
	void setGravity(float speed) {
		gravity = speed;
		gravityTimer.restart();
	}
	int getLinesCleared() {
		return linesCleared;
	}
	float getGravity() {
		return gravity;
	}
#pragma region Sandbox Functionality
	// Turn on creative mode, disable timers and allow blocks to be placed and removed by clicking
	void startCreativeMode() {
		spawnPiece(currentPiece->getPieceCode());
		creativeMode = true;
	}
	// Turn off creative mode
	void endCreativeMode() {
		gravityTimer.restart();
		creativeMode = false;
	}
	// Sandbox mode exclusive feature. Place and remove blocks by clicking on board
	void clickBlock(sf::Vector2f& clickPos) {
		if (!creativeMode) // Function only works if creative mode is toggled on
			return;
		// Convert mouse position to game coordinates.
		int col = (clickPos.x - gameBounds.left) / TILESIZE;
		int row = (clickPos.y - gameBounds.top) / TILESIZE + 2; // Adjust to exclude the rows outside of game window

		if (!board[row][col].getHasMovingBlock()) {
			board[row][col].toggleBlock();
			updateBlocks();
		}
	}
	// Works like click box. Fills in all columns in a row other than the clicked column. 
	void clickRow(sf::Vector2f& clickPos) {
		if (!creativeMode) // Function only works if creative mode is toggled on
			return;
		// Convert mouse position to game coordinates.
		int col = (clickPos.x - gameBounds.left) / TILESIZE;
		int row = (clickPos.y - gameBounds.top) / TILESIZE + 2; // Adjust to exclude the rows outside of game window
		for (int i = 0; i < NUMCOLS; i++) {
			if (!board[row][i].getHasMovingBlock())
				board[row][i].setBlock(true, WHITE);
		}
		board[row][col].setBlock(false);
		updateBlocks();
	}
#pragma endregion

#pragma region Graphics
	void drawScreen() {
		for (int i = 1; i < REALNUMROWS; i++) {
			for (int j = 0; j < NUMCOLS; j++) {
				board[i][j].draw(window);
			}
		}
		for (sf::Sprite& sprite : heldSprite)
			window->draw(sprite);
		for (vector<sf::Sprite> pieceSprite : nextPieceSprites)
			for (sf::Sprite& sprite : pieceSprite)
				window->draw(sprite);
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
			if (canGoDown)
				for (sf::Vector2i& pos : previewPositions)
					pos.x++;
		}
		sf::Color previewColor = currentPiece->color;
		previewColor.a = PREVIEWTRANSPARENCY;
		setPreviewBlocks(previewPositions, true, previewColor);
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
	// Hard reset moving and preview block sprites to debug.
	void clearMovingSprites() {
		for (int i = 1; i < REALNUMROWS; i++) {
			for (int j = 0; j < NUMCOLS; j++) {
				board[i][j].setMovingBlock(false);
				board[i][j].setPreviewBlock(false);
			}
		}
	}
#pragma endregion

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
	// Return true if a row is filled.
	bool checkLine(vector<Tile>& row) {
		for (Tile& tile : row)
			if (!tile.getHasBlock())
				return false;
		return true;
	}

	// Pause for a specified amount of time. Has hard limit of 10 seconds to avoid possible bugs
	void wait(float seconds) {
		sf::Clock cooldown;
		while (cooldown.getElapsedTime().asSeconds() < seconds && cooldown.getElapsedTime().asSeconds() < 10)
			continue;
	}
};