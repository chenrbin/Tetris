#pragma once
#include <cstdlib> 
#include <ctime>
#include <algorithm>
#include "Tetromino.h"
#include "Tile.h"

using namespace std;
using namespace TetrisVariables;

class Screen {
#pragma region Attributes
	// HUD Items
	vector<SfRectangleAtHome> screenRects;
	sf::FloatRect gameBounds, holdBounds, queueBounds;
	SfTextAtHome holdText, nextText, gamemodeText;
	vector<SfRectangleAtHome> lines;

	sf::RenderWindow* window;
	sf::Texture* blockTexture;
	float gravity, startingGravity; // Seconds between automatic movements. Smaller gravity falls faster. 0 disables gravity. 
	map<int, float> gravityTiers;

	int totalLinesCleared, gameMode, playerIndex;
	vector<vector<Tile>> board; // Coordinates are [row][col]

	Tetromino* currentPiece;
	Tetromino* heldPiece;
	vector<Tetromino*> tetrominos;
	vector<sf::Vector2i> currentPositions, previewPositions;

	vector<vector<sf::Sprite>> nextPieceSprites;
	vector<int> nextPieceQueue;
	vector<sf::Sprite> heldSprite;

	bool hasHeld, lockTimerStarted, touchedGround, creativeMode, autoFall, gameOver, paused;
	bool lastMoveSpin; // For checking T-spins
	sfClockAtHome gravityTimer, lockTimer, superLockTimer; // Timer to track gravity and locking

	int comboCounter;
	vector<FadeText> clearAnimations; // { &speedupText, &clearText, &b2bText, &comboText, &allClearText }
	bool backToBack; // Stores back-to-back clear flag
	PieceBag* bag; // Stores the random piece generation

	int inGarbage, outGarbage; // Lines of garbage to receive/send
	GarbageBin bin; // Queue for garbage inventory
	bool canDump; // Dump garbage if a piece has been set without clearing lines
	GarbageStack garbStack; // Visuals for garbage
	int garbLastCol; // Stores location of garbage for randomness settings

	DeathAnimation deathAnimation;

	// Customizable game settings
	float lockDelay, superLockDelay;
	int nextPieceCount;
	bool holdEnabled, ghostPieceEnabled, SRS_Enabled, bagEnabled;
	float garbRepeatProbability; // Probability for a guaranteed repeat garbage
	float garbageMultiplier; // Scalar for garbage exchange, rounded up
	int colorPallete;

