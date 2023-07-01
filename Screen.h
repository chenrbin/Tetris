#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include "Tetromino.h"
#include <cstdlib> 
#include <ctime>
#include "Tile.h"
#include "Drawing.h"
#include <map>
#include <algorithm>
using namespace std;
using namespace TetrisVariables;

class Screen {
	sf::RenderWindow* window;
	sf::FloatRect gameBounds;
	sf::Texture blockTexture;
	const int REALNUMROWS = NUMROWS + 2; // Actual number of rows. NUMROWS is the rows visible.
	float gravity, startingGravity; // Seconds between automatic movements. Smaller gravity falls faster. 0 disables gravity. 
	int totalLinesCleared, gameMode;
	vector<vector<Tile>> board; // Coordinates are [row][col]
	Tetromino* currentPiece;
	Tetromino* heldPiece;
	vector<Tetromino*> tetrominos;
	vector<sf::Vector2i> currentPositions, previewPositions;
	vector<vector<sf::Sprite>> pieceSprites, nextPieceSprites;
	vector<int> nextPieceQueue; // Stores up to the next 14 pieces. The next 5 will be shown.
	vector<sf::Sprite> heldSprite;
	sf::FloatRect holdBounds, queueBounds;
	bool hasHeld, lockTimerStarted, touchedGround, creativeMode, autoFall, gameOver;
	bool lastMoveSpin; // For checking T-spins
	sf::Clock gravityTimer, lockTimer;
	int superLockCounter;
	int comboCounter;
	map<int, float> gravityTiers;
	vector<Animation*> animations;
	// Stores back-to-back clear flag
	bool backToBack;

public:
	Screen(sf::RenderWindow& window, vector<sf::FloatRect>& gameScreenBounds, sf::Texture& blockTexture, vector<Animation*>& animations) {
		this->window = &window;
		this->gameBounds = gameScreenBounds[0];
		this->holdBounds = gameScreenBounds[1];
		this->queueBounds = gameScreenBounds[2];
		this->blockTexture = blockTexture;
		this->animations = animations;	

		totalLinesCleared = 0, comboCounter = 0, superLockCounter = 0;
		backToBack = false;
		heldPiece = nullptr;
		hasHeld = false, lockTimerStarted = false, touchedGround = false;
		creativeMode = false, autoFall = true, gameOver = false;
		gameMode = MAINMENU;
		startingGravity = DEFAULTGRAVITY; // REVIEW. May allow starting gravity to be customized
		gravity = startingGravity;
		tetrominos = { new IPiece, new JPiece, new LPiece, new OPiece, new SPiece, new ZPiece, new TPiece };
		for (int i = 0; i < tetrominos.size(); i++) {
			tetrominos[i]->setPieceCode(i); // This piece code mess ensures proper hold functionality and that the tetrominos vector element order does not matter.
			pieceSprites.push_back(tetrominos[i]->getPieceSprite(blockTexture, 0, 0, 0.8));
		}
		for (int i = 0; i < NEXTPIECECOUNT; i++) // Initialize next pieces sprites
			nextPieceSprites.push_back(tetrominos[i]->getPieceSprite(blockTexture, 0, 0, 1));
		for (int i = 0; i < GRAVITYTIERCOUNT; i++)
			gravityTiers[GRAVITYTIERLINES[i]] = GRAVITYSPEEDS[i];

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
		// Handles gravity. Disabled if autoFall is off (sandbox exclusive)
		if (gravityTimer.getElapsedTime().asSeconds() >= gravity && gravity > 0 && autoFall) {
			movePiece(1);
			gravityTimer.restart();
		}
		if (creativeMode) // Deactivates lock timer if creative mode is on
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
		// Handles super lock timer. Resets only when a new piece is dropped. 
		// Uses a frame counter instead of a timer.
		if (!checkBelow()) {
			superLockCounter++;
			if (superLockCounter > SUPERLOCKFRAMECOUNT)
				setPiece();
		}

	}
	
