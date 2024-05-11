#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <fstream>
#include <map>
#include "TetrisConstants.h"
#include "Screen.h"
#include "GameSettings.h"
#include "Mechanisms.h"
#include "Drawing.h"

using namespace std;
using namespace TetrisVariables;
// Ruobin Chen
// Tetris game made with SFML 2.5.1
// Sound effects and music made with IOS Garage Band
// Line count as of 5/11/2024: 3937

// Generate text exclusive to sandbox mode
vector<sf::Text> getSandboxText(sf::Font& font) {
	vector<sf::Text> textboxes;
	vector<string> menuItems = { "Auto-fall", "Fall speed", "Creative", "Reset", "Quit" };
	for (int i = 0; i < menuItems.size(); i++)
		textboxes.push_back(SfTextAtHome(font, WHITE, menuItems[i], MENUTEXTSIZE, { SANDBOXMENUPOS.x, SANDBOXMENUPOS.y + MENUSPACING * i }));
	return textboxes;
}
// Generate text for loss screen
vector<sf::Text> getLossText(sf::Font& font) {
	vector<sf::Text> textboxes;
	textboxes.push_back(SfTextAtHome(font, WHITE, "YOU LOST", GAMETEXTSIZE * 4, { WIDTH / 2, GAMEYPOS }, true, false, true));
	textboxes.push_back(SfTextAtHome(font, WHITE, "Press any key to return to Main Menu", GAMETEXTSIZE, { WIDTH / 2, HEIGHT / 2 }, true, false, true));

	return textboxes;
}
// Get collision bounds for checkboxes
vector<sf::FloatRect> getBoxBounds(vector<sf::RectangleShape>& rects) {
	vector<sf::FloatRect> bounds;
	for (auto& rect : rects)
		bounds.push_back(rect.getGlobalBounds());
	return bounds;
}

// Generate a map between nontext keys that can be used as keybinds and their strings 
map<sf::Keyboard::Key, string> getKeyStrings() {
	map<sf::Keyboard::Key, string> keyStrings;
	keyStrings[sf::Keyboard::Menu] = "Menu";
	keyStrings[sf::Keyboard::LBracket] = "[";
	keyStrings[sf::Keyboard::RBracket] = "]";
	keyStrings[sf::Keyboard::Semicolon] = ";";
	keyStrings[sf::Keyboard::Comma] = ",";
	keyStrings[sf::Keyboard::Period] = ".";
	keyStrings[sf::Keyboard::Quote] = "'";
	keyStrings[sf::Keyboard::Slash] = "/";
	keyStrings[sf::Keyboard::Backslash] = "\\";
	keyStrings[sf::Keyboard::Tilde] = "`";
	keyStrings[sf::Keyboard::Equal] = "=";
	keyStrings[sf::Keyboard::Hyphen] = "-";
	keyStrings[sf::Keyboard::LShift] = "LShift";
	keyStrings[sf::Keyboard::RShift] = "RShift";
	keyStrings[sf::Keyboard::Left] = "Left";
	keyStrings[sf::Keyboard::Up] = "Up";
	keyStrings[sf::Keyboard::Down] = "Down";
	keyStrings[sf::Keyboard::Right] = "Right";
	keyStrings[sf::Keyboard::LControl] = "LControl";
	keyStrings[sf::Keyboard::RControl] = "RControl";
	keyStrings[sf::Keyboard::LAlt] = "LAlt";
	keyStrings[sf::Keyboard::RAlt] = "RAlt";
	keyStrings[sf::Keyboard::Tab] = "Tab";
	keyStrings[sf::Keyboard::Enter] = "Enter";
	keyStrings[sf::Keyboard::Space] = "Space";
	keyStrings[sf::Keyboard::Backspace] = "Backspace";
	keyStrings[sf::Keyboard::PageUp] = "PageUp";
	keyStrings[sf::Keyboard::PageDown] = "PageDown";
	keyStrings[sf::Keyboard::End] = "End";
	keyStrings[sf::Keyboard::Home] = "Home";
	keyStrings[sf::Keyboard::Insert] = "Insert";
	keyStrings[sf::Keyboard::Delete] = "Delete";
	// Procedurally generate alphabet, num, numpad, and Fnum maps for cleaner code
	for (int i = 0; i <= 25; i++)
		keyStrings[sf::Keyboard::Key(i)] = string(1, i + 65);
	for (int i = 26; i <= 35; i++)
		keyStrings[sf::Keyboard::Key(i)] = "Num" + to_string(i - 26);
	for (int i = 75; i <= 84; i++)
		keyStrings[sf::Keyboard::Key(i)] = "Numpad" + to_string(i - 75);
	for (int i = 85; i <= 99; i++)
		keyStrings[sf::Keyboard::Key(i)] = "F" + to_string(i - 84);
	return keyStrings;
}

