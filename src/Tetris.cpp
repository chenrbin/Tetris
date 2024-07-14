#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <fstream>
#include <map>
#include <vector>
#include "TetrisConstants.h"
#include "Mechanisms.h"
#include "Drawing.h"
#include "Screen.h"
#include "GameSettings.h"
#include "Sandbox.h"

using namespace std;
using namespace TetrisVariables;
// Ruobin Chen
// Tetris game made with SFML 2.5.1
// Sound effects and music made with IOS Garage Band
// Line count as of 5/11/2024: 3937

// Generate text for loss screen
vector<sf::Text> getLossText(sf::Font& font) {
	vector<sf::Text> textboxes;
	textboxes.push_back(SfTextAtHome(font, WHITE, "YOU LOST", GAMETEXTSIZE * 4, { WIDTH / 2, GAMEYPOS }, true, false, true));
	textboxes.push_back(SfTextAtHome(font, WHITE, "Press any key to return to Main Menu", GAMETEXTSIZE, { WIDTH / 2, HEIGHT / 2 }, true, false, true));

	return textboxes;
}

// Load in all clip timestamps
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
	bgm.setVolume(BGMVOLUME);
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

	// Text for loss screen
	vector<sf::Text> lossText = getLossText(font);
#pragma endregion

#pragma region Controller Classes
	// Initialize controls with default keybinds
	KeySet* playerSoloKeys = new KeySet(LEFT, RIGHT, UP, DOWN, SPINCW, SPINCCW, HOLD);
	KeySet* player1Keys = new KeySet(LEFT1, RIGHT1, UP1, DOWN1, SPINCW1, SPINCCW1, HOLD1);
	KeySet* player2Keys = new KeySet(LEFT2, RIGHT2, UP2, DOWN2, SPINCW2, SPINCCW2, HOLD2);

	// Initiate DAS keys
	KeyDAS* playerSoloDAS = new KeyDAS(DASDELAY, DASSPEED, playerSoloKeys);
	KeyDAS* player1DAS = new KeyDAS(DASDELAY, DASSPEED, player1Keys);
	KeyDAS* player2DAS = new KeyDAS(DASDELAY, DASSPEED, player2Keys);