	SoundManager* soundFX;
#pragma endregion

public:
	Screen(sf::RenderWindow& window, sf::Vector2f gamePos, sf::Font& font, sf::Texture* blockTexture, PieceBag* bag, SoundManager* soundFX) {
		this->window = &window;
		setHUD(gamePos, font);
		this->blockTexture = blockTexture;
		this->clearAnimations = clearAnimations;	// Pass animations to screen class to play when prompted
		clearAnimations.push_back(FadeText(SfTextAtHome(font, WHITE, "SPEED UP", GAMETEXTSIZE * 2, { gamePos.x + GAMEWIDTH / 2, gamePos.y }, true, false, true), 1, 1));
		clearAnimations.push_back(FadeText(SfTextAtHome(font, WHITE, "T-spin Triple", CLEARTEXTSIZE, { gamePos.x + GAMEWIDTH + LINEWIDTH * 2, gamePos.y + GAMEHEIGHT / 1.5f }), 0, 2.5f));
		clearAnimations.push_back(FadeText(SfTextAtHome(font, WHITE, "Back-to-Back", CLEARTEXTSIZE, { gamePos.x + GAMEWIDTH + LINEWIDTH * 2, gamePos.y + GAMEHEIGHT / 1.5f + MENUSPACING }), 0, 2.5f));
		clearAnimations.push_back(FadeText(SfTextAtHome(font, WHITE, "2X Combo", CLEARTEXTSIZE, { gamePos.x + GAMEWIDTH + LINEWIDTH * 2, gamePos.y + GAMEHEIGHT / 1.5f + MENUSPACING * 2 }), 0, 2.5f));
		clearAnimations.push_back(FadeText(SfTextAtHome(font, WHITE, "All Clear", CLEARTEXTSIZE, { gamePos.x + GAMEWIDTH + LINEWIDTH * 2, gamePos.y + GAMEHEIGHT / 1.5f + MENUSPACING * 3 }), 0, 2.5f));

		this->bag = bag;
		this->soundFX = soundFX;

		// Set game settings to their default values
		lockDelay = LOCKDELAY;
		superLockDelay = SUPERLOCKDELAY;
		nextPieceCount = NEXTPIECECOUNT;
		holdEnabled = true, ghostPieceEnabled = true, SRS_Enabled = true, bagEnabled = true;
		startingGravity = DEFAULTGRAVITY;
		garbRepeatProbability = DEFAULTGARBAGEPROBABILITY;
		garbageMultiplier = 1;
		colorPallete = 0;

		playerIndex = bag->addPlayer();
		totalLinesCleared = 0, comboCounter = 0;
		heldPiece = nullptr;
		hasHeld = false, lockTimerStarted = false, touchedGround = false, backToBack = false;
		creativeMode = false, autoFall = true, gameOver = false, canDump = true, paused = false;
		gameMode = MAINMENU; // Gamemode will be set whenever a mode is selected from the main menu
		gravity = startingGravity;

		tetrominos = { new IPiece, new JPiece, new LPiece, new OPiece, new SPiece, new ZPiece, new TPiece };
		for (int i = 0; i < tetrominos.size(); i++) {
			tetrominos[i]->setPieceCode(i); // Piece codes allow for specified piece spawns
			tetrominos[i]->setColor(PIECECOLORSETS[colorPallete][i]);
		}
		for (int i = 0; i < nextPieceCount; i++) // Initialize next pieces sprites
			nextPieceSprites.push_back(tetrominos[i]->getPieceSprite(*blockTexture, 0, 0, 1));
		for (int i = 0; i < GRAVITYTIERCOUNT; i++) // Initialize gravity thresholds
			gravityTiers[GRAVITYTIERLINES[i]] = GRAVITYSPEEDS[i];

		deathAnimation = DeathAnimation({ gameBounds.left, gameBounds.top }, 2, 0.2f, *blockTexture);
		garbStack = GarbageStack({ gameBounds.left, gameBounds.top });
		garbLastCol = rand() % NUMCOLS; // Random column


		// Generate board
		for (int i = 0; i < REALNUMROWS; i++) {
			vector<Tile> row;
			for (int j = 0; j < NUMCOLS; j++) {
				row.push_back(Tile(*blockTexture, gameBounds.left + j * TILESIZE, gameBounds.top + (i - 2) * TILESIZE));
			}
			board.push_back(row);
		}
		spawnPiece();
	}