// Following functions are made for reuseability and requires specific parameters passed in.
// Functionality of the toggles in sandbox mode. 
void toggleGravity(Checkbox* autoFallBox, Screen* screen) {
	if (autoFallBox->getChecked()) {
		autoFallBox->setChecked(false);
		screen->setAutoFall(false);
	}
	else {
		autoFallBox->setChecked(true);
		screen->setAutoFall(true);
	}
}
void decrementGravity(IncrementalBox* gravityBox, Checkbox* autoFallBox, Screen* screen) {
	gravityBox->decrement();
	screen->setGravity(GRAVITYSPEEDS[gravityBox->getCurrentNum() - 1]);
}
void incrementGravity(IncrementalBox* gravityBox, Checkbox* autoFallBox, Screen* screen) {
	gravityBox->increment();
	screen->setGravity(GRAVITYSPEEDS[gravityBox->getCurrentNum() - 1]);
}
void toggleCreative(Checkbox* creativeModeBox, Checkbox* autoFallBox, Screen* screen) {
	if (creativeModeBox->getChecked()) { // If box is already checked, turn off
		creativeModeBox->setChecked(false);
		autoFallBox->setChecked(true);
		screen->setAutoFall(true);
		screen->endCreativeMode();
	}
	else { // If box isn't checked, turn on
		creativeModeBox->setChecked(true);
		autoFallBox->setChecked(false);
		screen->setAutoFall(false);
		screen->startCreativeMode();
	}
}

// Draw all objects in a vector
void drawVector(sf::RenderWindow& window, vector<sf::Text>& vec) {
	for (sf::Text& item : vec)
		window.draw(item);
}

// Update gameplay settings for a single player
void updateGameplaySettings(vector<int> values, Screen* screen) {
	// Starting speed. Will not take effect in sandbox mode
	screen->setStartingGravity(GRAVITYSPEEDS[values[0]]);
	screen->setNextPieceCount(values[1]); // Next piece count
	screen->setHoldEnabled(values[2]);	// Piece holding
	screen->setGhostPieceEnabled(values[3]); // Ghost piece
	screen->setBagEnabled(values[6]); // 7-bag
	screen->setSRS(values[7]); // Rotation style
	screen->setGarbageTimer(GARBAGETIMERS[values[8]]); // Garbage timer
	screen->setGarbageMultiplier(GARBAGEMULTIPLIERS[values[9]]); // Garbage multiplier
	screen->setGarbRepeatProbability(GARBAGEREPEATPROBABILITIES[values[10]]); // Garbage repeat probability
}

// Update keybind configurations from setting tab entries
void updateKeybinds(vector<KeySet*>& keySets, vector<KeyRecorder*> newKeybinds) {
	for (int i = 0; i < newKeybinds.size(); i++) // Update keybind controls
		*keySets[i / 7]->getSet()[i % 7] = *newKeybinds[i]->getKey();
}

// Convert settings menu contents to game variables
void updateAllSettings(SettingsMenu& gameSettings, vector<Screen*> screens, vector<KeyDAS*> dasSets, vector<KeySet*> keySets) {
	vector<int> gameplaySettings = gameSettings[0].getValues();
	// Update hold and queue display
	for (Screen* screen : screens) {
		updateGameplaySettings(gameplaySettings, screen);
		screen->getScreenRects()[1].setSize(sf::Vector2f{ TILESIZE * 4.0f * gameplaySettings[2], TILESIZE * 4.0f * gameplaySettings[2] });
		screen->getScreenRects()[2].setSize(sf::Vector2f{ TILESIZE * 4.0f * gameplaySettings[1] / gameplaySettings[1], GAMEHEIGHT / 9.0f * gameplaySettings[1] });
	}
	for (int i = 0; i < dasSets.size(); i++) {
		dasSets[i]->setStartDelay(DASDELAYVALUES[gameplaySettings[4]]); // DAS delay
		dasSets[i]->setHoldDelay(DASSPEEDVALUES[gameplaySettings[5]]); // DAS speed
	}

	updateKeybinds(keySets, gameSettings[1].getKeybinds());
}