	// Spawn controllable tetromino. Can be pseudorandom or specified
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
					queueBounds.top + i * 2.5 * TILESIZE * SCALE + TILESIZE / 2.0, SCALE);
			}
		}
		currentPiece = tetrominos[pieceCode]->getNewPiece();
		updateBlocks();
		for (sf::Vector2i& pos : currentPiece->getPositions()) {
			// Gave over is spawn position is occupied
			if (board[pos.x][pos.y].getHasBlock()) { 
				if (gameMode == SANDBOX)
					resetBoard();
				else if (gameMode == CLASSIC)
					gameOver = true;
				return;
			}
		}
		currentPiece->setPieceCode(pieceCode);
		movePiece(1);
		updateBlocks();
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
		if (gameMode != SANDBOX) // Reset gravity if not in sandbox mode
			setGravity();
		totalLinesCleared = 0;
		gravityTimer.restart();
		superLockCounter = 0;
		gameOver = false;
		spawnPiece();
	}
	
	// 0 to move left, 1 to move down, 2 to move right
	void movePiece(int direction) {
		switch (direction)
		{
		case(0):
			if (checkLeft()) {
				currentPiece->moveLeft();
				lockTimer.restart();
				lastMoveSpin = false;
			}
			break;
		case(1):
			if (checkBelow()) {
				currentPiece->moveDown();
				lockTimer.restart();
				lastMoveSpin = false;
			}
			break;
		case(2):
			if (checkRight()) {
				currentPiece->moveRight();
				lockTimer.restart();
				lastMoveSpin = false;
			}
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
				lastMoveSpin = true;
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
		heldSprite = heldPiece->getPieceSprite(blockTexture, holdBounds.left + TILESIZE * SCALE, holdBounds.top + TILESIZE / 2.0 * SCALE, SCALE);
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
		lockTimer.restart();
		superLockCounter = 0;
		spawnPiece();
	}
	// Check and clear any filled rows. Records number of lines cleared
	void clearLines() {
		int linesCleared = 0; // Counts amount of lines cleared by this piece for scoring
		bool hasCleared = false;
		bool isTspin = checkTspin(); // Check for t-spin before lines are cleared
		
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
				totalLinesCleared++;
				linesCleared++;
				hasCleared = true;
			}
		}
		if (hasCleared) { // Execute when lines have been cleared
			clearMovingSprites(); // Cleaning up sprite flags
			
			switch (linesCleared)
			{
			case 1:
				if (isTspin) {
					if (backToBack)
						cout << "Back-to-back\n";
					cout << "T-spin single\n";
					backToBack = true;
				}
				else {
					cout << "Single\n";
					backToBack = false;
				}
				break;
			case 2:
				if (isTspin) {
					if (backToBack)
						cout << "Back-to-back\n";
					cout << "T-spin double\n";
					backToBack = true;
				}
				else {
					cout << "Double\n";
					backToBack = false;
				}
				break; 
			case 3:
				if (isTspin) {
					if (backToBack)
						cout << "Back-to-back\n";
					cout << "T-spin triple\n";
					backToBack = true;
				}
				else {
					cout << "Triple\n";
					backToBack = false;
				}
				break;
			case 4:
				if (backToBack)
					cout << "Back-to-back\n";
				cout << "Tetris\n";
				backToBack = true;
				break;
			default:
				break;
			}
			// Records combo line clears 
			comboCounter++;
			if (comboCounter > 1)
				cout << comboCounter << "X combo\n";
			// Check for All Clear
			if (checkAllClear())
				cout << "All Clear\n";
		}
		else {
			comboCounter = 0;
		}
		if (gameMode == CLASSIC)
			checkSpeedUp();
	}
	
	// Check to increase gravity after a number of lines has been cleared. Only in classic mode.
	void checkSpeedUp() {
		auto iter = gravityTiers.begin();
		// Iterate to the next speed tier and the number of lines required to reach it
		for (; iter != gravityTiers.end() && iter->second >= gravity; iter++);
		if (iter == gravityTiers.end())
			return;
		if (totalLinesCleared >= iter->first) {
			setGravity(iter->second);
			animations[0]->restart();
		}
	}
	
#pragma region Getters/Setters
	// Set gravity to a specific speed or back to its starting speed
	void setGravity(float speed) {
		gravity = speed;
		gravityTimer.restart();
	}
	void setGravity() {
		gravity = startingGravity;
		gravityTimer.restart();
	}
	// Set game mode with a constant int code defined in TetrisConstants
	void setGameMode(const int MODE) {
		gameMode = MODE;
	}
	int getLinesCleared() {
		return totalLinesCleared;
	}
	float getGravity() {
		return gravity;
	}
	bool getGameOver() {
		return gameOver;
	}
#pragma endregion

#pragma region Sandbox Functionality
	void setAutoFall(bool value) {
		autoFall = value;
	}
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
	// True if the piece can move up
	bool checkUp() {
		for (sf::Vector2i& pos : currentPositions)
			if (pos.x <= 0 || board[pos.x - 1][pos.y].getHasBlock())
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
	// Return true if the board is cleared. Only checks if bottom row is empty.
	bool checkAllClear() {
		for (Tile tile : board[REALNUMROWS - 1])
			if (tile.getHasBlock())
				return false;
		return true;
	}
	
	// Return true if a t-spin has been achieved. 
	bool checkTspin(){
		if (currentPiece->getColor() != TPIECECOLOR || !lastMoveSpin) // Skip the corner checks if first two conditions fail
			return false;
		int cornerBlockCount = 0;
		int row = currentPositions[3].x, col = currentPositions[3].y; // Center square position
		
		// Check if corners from the center position are occupied
		if (col >= NUMCOLS - 1 || row >= REALNUMROWS - 1 || board[row + 1][col + 1].getHasBlock())
			cornerBlockCount++;
		if (col <= 0 || row <= 0 || board[row - 1][col - 1].getHasBlock())
			cornerBlockCount++;
		if (col >= NUMCOLS - 1 || row <= 0 || board[row - 1][col + 1].getHasBlock())
			cornerBlockCount++;
		if (col <= 0 || row >= REALNUMROWS - 1 || board[row + 1][col - 1].getHasBlock())
			cornerBlockCount++;

		return cornerBlockCount >= 3;
	}
	// Pause for a specified amount of time. Has hard limit of 10 seconds to avoid possible bugs
	void wait(float seconds) {
		sf::Clock cooldown;
		while (cooldown.getElapsedTime().asSeconds() < seconds && cooldown.getElapsedTime().asSeconds() < 10)
			continue;
	}
};