	~Screen() {
		for (Tetromino* tet : tetrominos)
			delete tet;
	}
#pragma region Core Gameplay
	// Checked every frame. Handles timer related events
	void doTimeStuff() {
		// Disable if paused
		if (paused)
			return;
		// Update garbageStack if gameMode is sandbox or PVP
		if (gameMode != CLASSIC)
			garbStack.updateStack(getStackVector());
		// Handles gravity. Disabled if autoFall is off (sandbox exclusive)
		if (gravityTimer.getTimeSeconds() >= gravity && gravity > 0 && autoFall) {
			movePiece(1);
			gravityTimer.restart();
		}
		if (creativeMode) // Deactivates the rest if creative mode is on
			return;
		// Handles lock timer
		if (!checkBelow()) {
			touchedGround = true;
			if (!lockTimerStarted) {
				lockTimer.restart();
				lockTimerStarted = true;
			}
			else
				if (lockTimer.getTimeSeconds() >= lockDelay) {
					soundFX->play(MEDIUMBEEP);
					setPiece();
				}
		}
		else {
			if (touchedGround) { // Restarts gravity timer if piece leaves ground
				gravityTimer.restart();
			}
			lockTimerStarted = false;
			touchedGround = false;
		}
		// Handles super lock timer. Resets only when a new piece is dropped. 
		if (!checkBelow()) {
			superLockTimer.resume();
			if (superLockTimer.getTimeSeconds() >= superLockDelay)
				setPiece();
		}
		else
			superLockTimer.pause();
		updateGarbage(); // Garbage timers
	}
	// Spawn controllable tetromino. Can be pseudorandom or specified
	void spawnPiece(int pieceCode = -1) {
		// Disable if paused
		if (paused)
			return;
		if (currentPositions.size() != 0) { // Delete current piece if it exists
			delete currentPiece;
		}
		if (pieceCode == -1) { // Generate random piece
			if (bagEnabled) {
				pieceCode = bag->getPiece(playerIndex);
				nextPieceQueue = bag->getNextPieces(playerIndex, NEXTPIECECOUNT);
			}
			else {
				if (nextPieceQueue.size() < 7) // Replenish queue
					for (int i = 0; i < 7; i++)
						nextPieceQueue.push_back(rand() % 7);
				pieceCode = nextPieceQueue[0];
				nextPieceQueue.erase(nextPieceQueue.begin());
			}
			for (int i = 0; i < nextPieceSprites.size(); i++) { // Display queue
				nextPieceSprites[i] = tetrominos[nextPieceQueue[i]]->getPieceSprite(*blockTexture,
					queueBounds.left + TILESIZE * HUDPIECESCALE,
					queueBounds.top + i * 2.5f * TILESIZE * HUDPIECESCALE + TILESIZE / 2.0f, HUDPIECESCALE);
			}
		}
		currentPiece = tetrominos[pieceCode]->getNewPiece();
		updateBlocks();
		// Gave over is spawn position is occupied
		for (sf::Vector2i& pos : currentPiece->getPositions()) {
			if (board[pos.x][pos.y].getHasBlock()) {
				doGameOver();
				return;
			}
		}
		currentPiece->setPieceCode(pieceCode);
		movePiece(1);
		updateBlocks();
	}
	// 0 to move left, 1 to move down, 2 to move right, 3 to instant drop
	void movePiece(int direction) {
		// Disable if paused
		if (paused)
			return;
		switch (direction)
		{
		case(0):
			if (checkLeft()) {
				currentPiece->moveLeft();
				lockTimer.restart();
				lastMoveSpin = false;
				soundFX->play(LIGHTTAP);
			}
			break;
		case(1):
			if (checkBelow()) {
				currentPiece->moveDown();
				lockTimer.restart();
				gravityTimer.restart();
				lastMoveSpin = false;
			}
			break;
		case(2):
			if (checkRight()) {
				currentPiece->moveRight();
				lockTimer.restart();
				lastMoveSpin = false;
				soundFX->play(LIGHTTAP);
			}
			break;
		case(3):
			if (checkBelow()) {
				movePiece(1);
				movePiece(3);
			}
			else {
				setPiece();
				soundFX->play(HIGHBEEP);
			}
			break;
		default:
			break;
		}

		// Hide current tiles, update position, set new tiles
		updateBlocks();
	}
	// Spin piece either counterclockwise or clockwise
	void spinPiece(bool clockwise) {
		// Disable if paused
		if (paused)
			return;
		vector<vector<sf::Vector2i>> kickTestPositions;
		if (clockwise) // Attempt spin, get possible new positions
			kickTestPositions = currentPiece->spinCW();
		else
			kickTestPositions = currentPiece->spinCCW();
		bool spinSuccessful = false;
		// Find the first valid kick position
		for (vector<sf::Vector2i>& positions : kickTestPositions) {
			bool validPosition = true;
			for (sf::Vector2i& pos : positions) {
				if (pos.y < 0 || pos.y >= NUMCOLS || pos.x >= REALNUMROWS || pos.x < 0 || board[pos.x][pos.y].getHasBlock()) {
					validPosition = false;
					break;
				}
			}
			if (validPosition) { // Execute spin
				currentPiece->setPositions(positions);
				lockTimer.restart();
				spinSuccessful = true;
				lastMoveSpin = true;
				soundFX->play(LIGHTTAP);
				break;
			}
			if (!SRS_Enabled)
				break; // Break after first iteration
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
		// Do nothing if paused or disabled
		if (paused || !holdEnabled)
			return;
		if (hasHeld) // hasHeld is true after a successful hold, and false after a piece sets
			return;
		if (heldPiece == nullptr) { // If holding for the first time
			heldPiece = currentPiece->getNewPiece();
			heldPiece->setPieceCode(currentPiece->getPieceCode());
			spawnPiece();
		}
		else {
			int temp = heldPiece->getPieceCode();
			delete heldPiece;
			heldPiece = currentPiece->getNewPiece();
			heldPiece->setPieceCode(currentPiece->getPieceCode());
			spawnPiece(temp);
		}
		heldSprite = heldPiece->getPieceSprite(*blockTexture, holdBounds.left + TILESIZE * HUDPIECESCALE, holdBounds.top + TILESIZE / 2.0f * HUDPIECESCALE, HUDPIECESCALE);
		hasHeld = true;
		soundFX->play(MEDIUMBEEP);
	}
	// Sets the current piece and spawns a new piece
	void setPiece() {
		setBlocks(currentPositions, true);
		updateBlocks();
		doClearLines();
		hasHeld = false;
		touchedGround = false;
		gravityTimer.restart(), lockTimer.restart(), superLockTimer.restart();
		if (inGarbage > 0 && canDump)
			dumpGarbage(inGarbage);
		spawnPiece();
	}
	// Check and clear any filled rows. Records number of lines cleared
	void doClearLines() {
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
					newRow.push_back(Tile(*blockTexture, gameBounds.left + j * TILESIZE, gameBounds.top - 2 * TILESIZE));
				}
				board.insert(board.begin(), newRow);
				totalLinesCleared++;
				linesCleared++;
				hasCleared = true;
			}
		}
		canDump = !hasCleared; // For garbage
		if (hasCleared) { // Execute when lines have been cleared
			clearMovingSprites(); // Cleaning up sprite flags
			soundFX->pauseAll();
			soundFX->play(HIGHHIGHBEEP);

			switch (linesCleared) // Process scoring
			{
			case 1:
				if (isTspin) {
					if (backToBack) {
						playBackToBackText();
						sendGarbage(1);
					}
					playClearText("T-spin Single");
					sendGarbage(2);
					backToBack = true; // backToBack = isTspin
				}
				else {
					backToBack = false;
				}
				break;
			case 2:
				if (isTspin) {
					if (backToBack) {
						playBackToBackText();
						sendGarbage(1);
					}
					playClearText("T-spin Double");
					sendGarbage(4);
					backToBack = true;
				}
				else {
					playClearText("Double");
					sendGarbage(1);
					backToBack = false;
				}
				break;
			case 3:
				if (isTspin) {
					if (backToBack) {
						playBackToBackText();
						sendGarbage(3);
					}
					playClearText("T-spin Triple");
					sendGarbage(6);
					backToBack = true;
				}
				else {
					playClearText("Triple");
					sendGarbage(2);
					backToBack = false;
				}
				break;
			case 4:
				if (backToBack) {
					playBackToBackText();
					sendGarbage(2);
				}
				playClearText("Tetris");
				sendGarbage(4);
				backToBack = true;
				break;
			default:
				break;
			}
			// Records combo line clears 
			comboCounter++;
			// Process combos
			if (comboCounter > 1) {
				if (comboCounter > 2)
					sendGarbage(1);
				else if (comboCounter > 4)
					sendGarbage(2);
				else if (comboCounter > 6)
					sendGarbage(3);
				else if (comboCounter > 8)
					sendGarbage(4);
				else if (comboCounter > 11)
					sendGarbage(5);
				playComboText();
			}
			// Process All Clear
			if (checkAllClear()) {
				playAllClearText();
				sendGarbage(10);
			}
		}
		else {
			comboCounter = 0;
		}
		if (gameMode == CLASSIC)
			doSpeedUp();
	}