// Play a sound starting at a time point
SoundManager* generateSoundManager() {
	SoundManager* soundFX = new SoundManager(SOUNDFXFILEPATH);
	soundFX->addEffect(MEDIUMBEEP);
	soundFX->addEffect(HIGHBEEP);
	soundFX->addEffect(LIGHTTAP);
	soundFX->addEffect(HIGHHIGHBEEP);
	soundFX->addEffect(LOWBEEP);
	soundFX->addEffect(LOWTHUD);
	soundFX->setVolume(20);
	return soundFX;
}

// Read config file and return a vector of integers
vector<int> readConfigFile(string fileName) {
	ifstream inFile(fileName);
	try {
		if (!inFile.is_open())
			throw ConfigError();
		vector<int> configValues;
		string line;
		while (getline(inFile, line))
			configValues.push_back(stoi(line));
		return configValues;
	}
	catch (ConfigError err) {
		cout << "Config file does not exist. Creating default file.\n";
		return DEFAULTSETTINGS;
	}
	catch (exception err) {
		cout << "Config file reading error. Restoring to defaults.\n";
		return DEFAULTSETTINGS;
	}
	inFile.close();
}

// Write setting menu contents to a file
void writeConfigFile(string fileName, vector<int> values) {
	ofstream outFile(fileName);
	if (!outFile.is_open()) {
		cout << "Failed to write file";
		throw exception();
	}
	for (int val : values)
		outFile << to_string(val) << endl;
	outFile.close();
}