#pragma endregion

	// Initiate piece rng
	PieceBag bag;

	// Set up game screen
	Screen* screen = new Screen(window, GAMEPOS, font, &texture, &bag, soundFX);
	Screen* screenP2 = new Screen(window, GAMEPOSP2, font, &texture, &bag, soundFX);
	screenP2->setGamemodeTextString("PVP Mode"); // This will be the title text used in pvp mode. Hide the other title text
	screenP2->setGamemodeTextXPos(WIDTH);
	
	sf::Text linesClearedText = SfTextAtHome(font, WHITE, "Lines: 0", 25, { GAMEXPOS + GAMEWIDTH + 150, GAMEYPOS });
	int currentScreen = MAINMENU;

	// Pause screen sprites
	vector<string> pauseMenuText = { "Continue", "Restart", "Quit" };
	PauseScreen pauseMenu(GAMEPOS, pauseMenuText, font);
	// Sandbox mode exclusive sprites
	SandboxMenu* sandboxMenu = new SandboxMenu(font, screen);
	// Set up settings menu
	SettingsMenu gameSettings({ screen, screenP2 }, { playerSoloDAS, player1DAS, player2DAS }, soundFX, font, &bgm, &currentScreen);

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
					if (gameMenu.onMouseMove(event.mouseMove.x, event.mouseMove.y))
						soundFX->play(LIGHTTAP);
					break;
				}
				case sf::Event::MouseButtonPressed: {
					if (gameMenu.onMouseClick(event.mouseButton.x, event.mouseButton.y))
						modeSelected = true;
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
					sandboxMenu->reset();
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
				window.draw(pauseMenu);
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
							pauseMenu.getMenu().moveDown();
						}
						else if (event.key.code == sf::Keyboard::Up) {
							soundFX->play(LIGHTTAP);
							pauseMenu.getMenu().moveUp();
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
						pauseMenu.getMenu().resetCursorPos();
						soundFX->play(MEDIUMBEEP);
					}
					break;
				}
				case sf::Event::KeyReleased: {
					playerSoloDAS->releaseKey(event.key.code);
				}
				case sf::Event::MouseButtonPressed: {
					if (screen->getPaused() && pauseMenu.getMenu().onMouseClick(event.mouseButton.x, event.mouseButton.y))
						modeSelected = true;
					break;
				}
				case sf::Event::MouseMoved: {
					if (screen->getPaused() && pauseMenu.getMenu().onMouseMove(event.mouseMove.x, event.mouseMove.y))
						soundFX->play(LIGHTTAP);
					break;
				}
				default:
					break;
				}
			}
			// Select menu option if modeSelected is true
			if (modeSelected) {
				switch (pauseMenu.getMenu().getCursorPos())
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
				pauseMenu.getMenu().resetCursorPos();
			}
		}
		// Run on sandbox mode
		else if (currentScreen == SANDBOX) {
			window.clear(BLUE);
			screen->drawScreen();
			window.draw(*sandboxMenu);

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
					else // Check for sandbox hotkeys
						sandboxMenu->onKeyPress(event.key.code, bag, bgm, currentScreen);
					break;
				}
				case sf::Event::KeyReleased: {
					playerSoloDAS->releaseKey(event.key.code);
				}
				case sf::Event::MouseButtonPressed: {
					sf::Vector2f clickPos(event.mouseButton.x, event.mouseButton.y);
					if (event.mouseButton.button == sf::Mouse::Left) {
						if (screen->getGameBounds().contains(clickPos))  // Creative mode click
							screen->clickBlock(clickPos); // Will do nothing if creative mode isn't on
						sandboxMenu->onLeftClick(clickPos, bag, bgm, currentScreen);
					}
					else if (event.mouseButton.button == sf::Mouse::Right && screen->getGameBounds().contains(clickPos)) {
						screen->clickRow(clickPos); // Creative mode right click
					}
					break;
				}
				case sf::Event::MouseMoved: { // Shows a transparent X when hovering over unchecked boxes
					sf::Vector2f clickPos(event.mouseMove.x, event.mouseMove.y);
					sandboxMenu->onMouseMove(clickPos);
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
				window.draw(pauseMenu);
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
							pauseMenu.getMenu().moveDown();
						}
						else if (event.key.code == sf::Keyboard::Up) {
							soundFX->play(LIGHTTAP);
							pauseMenu.getMenu().moveUp();
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
							pauseMenu.getMenu().resetCursorPos();
						}
					}
					break;
				}
				case sf::Event::KeyReleased: {
					player1DAS->releaseKey(event.key.code);
					player2DAS->releaseKey(event.key.code);
				}
				case sf::Event::MouseButtonPressed: {
					if (screen->getPaused() && pauseMenu.getMenu().onMouseClick(event.mouseButton.x, event.mouseButton.y))
						modeSelected = true;
					break;
				}
				case sf::Event::MouseMoved: {
					if (screen->getPaused() && pauseMenu.getMenu().onMouseMove(event.mouseMove.x, event.mouseMove.y))
						soundFX->play(LIGHTTAP);
					break;
				}
				default:
					break;
				}
			}

			// Select menu option if modeSelected is true
			if (modeSelected) {
				switch (pauseMenu.getMenu().getCursorPos())
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
				pauseMenu.getMenu().resetCursorPos();
			}
		}
		else if (currentScreen == LOSESCREEN) {
			window.clear(BLACK);
			for (sf::Text& text : lossText)
				window.draw(text);

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
					gameSettings.onMouseMove(event.mouseMove.x, event.mouseMove.y);
					break;
				case sf::Event::MouseButtonPressed:
					gameSettings.onMouseClick(event.mouseButton.x, event.mouseButton.y);
					break;
				case sf::Event::MouseButtonReleased:
					gameSettings.onMouseRelease();
					break;
				default:
					break;
				}
			}
		}
		window.display();
	}

	// Cleanup
	delete sandboxMenu;
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