#pragma endregion

	// Pause the game and disable timers
	void pauseGame() {
		paused = true;
		gravityTimer.pause(), lockTimer.pause(), superLockTimer.pause();
		bin.pause();
	}
	// Resume game and timers
	void resumeGame() {
		paused = false;
		gravityTimer.resume(), lockTimer.resume(), superLockTimer.resume();
		bin.resume();
	}
	// Handles whether to pause or resume the game
	void doPauseResume() {
		if (gameOver) // Disable if game is over. Prevents death animation bug
			return;
		if (paused)
			resumeGame();
		else
			pauseGame();
	}
	// Clear board and restart game. Reset gravity and piece queue
	void resetBoard() {
		board.clear();
		// Generate new board
		for (int i = 0; i < REALNUMROWS; i++) {
			vector<Tile> row;
			for (int j = 0; j < NUMCOLS; j++) {
				row.push_back(Tile(*blockTexture, gameBounds.left + j * TILESIZE, gameBounds.top + (i - 2) * TILESIZE));
			}
			board.push_back(row);
		}
		clearMovingSprites();
		heldPiece = nullptr;
		heldSprite.clear();
		if (gameMode != SANDBOX) // Reset gravity if not in sandbox mode
			resetGravity();
		gravityTimer.restart(), lockTimer.restart(), superLockTimer.restart();
		bag->resetPosition(playerIndex);
		hasHeld = false, gameOver = false, paused = false;
		bin.clear();
		totalLinesCleared = 0, inGarbage = 0; outGarbage = 0;
		garbLastCol = rand() % NUMCOLS;
		spawnPiece();
	}
	// Handle loss based on game mode
	void doGameOver() {
		switch (gameMode)
		{
		case CLASSIC:
			paused = true;
			gameOver = true;
			playDeathAnimation();
			break;
		case SANDBOX:
			bag->resetQueue();
			resetBoard();
			break;
		case MULTIPLAYER:
			paused = true;
			gameOver = true;
			playDeathAnimation();
			break;
		default:
			break;
		}
	}
	// Check to increase gravity after a number of lines has been cleared. Only in classic mode.
	void doSpeedUp() {
		auto iter = gravityTiers.begin();
		// Iterate to the next speed tier and the number of lines required to reach it
		for (; iter != gravityTiers.end() && iter->second >= gravity; iter++);
		if (iter == gravityTiers.end())
			return;
		if (totalLinesCleared >= iter->first && gravity >= iter->second) {
			setGravity(iter->second);
			clearAnimations[0].restart();
		}
	}