int main() {
	srand(time(NULL));
#pragma region SFML Setup
	// Set SFML objects
	sf::Font font;
	if (!font.loadFromFile(FONTFILEPATH))
		return -1;
	sf::Texture texture;
	if (!texture.loadFromFile(BLOCKFILEPATH))
		return -1;
	SoundManager* soundFX = generateSoundManager();
	sf::Music bgm;
	if (!bgm.openFromFile(BGMFILEPATH))
		return -1;
	bgm.setVolume(5);
	bgm.setLoop(true);

	sf::ContextSettings windowSettings;
	windowSettings.antialiasingLevel = 8;
	sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Tetris", sf::Style::Close | sf::Style::Titlebar, windowSettings);
	window.setFramerateLimit(FPS);
	window.setKeyRepeatEnabled(false);
#pragma endregion

#pragma region Basic Assets
	// Title screen sprites
	sf::Text titleText(SfTextAtHome(font, WHITE, "TETRIS", 150, TITLETEXTPOS, true, false, true));
	sf::CircleShape* cursor = new sf::CircleShape(15.f, 3); // Triangle shaped cursor
	cursor->rotate(90.f);
	vector<string> menuText = { "Classic Mode", "Sandbox Mode", "PVP Mode", "Settings", "Quit" };
	ClickableMenu gameMenu(font, WHITE, menuText, MENUTEXTSIZE, MENUPOS, MENUSPACING, *cursor);
#pragma endregion

#pragma region Extra Assets
	// Sandbox mode exclusive sprites
	vector<sf::Text> sandboxText = getSandboxText(font);
	Checkbox* autoFallBox = new Checkbox(TILESIZE, SANDBOXMENUPOS.x + 180, SANDBOXMENUPOS.y, true, font);
	IncrementalBox* gravityBox = new IncrementalBox(TILESIZE, SANDBOXMENUPOS.x + 180, SANDBOXMENUPOS.y + MENUSPACING, 1, GRAVITYTIERCOUNT, font);
	Checkbox* creativeModeBox = new Checkbox(TILESIZE, SANDBOXMENUPOS.x + 180, SANDBOXMENUPOS.y + MENUSPACING * 2, false, font);
	Checkbox* resetBox = new Checkbox(TILESIZE, SANDBOXMENUPOS.x + 180, SANDBOXMENUPOS.y + MENUSPACING * 3, false, font);
	Checkbox* quitBox = new Checkbox(TILESIZE, SANDBOXMENUPOS.x + 180, SANDBOXMENUPOS.y + MENUSPACING * 4, false, font);
	vector<Checkbox*> sandboxes = { autoFallBox, gravityBox, creativeModeBox, resetBox, quitBox };
	bool creativeModeOn = false;

	// Pause screen sprites
	vector<string> pauseMenuText = { "Continue", "Restart", "Quit" };
	pauseScreen pause(GAMEPOS, pauseMenuText, font);

	// Text for loss screen
	vector<sf::Text> lossText = getLossText(font);

#pragma endregion

#pragma region Controller Classes
	// Initialize controls with default keybinds
	KeySet* playerSoloKeys = new KeySet(LEFT, RIGHT, UP, DOWN, SPINCW, SPINCCW, HOLD);
	KeySet* player1Keys = new KeySet(LEFT1, RIGHT1, UP1, DOWN1, SPINCW1, SPINCCW1, HOLD1);
	KeySet* player2Keys = new KeySet(LEFT2, RIGHT2, UP2, DOWN2, SPINCW2, SPINCCW2, HOLD2);
	vector<KeySet*> keySets = { playerSoloKeys, player1Keys, player2Keys };

	// Initiate DAS keys
	KeyDAS* playerSoloDAS = new KeyDAS(DASDELAY, DASSPEED, playerSoloKeys);
	KeyDAS* player1DAS = new KeyDAS(DASDELAY, DASSPEED, player1Keys);
	KeyDAS* player2DAS = new KeyDAS(DASDELAY, DASSPEED, player2Keys);
#pragma endregion

	// Initiate piece rng
	pieceBag bag;

	// Set up game screen
	Screen* screen = new Screen(window, GAMEPOS, font, texture, &bag, soundFX);
	Screen* screenP2 = new Screen(window, GAMEPOSP2, font, texture, &bag, soundFX);
	screenP2->setGamemodeTextString("PVP Mode"); // This will be the title text used in pvp mode. Hide the other title text
	screenP2->setGamemodeTextX(WIDTH);
	sf::Text linesClearedText = SfTextAtHome(font, WHITE, "Lines: 0", 25, { GAMEXPOS + GAMEWIDTH + 150, GAMEYPOS });
	int currentScreen = MAINMENU;

	// Key recorders need a pointer to keyStrings stored in main
	map<sf::Keyboard::Key, string> keyStrings = getKeyStrings();

	// Set up settings menu
	SettingsMenu gameSettings(soundFX, font, keyStrings);

	vector<ClickableButton> clickableButtons; // { Reset, Discard, Save }
	clickableButtons.push_back(ClickableButton({ 230, 40 }, { 150, 725 }, font, "Reset Defaults", BUTTONTEXTSIZE, RED));
	clickableButtons.push_back(ClickableButton({ 230, 40 }, { 400, 725 }, font, "Discard & Quit", BUTTONTEXTSIZE, RED));
	clickableButtons.push_back(ClickableButton({ 230, 40 }, { 650, 725 }, font, "Save & Quit", BUTTONTEXTSIZE, RED));

	// Read settings file
	vector<int> configValues = readConfigFile(CONFIGFILEPATH); // Stores a copy of file contents
	gameSettings.applyConfig(configValues);
	// Rewrite settings file to fix invalid configurations from previous two lines.
	writeConfigFile(CONFIGFILEPATH, gameSettings.getValues());

	updateAllSettings(gameSettings, { screen, screenP2 },
		{ playerSoloDAS, player1DAS, player2DAS }, keySets);

	// Game loop
	while (window.isOpen())
	{
		// Manage audio across all screens
		soundFX->checkTimers();

		// Run on main menu
		if (currentScreen == MAINMENU) {
			window.clear(BLUE);
			window.draw(titleText);
			window.draw(gameMenu);

			bool modeSelected = false;

			// Event handler for menu screen
			sf::Event event;
			while (window.pollEvent(event)) {
				switch (event.type)
				{
				case sf::Event::Closed:
					window.close();
					break;
				case sf::Event::KeyPressed:
				{
					if (event.key.code == sf::Keyboard::Down) {
						soundFX->play(LIGHTTAP);
						gameMenu.moveDown();
					}
					else if (event.key.code == sf::Keyboard::Up) {
						soundFX->play(LIGHTTAP);
						gameMenu.moveUp();
					}
					else if (event.key.code == sf::Keyboard::Z)
						modeSelected = true;
					break;
				}
				case sf::Event::MouseMoved: {
					if (gameMenu.updateMouseMove(event.mouseMove.x, event.mouseMove.y))
						soundFX->play(LIGHTTAP);
					break;
				}
				case sf::Event::MouseButtonPressed: {
					if (gameMenu.updateMouseClick(event.mouseButton.x, event.mouseButton.y))
						modeSelected = true;

					break;
				}
				case sf::Event::MouseButtonReleased: {
					break;
				}
				case sf::Event::TextEntered: {
					break;
				}
				default:
					break;
				}
			}

			// Select menu option if modeSelected is true
			if (modeSelected) {
				soundFX->play(HIGHBEEP);
				switch (gameMenu.getCursorPos())
				{
				case 0: // Classic mode
					currentScreen = CLASSIC;
					screen->setGameMode(CLASSIC);
					screen->setGamemodeTextString("Classic Mode");
					screen->setAutoFall(true);
					screen->endCreativeMode();
					bag.resetQueue();
					screen->resetBoard();
					bgm.play();
					break;
				case 1: // Sandbox mode
					currentScreen = SANDBOX;
					screen->setGamemodeTextString("Sandbox Mode");
					screen->setGameMode(SANDBOX);
					// Reset sandbox settings
					autoFallBox->setChecked(true);
					gravityBox->setValue(gravityBox->getMin());
					creativeModeBox->setChecked(false);
					screen->setAutoFall(true);
					screen->endCreativeMode();
					bag.resetQueue();
					screen->resetBoard();
					bgm.play();
					break;
				case 2: // PVP mode
					currentScreen = MULTIPLAYER;
					window.setSize({ WIDTH * 2, HEIGHT });
					window.setView(sf::View(sf::FloatRect(0, 0, WIDTH * 2, HEIGHT)));
					window.setPosition({ 100, 100 });
					screen->setGameMode(MULTIPLAYER);
					screen->setGamemodeTextString("");
					screen->setAutoFall(true);
					screen->endCreativeMode();
					bag.resetQueue();
					screen->resetBoard();

					screenP2->setGameMode(MULTIPLAYER);
					screenP2->resetBoard();
					bgm.play();
					break;
				case 3: // Settings
					gameSettings.selectTab(0);
					currentScreen = SETTINGSCREEN;
					break;
				case 4: // Quit
					window.close();
					break;
				default:
					break;
				}
				gameMenu.resetCursorPos();
			}
		}
		// Run on classic mode
		else if (currentScreen == CLASSIC) {
			window.clear(BLUE);
			screen->drawScreen();

			// This is only shown in classic mode
			linesClearedText.setString("Lines: " + to_string(screen->getLinesCleared()));
			window.draw(linesClearedText);

			if (screen->getPaused() && !screen->getGameOver())
				window.draw(pause);
			bool modeSelected = false; // For pause screen

			// Manage audio
			soundFX->checkTimers();

			// In-game timer events
			screen->doTimeStuff();

			// Check for game over
			if (screen->getGameOver()) {
				bgm.stop();
				if (screen->isDeathAnimationOver()) {
					currentScreen = LOSESCREEN;
					lossText[0] = SfTextAtHome(font, WHITE, "YOU LOST", GAMETEXTSIZE * 4, { WIDTH / 2, GAMEYPOS }, true, false, true);
				}
			}

			// Handles movement with auto-repeat (DAS)
			playerSoloDAS->checkKeyPress(screen);

			// Event handler for game screen
			sf::Event event;
			while (window.pollEvent(event)) {
				switch (event.type)
				{
				case sf::Event::Closed:
					window.close();
					break;
				case sf::Event::KeyPressed:
				{
					if (screen->getPaused()) { // Pause screen
						if (event.key.code == sf::Keyboard::Down) {
							soundFX->play(LIGHTTAP);
							pause.getMenu().moveDown();
						}
						else if (event.key.code == sf::Keyboard::Up) {
							soundFX->play(LIGHTTAP);
							pause.getMenu().moveUp();
						}
						else if (event.key.code == sf::Keyboard::Z)
							modeSelected = true;
					}
					else { // Game controls
						if (event.key.code == playerSoloKeys->getUp())
							screen->movePiece(3);
						else if (event.key.code == playerSoloKeys->getSpinCCW())
							screen->spinPiece(false);
						else if (event.key.code == playerSoloKeys->getSpinCW())
							screen->spinPiece(true);
						else if (event.key.code == playerSoloKeys->getHold())
							screen->holdPiece();
					}
					if (event.key.code == sf::Keyboard::Escape) {
						screen->doPauseResume();
						pause.getMenu().resetCursorPos();
						soundFX->play(MEDIUMBEEP);
					}
					break;
				}
				case sf::Event::KeyReleased: {
					playerSoloDAS->releaseKey(event.key.code);
				}
				case sf::Event::MouseButtonPressed: {
					if (screen->getPaused() && pause.getMenu().updateMouseClick(event.mouseButton.x, event.mouseButton.y))
						modeSelected = true;
					break;
				}
				case sf::Event::MouseMoved: {
					if (screen->getPaused() && pause.getMenu().updateMouseMove(event.mouseMove.x, event.mouseMove.y))
						soundFX->play(LIGHTTAP);
					break;
				}
				default:
					break;
				}
			}
			// Select menu option if modeSelected is true
			if (modeSelected) {
				switch (pause.getMenu().getCursorPos())
				{
				case 0: // Continue
					screen->doPauseResume();
					soundFX->play(MEDIUMBEEP);
					break;
				case 1: // Restart
					bag.resetQueue();
					screen->resetBoard();
					soundFX->play(HIGHBEEP);
					break;
				case 2: // Quit
					bgm.stop();
					currentScreen = MAINMENU;
					soundFX->play(HIGHBEEP);
					break;
				default:
					break;
				}
				modeSelected = false;
				pause.getMenu().resetCursorPos();
			}
		}
		// Run on sandbox mode
		else if (currentScreen == SANDBOX) {
			window.clear(BLUE);
			screen->drawScreen();
			drawVector(window, sandboxText);
			for (auto box : sandboxes)
				window.draw(*box);

			// Manage audio
			soundFX->checkTimers();

			// In-game timer events
			screen->doTimeStuff();
			// Handles movement with auto-repeat (DAS)
			playerSoloDAS->checkKeyPress(screen);

			// Event handler for game screen
			sf::Event event;
			while (window.pollEvent(event)) {
				switch (event.type)
				{
				case sf::Event::Closed:
					window.close();
					break;
				case sf::Event::KeyPressed:
				{
					if (event.key.code == playerSoloKeys->getUp())
						screen->movePiece(3);
					else if (event.key.code == playerSoloKeys->getSpinCCW())
						screen->spinPiece(false);
					else if (event.key.code == playerSoloKeys->getSpinCW())
						screen->spinPiece(true);
					else if (event.key.code == playerSoloKeys->getHold())
						screen->holdPiece();
					else if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num7)
						screen->spawnPiece(event.key.code - 27);
					// Hot keys for sandbox controls
					else if (event.key.code == sf::Keyboard::Q || event.key.code == sf::Keyboard::Escape)
						toggleGravity(autoFallBox, screen);
					else if (event.key.code == sf::Keyboard::W)
						decrementGravity(gravityBox, autoFallBox, screen);
					else if (event.key.code == sf::Keyboard::S)
						incrementGravity(gravityBox, autoFallBox, screen);
					else if (event.key.code == sf::Keyboard::E)
						toggleCreative(creativeModeBox, autoFallBox, screen);
					else if (event.key.code == sf::Keyboard::R) {
						bag.resetQueue();
						screen->resetBoard();
					}
					else if (event.key.code == sf::Keyboard::T) {
						bgm.stop();
						currentScreen = MAINMENU;
					}
					else if (event.key.code == sf::Keyboard::G)
						screen->receiveGarbage(1);
					else if (event.key.code == sf::Keyboard::H)
						screen->receiveGarbage(4);
					break;
				}
				case sf::Event::KeyReleased: {
					playerSoloDAS->releaseKey(event.key.code);
				}
				case sf::Event::MouseButtonPressed: {
					sf::Vector2f clickPos(event.mouseButton.x, event.mouseButton.y);
					if (event.mouseButton.button == sf::Mouse::Left) {
						if (autoFallBox->getBounds().contains(clickPos))  // Turn off gravity
							toggleGravity(autoFallBox, screen);
						else if (gravityBox->getLeftBound().contains(clickPos))  // Speed arrows
							decrementGravity(gravityBox, autoFallBox, screen);
						else if (gravityBox->getRightBound().contains(clickPos))
							incrementGravity(gravityBox, autoFallBox, screen);
						else if (creativeModeBox->getBounds().contains(clickPos))
							toggleCreative(creativeModeBox, autoFallBox, screen);
						else if (screen->getGameBounds().contains(clickPos))  // Creative mode click
							screen->clickBlock(clickPos); // Will do nothing if creative mode isn't on
						else if (resetBox->getBounds().contains(clickPos)) {
							bag.resetQueue();
							screen->resetBoard();
						}
						else if (quitBox->getBounds().contains(clickPos)) { // Return to menu
							bgm.stop();
							currentScreen = MAINMENU;
						}
					}
					else if (event.mouseButton.button == sf::Mouse::Right && screen->getGameBounds().contains(clickPos)) {
						screen->clickRow(clickPos); // Creative mode right click
					}
					break;
				}
				case sf::Event::MouseMoved: { // Shows a transparent X when hovering over unchecked boxes
					for (Checkbox* box : sandboxes)
						box->setHovering(box->getBounds().contains((float)event.mouseMove.x, (float)event.mouseMove.y));
				}
				default:
					break;
				}
			}
		}
		// Run on PVP mode
		else if (currentScreen == MULTIPLAYER) {
			window.clear(BLUE);
			screen->drawScreen();
			screenP2->drawScreen();


			if (screen->getPaused() && !screen->getGameOver() && !screenP2->getGameOver())
				window.draw(pause);
			bool modeSelected = false; // For pause screen

			// Manage audio
			soundFX->checkTimers();

			// In-game timer events
			screen->doTimeStuff();
			screenP2->doTimeStuff();
			// Process garbage exchange
			screen->receiveGarbage(screenP2->getOutGarbage());
			screenP2->receiveGarbage(screen->getOutGarbage());

			// Check for game over
			if (screen->getGameOver()) {
				bgm.stop();
				screenP2->pauseGame();
				if (screen->isDeathAnimationOver()) {
					window.setSize({ WIDTH, HEIGHT });
					window.setView(sf::View(sf::FloatRect(0, 0, WIDTH, HEIGHT)));
					currentScreen = LOSESCREEN;
					lossText[0] = SfTextAtHome(font, WHITE, "PLAYER 2 WINS!", GAMETEXTSIZE * 4, { WIDTH / 2, GAMEYPOS }, true, false, true);
					if (screenP2->getGameOver() && screenP2->isDeathAnimationOver()) // Rare event if both players lose at the same time
						lossText[0] = SfTextAtHome(font, WHITE, "DRAW!", GAMETEXTSIZE * 4, { WIDTH / 2, GAMEYPOS }, true, false, true);
				}
			}
			else if (screenP2->getGameOver()) {
				bgm.stop();
				screen->pauseGame();
				if (screenP2->isDeathAnimationOver()) {
					window.setSize({ WIDTH, HEIGHT });
					window.setView(sf::View(sf::FloatRect(0, 0, WIDTH, HEIGHT)));
					currentScreen = LOSESCREEN;
					lossText[0] = SfTextAtHome(font, WHITE, "PLAYER 1 WINS!", GAMETEXTSIZE * 4, { WIDTH / 2, GAMEYPOS }, true, false, true);
				}
			}

			// Handles movement with auto-shift (DAS)
			player1DAS->checkKeyPress(screen);
			player2DAS->checkKeyPress(screenP2);

			// Event handler for game screen
			sf::Event event;
			while (window.pollEvent(event)) {
				switch (event.type)
				{
				case sf::Event::Closed:
					window.close();
					break;
				case sf::Event::KeyPressed:
				{
					if (screen->getPaused()) { // Pause screen
						if (event.key.code == sf::Keyboard::Down) {
							soundFX->play(LIGHTTAP);
							pause.getMenu().moveDown();
						}
						else if (event.key.code == sf::Keyboard::Up) {
							soundFX->play(LIGHTTAP);
							pause.getMenu().moveUp();
						}
						else if (event.key.code == sf::Keyboard::Z)
							modeSelected = true;
					}
					else {
						if (event.key.code == player1Keys->getUp())
							screen->movePiece(3);
						else if (event.key.code == player1Keys->getSpinCCW())
							screen->spinPiece(false);
						else if (event.key.code == player1Keys->getSpinCW())
							screen->spinPiece(true);
						else if (event.key.code == player1Keys->getHold())
							screen->holdPiece();
						else if (event.key.code == player2Keys->getUp())
							screenP2->movePiece(3);
						else if (event.key.code == player2Keys->getSpinCCW())
							screenP2->spinPiece(false);
						else if (event.key.code == player2Keys->getSpinCW())
							screenP2->spinPiece(true);
						else if (event.key.code == player2Keys->getHold())
							screenP2->holdPiece();
					}
					if (event.key.code == sf::Keyboard::Escape) {
						// Pause game and show menu. Disable during death animation for bug fix
						if (!screen->getGameOver() && !screenP2->getGameOver()) {
							soundFX->play(MEDIUMBEEP);
							screen->doPauseResume();
							screenP2->doPauseResume();
							pause.getMenu().resetCursorPos();
						}
					}
					break;
				}
				case sf::Event::KeyReleased: {
					player1DAS->releaseKey(event.key.code);
					player2DAS->releaseKey(event.key.code);
				}
				case sf::Event::MouseButtonPressed: {
					if (screen->getPaused() && pause.getMenu().updateMouseClick(event.mouseButton.x, event.mouseButton.y))
						modeSelected = true;
					break;
				}
				case sf::Event::MouseMoved: {
					if (screen->getPaused() && pause.getMenu().updateMouseMove(event.mouseMove.x, event.mouseMove.y))
						soundFX->play(LIGHTTAP);
					break;
				}
				default:
					break;
				}
			}

			// Select menu option if modeSelected is true
			if (modeSelected) {
				switch (pause.getMenu().getCursorPos())
				{
				case 0: // Continue
					screen->doPauseResume();
					screenP2->doPauseResume();
					soundFX->play(MEDIUMBEEP);
					break;
				case 1: // Restart
					bag.resetQueue();
					screen->resetBoard();
					screenP2->resetBoard();
					soundFX->play(HIGHBEEP);
					break;
				case 2: // Quit
					window.setSize({ WIDTH, HEIGHT });
					window.setView(sf::View(sf::FloatRect(0, 0, WIDTH, HEIGHT)));
					bgm.stop();
					currentScreen = MAINMENU;
					soundFX->play(HIGHBEEP);
					break;
				default:
					break;
				}
				modeSelected = false;
				pause.getMenu().resetCursorPos();
			}
		}
		else if (currentScreen == LOSESCREEN) {
			window.clear(BLACK);
			drawVector(window, lossText);

			// Manage audio
			soundFX->checkTimers();

			sf::Event event;
			while (window.pollEvent(event)) {
				switch (event.type)
				{
				case sf::Event::Closed:
					window.close();
					break;
				case sf::Event::KeyPressed:
					currentScreen = MAINMENU;
					soundFX->play(MEDIUMBEEP);
					break;
				case sf::Event::MouseButtonPressed:
					currentScreen = MAINMENU;
					soundFX->play(MEDIUMBEEP);
					break;
				default:
					break;
				}
			}
		}
		else if (currentScreen == SETTINGSCREEN) {
			window.clear(BLUE);
			window.draw(gameSettings);
			for (ClickableButton& button : clickableButtons)
				window.draw(button);

			// Manage audio
			soundFX->checkTimers();

			sf::Event event;
			while (window.pollEvent(event)) {
				switch (event.type)
				{
				case sf::Event::Closed:
					window.close();
					break;
				case sf::Event::KeyPressed:
					gameSettings[1].readKeys(event.key.code);
					break;
				case sf::Event::MouseMoved:
					gameSettings.processMouseMove(event.mouseMove.x, event.mouseMove.y);
					break;
				case sf::Event::MouseButtonPressed:
					gameSettings.processMouseClick(event.mouseButton.x, event.mouseButton.y);
					// Restore to default settings
					if (clickableButtons[0].checkClick(event.mouseButton.x, event.mouseButton.y)) {
						gameSettings.applyConfig(DEFAULTSETTINGS);
						soundFX->play(MEDIUMBEEP);
					}
					// Discard & quit
					else if (clickableButtons[1].checkClick(event.mouseButton.x, event.mouseButton.y)) {
						gameSettings.applyConfig(configValues);
						currentScreen = MAINMENU;
						soundFX->play(HIGHBEEP);
					}
					// Save & quit
					else if (clickableButtons[2].checkClick(event.mouseButton.x, event.mouseButton.y)) {
						// Update gameplay settings for two screens and three DAS sets
						updateAllSettings(gameSettings, { screen, screenP2 },
							{ playerSoloDAS, player1DAS, player2DAS }, keySets);
						writeConfigFile(CONFIGFILEPATH, gameSettings.getValues());
						// Refresh file contents copy without having to open the file again
						configValues = gameSettings.getValues();

						currentScreen = MAINMENU;
						soundFX->play(HIGHBEEP);
					}
					break;
				case sf::Event::MouseButtonReleased:
					gameSettings.processMouseRelease();
					break;
				default:
					break;
				}
			}
		}
		window.display();
	}

	// Cleanup
	for (Checkbox* box : sandboxes)
		delete box;
	delete playerSoloKeys;
	delete player1Keys;
	delete player2Keys;
	delete playerSoloDAS;
	delete player1DAS;
	delete player2DAS;
	delete screen;
	delete screenP2;
	delete soundFX;
	delete cursor;
	return 0;
}