#pragma region Garbage Interaction
	// When clearing lines, send garbage to the opponent or cancel out incoming garbage
	void sendGarbage(int lineCount) {
		outGarbage += bin.clearGarbage(ceil(lineCount * garbageMultiplier));
	}
	// Add incoming garbage with a timer. // Does nothing if count is 0;
	void receiveGarbage(int lineCount) {
		if (lineCount == 0)
			return;
		bin.addGarbage(lineCount);
		bin.getBin();
	}
	// Check garbage bin. See if garbage should be dumped.
	void updateGarbage() {
		int lines = bin.getGarbage();
		if (lines > 0)
			inGarbage += lines;
	}
	// Incoming garbage pushes the board up
	void dumpGarbage(int lineCount) {
		if (creativeMode) // Disable if creative mode is on
			return;
		for (int i = 0; i < lineCount; i++) {
			// If top row has blocks, game over.
			for (Tile& tile : board[0])
				if (tile.getHasBlock()) {
					doGameOver();
					return;
				}

			// Move all rows up
			for (int j = 0; j < REALNUMROWS; j++)
				for (Tile& tile : board[j])
					tile.moveUp();
			board.erase(board.begin());

			// Insert new row at bottom
			vector<Tile> newRow;
			for (int j = 0; j < NUMCOLS; j++)
				newRow.push_back(Tile(*blockTexture, gameBounds.left + j * TILESIZE, gameBounds.top + GAMEHEIGHT - TILESIZE));

			// Fill in new row except one square.
			int randomColumn;
			// Increase chance of repeat columns
			float num = (float)rand() / RAND_MAX;
			if (num < garbRepeatProbability)
				randomColumn = garbLastCol;
			else {
				randomColumn = rand() % NUMCOLS;
				garbLastCol = randomColumn;
			}
			for (int j = 0; j < NUMCOLS; j++) {
				if (j != randomColumn)
					newRow[j].setBlock(true, WHITE);
			}
			board.insert(board.end(), newRow);
		}
		inGarbage = 0;
		soundFX->play(LOWBEEP);
	}
#pragma endregion

#pragma region Getters/Setters
	// Create HUD on game position
	void setHUD(sf::Vector2f gamePos, sf::Font& font) {
		// Rectangles for game screen, hold, queue, garbage bin, and top row, 
		screenRects.push_back(SfRectangleAtHome(BLACK, { GAMEWIDTH, GAMEHEIGHT },
			gamePos, false, WHITE, LINEWIDTH));
		screenRects.push_back(SfRectangleAtHome(BLACK, { TILESIZE * 4, TILESIZE * 4 },
			{ gamePos.x - TILESIZE * 4.5f - LINEWIDTH, gamePos.y + LINEWIDTH }, false, WHITE, LINEWIDTH));
		screenRects.push_back(SfRectangleAtHome(BLACK, { TILESIZE * 4, GAMEHEIGHT / 9 * NEXTPIECECOUNT },
			{ gamePos.x + GAMEWIDTH + LINEWIDTH, gamePos.y + LINEWIDTH }, false, WHITE, LINEWIDTH));
		screenRects.push_back(SfRectangleAtHome(BLACK, { TILESIZE / 2, GAMEHEIGHT - LINEWIDTH },
			{ gamePos.x - TILESIZE / 2 - LINEWIDTH, gamePos.y + LINEWIDTH }, false, WHITE, LINEWIDTH));

		// Rectangles to show a couple pixels of the very top row
		screenRects.push_back(SfRectangleAtHome(BLACK, { GAMEWIDTH, TOPROWPIXELS },
			{ gamePos.x, gamePos.y - TOPROWPIXELS }, false, WHITE, LINEWIDTH));
		screenRects.push_back(SfRectangleAtHome(BLUE, { GAMEWIDTH + 1, TILESIZE - 9 },
			{ gamePos.x - 1, gamePos.y - TILESIZE - 1 }));

		// Get bounds of first three rectangles (game, hold, queue)
		gameBounds = screenRects[0].getGlobalBounds();
		holdBounds = screenRects[1].getGlobalBounds();
		queueBounds = screenRects[2].getGlobalBounds();


		// Generate all static text on game screen
		holdText = SfTextAtHome(font, WHITE, "Hold", GAMETEXTSIZE, { holdBounds.left + holdBounds.width / 2, holdBounds.top - GAMETEXTSIZE }, true, false, true);
		nextText = SfTextAtHome(font, WHITE, "Next", GAMETEXTSIZE, { queueBounds.left + queueBounds.width / 2, queueBounds.top - GAMETEXTSIZE }, true, false, true);
		gamemodeText = SfTextAtHome(font, WHITE, "Classic Mode", GAMETEXTSIZE * 2, { gameBounds.left + gameBounds.width / 2, gameBounds.top - GAMETEXTSIZE * 2 }, true, false, true);

		// Generate game field lines
		for (int i = 1; i < NUMROWS; i++) { // Horizontal lines
			lines.push_back(SfRectangleAtHome(SEETHROUGH, { GAMEWIDTH, LINEWIDTH }, { gameBounds.left, gameBounds.top + i * TILESIZE - 1 }));
		}
		for (int j = 1; j < NUMCOLS; j++) { // Vertical lines
			lines.push_back(SfRectangleAtHome(SEETHROUGH, { LINEWIDTH, GAMEHEIGHT + TOPROWPIXELS },
				{ gameBounds.left + j * TILESIZE - 1, gameBounds.top - TOPROWPIXELS }));
		}
	}
	sf::FloatRect& getGameBounds() {
		return gameBounds;
	}
	void setGamemodeTextString(string str) {
		gamemodeText.setString(str);
	}
	void setGamemodeTextXPos(float x) {
		gamemodeText.setPosition(x, gamemodeText.getPosition().y);
	}
	vector<SfRectangleAtHome>& getScreenRects() {
		return screenRects;
	}
	// Set gravity to a specific speed or back to its starting speed
	void setGravity(float speed) {
		gravity = speed;
		gravityTimer.restart();
	}
	void resetGravity() {
		gravity = startingGravity;
		gravityTimer.restart();
	}
	void setStartingGravity(float val) {
		startingGravity = val;
	}
	// Set game mode with a constant int code defined in TetrisConstants
	void setGameMode(const int MODE) {
		gameMode = MODE;
	}
	void setLockDelay(float val) {
		lockDelay = val;
	}
	void setSuperLockDelay(float val) {
		superLockDelay = val;
	}
	void setNextPieceCount(int val) {
		nextPieceCount = val;
	}
	void setHoldEnabled(bool val) {
		holdEnabled = val;
	}
	void setGhostPieceEnabled(bool val) {
		ghostPieceEnabled = val;
	}
	void setBagEnabled(bool val) {
		bagEnabled = val;
	}
	void setSRS(bool val) {
		SRS_Enabled = val;
	}
	void setGarbageTimer(float val) {
		bin.setDuration(val);
	}
	void setGarbRepeatProbability(float val) {
		garbRepeatProbability = val;
	}
	void setGarbageMultiplier(float val) {
		garbageMultiplier = val;
	}
	void setColorPallete(int val) {
		// Update color pallete
		colorPallete = val;
		for (int i = 0; i < tetrominos.size(); i++)
			tetrominos[i]->setColor(PIECECOLORSETS[colorPallete][i]);
	}
	sf::Texture& getBlockTexture(){
		return *blockTexture;
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
	bool getPaused() {
		return paused;
	}
	// Sends out outgoing garbage to main for other players to receive
	// NOTE: This will be called in main to send garbage to other players
	int getOutGarbage() {
		int temp = outGarbage;
		outGarbage = 0;
		return temp;
	}
	// Return vector for drawing GarbageStack
	vector<float> getStackVector() {
		if (creativeMode) // Return nothing if creative mode is on
			return {};
		vector<float> vec = bin.getBin();
		// Add a 1 for each garbage line ready to dump
		for (int i = 0; i < inGarbage; i++)
			vec.insert(vec.begin(), 1);
		return vec;
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
		int col = (int)((clickPos.x - gameBounds.left) / TILESIZE);
		int row = (int)((clickPos.y - gameBounds.top) / TILESIZE + 2); // Adjust to exclude the rows outside of game window

		if (!board[row][col].getHasMovingBlock()) {
			board[row][col].toggleBlock();
			updateBlocks();
		}
	}
	// Works like clickBlock. Fills in all columns in a row other than the clicked column. 
	void clickRow(sf::Vector2f& clickPos) {
		if (!creativeMode) // Function only works if creative mode is toggled on
			return;
		// Convert mouse position to game coordinates.
		int col = (int)((clickPos.x - gameBounds.left) / TILESIZE);
		int row = (int)((clickPos.y - gameBounds.top) / TILESIZE + 2); // Adjust to exclude the rows outside of game window
		for (int i = 0; i < NUMCOLS; i++) {
			if (!board[row][i].getHasMovingBlock())
				board[row][i].setBlock(true, WHITE);
		}
		board[row][col].setBlock(false);
		updateBlocks();
	}
#pragma endregion

#pragma region Graphics
	// Draw all tiles, held piece, and queue pieces
	void drawScreen() {
		// Last rectangle needs to be redrawn manually
		for (const SfRectangleAtHome& rect : screenRects)
			window->draw(rect);
		if (!paused) {
			for (const SfRectangleAtHome& line : lines)
				window->draw(line);
			for (int i = 1; i < REALNUMROWS; i++) {
				for (int j = 0; j < NUMCOLS; j++) {
					window->draw(board[i][j]);
				}
			}
			for (sf::Sprite& sprite : heldSprite)
				window->draw(sprite);
			for (int i = 0; i < nextPieceCount; i++)
				for (sf::Sprite& sprite : nextPieceSprites[i])
					window->draw(sprite);

			// Draw garbage stack if game mode is sandbox or PVP
			if (gameMode != CLASSIC)
				window->draw(garbStack);
		}
		// Enable death animation if game is over
		if (gameOver)
			deathAnimation.update(*window);

		window->draw(screenRects.back()); // Redraw last rectangle

		if (holdEnabled)
			window->draw(holdText);
		if (nextPieceCount > 0)
			window->draw(nextText);
		window->draw(gamemodeText);
		for (FadeText& animation : clearAnimations)
			animation.update(*window);
	}
	// Hide current moving tiles, update position, set new tiles
	void updateBlocks() {
		if (currentPositions.size() != 0)
			setMovingBlocks(currentPositions, false);
		currentPositions = currentPiece->getPositions();
		updatePreview();
		setMovingBlocks(currentPositions, true, currentPiece->color);
	}
	// Update ghost piece visuals
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
		if (ghostPieceEnabled)
			previewColor.a = PREVIEWTRANSPARENCY;
		else // Disable ghost piece
			previewColor.a = 0;
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
	// Play fadeText animations
	void playClearText(string str) {
		clearAnimations[1].setString(str);
		clearAnimations[1].restart();
	}
	void playBackToBackText() {
		clearAnimations[2].restart();
	}
	void playComboText() {
		clearAnimations[3].setString(to_string(comboCounter) + "X Combo");
		clearAnimations[3].restart();
	}
	void playAllClearText() {
		clearAnimations[4].restart();
	}
	// Play death animation. This is performed right before game over.
	// NOTE: This will be called by main to avoid bugs with simultaneous loss in pvp
	void playDeathAnimation() {
		deathAnimation.restart();
		soundFX->pauseAll();
		soundFX->play(LOWTHUD);
	}
#pragma endregion

#pragma region Boolean Checks
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
	bool checkTspin() {
		if (!currentPiece->isTPiece() || !lastMoveSpin) // Skip the corner checks if first two conditions fail
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
	// Check if death animation has finished playing
	bool isDeathAnimationOver() {
		return deathAnimation.isOver();
	}
#pragma endregion
};