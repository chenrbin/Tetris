#include <iostream>
#include <SFML/Graphics.hpp>
#include "Screen.h"
#include "TetrisConstants.h"
#include <map>
#include "Drawing.h"
#include "Mechanisms.h"

using namespace std;
using namespace TetrisVariables;
// Ruobin Chen
// Line count as of 7/31/2023: 3118

// Generate centered text entity. Can specify font, color, message, size, position, and style
sf::Text generateText(sf::Font& font, sf::Color color, string message, unsigned int textSize, sf::Vector2f coords, bool bold = true, bool underlined = false, bool centered = false) {
	sf::Text text;
	text.setFont(font);
	text.setFillColor(color);
	text.setString(message);
	text.setCharacterSize(textSize);
	const sf::FloatRect box = text.getLocalBounds();
	if (centered)
		text.setOrigin(box.width / 2.0f, box.height / 2.0f);
	text.setPosition(coords);
	if (bold)
		text.setStyle(sf::Text::Bold);
	if (underlined)
		text.setStyle(sf::Text::Underlined);
	return text;
}
// Generate rectangle entity. Can specify dimensions, color, position, origin, outline
sf::RectangleShape generateRectangle(sf::Vector2f dimensions, sf::Color fillColor, sf::Vector2f position, sf::Vector2f origin = { 0, 0 }, sf::Color outlineColor = sf::Color(), float outlineThickness = 0)
{
	sf::RectangleShape rect(dimensions);
	rect.setFillColor(fillColor);
	rect.setPosition(position);
	rect.setOrigin(origin);
	rect.setOutlineColor(outlineColor);
	rect.setOutlineThickness(outlineThickness);
	return rect;
}

// Generate vector of preset game screen rectangles
vector<sf::RectangleShape> getGameRects(sf::Vector2f gamePos) {
	// Rectangles for game screen, hold, queue, garbage bin, and top row, 
	vector<sf::RectangleShape> rects;
	rects.push_back(generateRectangle({ GAMEWIDTH, GAMEHEIGHT }, BLACK,
		gamePos, { 0, 0 }, WHITE, LINEWIDTH));
	rects.push_back(generateRectangle({ TILESIZE * 4, TILESIZE * 4  }, BLACK,
		{ gamePos.x - TILESIZE * 4.5f - LINEWIDTH, gamePos.y + LINEWIDTH }, { 0,  0 }, WHITE, LINEWIDTH));
	rects.push_back(generateRectangle({ TILESIZE * 4, GAMEHEIGHT / 9 * NEXTPIECECOUNT },
		BLACK, { gamePos.x + GAMEWIDTH + LINEWIDTH, gamePos.y + LINEWIDTH }, { 0, 0 }, WHITE, LINEWIDTH));
	rects.push_back(generateRectangle({ TILESIZE / 2, GAMEHEIGHT - LINEWIDTH }, BLACK,
		{ gamePos.x - TILESIZE / 2 - LINEWIDTH, gamePos.y + LINEWIDTH }, { 0, 0 }, WHITE, LINEWIDTH));

	// Rectangles to show a couple pixels of the very top row
	rects.push_back(generateRectangle({ GAMEWIDTH, TOPROWPIXELS }, BLACK,
		{ gamePos.x, gamePos.y - TOPROWPIXELS }, { 0, 0 }, WHITE, LINEWIDTH));
	rects.push_back(generateRectangle({ GAMEWIDTH + 1, TILESIZE - 9 }, BLUE,
		{ gamePos.x - 1, gamePos.y - TILESIZE - 1 }));
	return rects;
}
// Get bounds of first three rectangles (game, hold, queue)
vector<sf::FloatRect> getGameBounds(vector<sf::RectangleShape>& rects) {
	vector<sf::FloatRect> bounds;
	// { gameBounds, holdBounds, queueBounds }
	for (int i = 0; i < 3; i++)
		bounds.push_back(rects[i].getGlobalBounds());
	return bounds;
}
// Generate a vector of numRows x numCols lines across the specified bounds
vector<sf::RectangleShape> getLines(sf::FloatRect& bounds) {
	vector<sf::RectangleShape> lines;
	for (int i = 1; i < NUMROWS; i++) { // Horizontal lines
		sf::RectangleShape line({ GAMEWIDTH, LINEWIDTH });
		line.setPosition(bounds.left, bounds.top + i * TILESIZE - 1);
		line.setFillColor(SEETHROUGH);
		lines.push_back(line);
	}
	for (int j = 1; j < NUMCOLS; j++) { // Vertical lines
		sf::RectangleShape line({ LINEWIDTH, GAMEHEIGHT + TOPROWPIXELS });
		line.setPosition(bounds.left + j * TILESIZE - 1, bounds.top - TOPROWPIXELS);
		line.setFillColor(SEETHROUGH);
		lines.push_back(line);
	}
	return lines;
}

// Generate all static text on game screen
vector<sf::Text> getGameText(vector<sf::FloatRect>& bounds, sf::Font& font) {
	// Redefine bounds for cleaner code
	sf::FloatRect& gameBounds = bounds[0], holdBounds = bounds[1], queueBounds = bounds[2];
	vector<sf::Text> textboxes;
	textboxes.push_back(generateText(font, WHITE, "Hold", GAMETEXTSIZE, { holdBounds.left + holdBounds.width / 2, holdBounds.top - GAMETEXTSIZE }, true, false, true));
	textboxes.push_back(generateText(font, WHITE, "Next", GAMETEXTSIZE, { queueBounds.left + queueBounds.width / 2, queueBounds.top - GAMETEXTSIZE }, true, false, true));
	textboxes.push_back(generateText(font, WHITE, "Classic Mode", GAMETEXTSIZE * 2, { gameBounds.left + gameBounds.width / 2, gameBounds.top - GAMETEXTSIZE * 2 }, true, false, true));
	return textboxes;
}
// Generate text exclusive to sandbox mode
vector<sf::Text> getSandboxText(sf::Font& font) {
	vector<sf::Text> textboxes;
	vector<string> menuItems = { "Auto-fall", "Fall speed", "Creative", "Reset", "Quit" };
	for (int i = 0; i < menuItems.size(); i++)
		textboxes.push_back(generateText(font, WHITE, menuItems[i], MENUTEXTSIZE, { SANDBOXMENUPOS.x, SANDBOXMENUPOS.y + MENUSPACING * i }));
	return textboxes;
}
// Generate text for loss screen
vector<sf::Text> getLossText(sf::Font& font) {
	vector<sf::Text> textboxes;
	textboxes.push_back(generateText(font, WHITE, "YOU LOST", GAMETEXTSIZE * 4, { WIDTH / 2, GAMEYPOS }, true, false, true));
	textboxes.push_back(generateText(font, WHITE, "Press any key to return to Main Menu", GAMETEXTSIZE, { WIDTH / 2, HEIGHT / 2 }, true, false, true));

	return textboxes;
}
// Generate fadeText animations
vector<FadeText> getFadeText(sf::Vector2f gamePos, sf::Font& font) {
	FadeText speedupText(generateText(font, WHITE, "SPEED UP", GAMETEXTSIZE * 2, { gamePos.x + GAMEWIDTH / 2, gamePos.y }, true, false, true), 1, 1);
	FadeText clearText(generateText(font, WHITE, "T-spin Triple", CLEARTEXTSIZE, { gamePos.x + GAMEWIDTH + LINEWIDTH * 2, gamePos.y + GAMEHEIGHT / 1.5f }), 0, 2.5f);
	FadeText b2bText(generateText(font, WHITE, "Back-to-Back", CLEARTEXTSIZE, { gamePos.x + GAMEWIDTH + LINEWIDTH * 2, gamePos.y + GAMEHEIGHT / 1.5f + MENUSPACING }), 0, 2.5f);
	FadeText comboText(generateText(font, WHITE, "2X Combo", CLEARTEXTSIZE, { gamePos.x + GAMEWIDTH + LINEWIDTH * 2, gamePos.y + GAMEHEIGHT / 1.5f + MENUSPACING * 2 }), 0, 2.5f);
	FadeText allClearText(generateText(font, WHITE, "All Clear", CLEARTEXTSIZE, { gamePos.x + GAMEWIDTH + LINEWIDTH * 2, gamePos.y + GAMEHEIGHT / 1.5f + MENUSPACING * 3 }), 0, 2.5f);
	return { speedupText, clearText, b2bText, comboText, allClearText };
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
	return keyStrings;
}


// Following functions are made for reuseability and requires specific parameters passed in.
// Functionality of the toggles in sandbox mode. 
void toggleGravity(Checkbox& autoFallBox, Screen* screen) {
	if (autoFallBox.getChecked()) {
		autoFallBox.setChecked(false);
		screen->setAutoFall(false);
	}
	else {
		autoFallBox.setChecked(true);
		screen->setAutoFall(true);
	}
}
void decrementGravity(Checkbox& gravityBox, Checkbox& autoFallBox, Screen* screen) {
	gravityBox.decrement();
	screen->setGravity(GRAVITYSPEEDS[gravityBox.getCurrentNum() - 1]);
}
void incrementGravity(Checkbox& gravityBox, Checkbox& autoFallBox, Screen* screen) {
	gravityBox.increment();
	screen->setGravity(GRAVITYSPEEDS[gravityBox.getCurrentNum() - 1]);
}
void toggleCreative(Checkbox& creativeModeBox, Checkbox& autoFallBox, Screen* screen) {
	if (creativeModeBox.getChecked()) { // If box is already checked, turn off
		creativeModeBox.setChecked(false);
		autoFallBox.setChecked(true);
		screen->setAutoFall(true);
		screen->endCreativeMode();
	}
	else { // If box isn't checked, turn on
		creativeModeBox.setChecked(true);
		autoFallBox.setChecked(false);
		screen->setAutoFall(false);
		screen->startCreativeMode();
	}
}

// Draw all objects in a vector
void drawVector(sf::RenderWindow& window, vector<sf::RectangleShape>& vec) {
	for (sf::RectangleShape& item : vec)
		window.draw(item);
}
void drawVector(sf::RenderWindow& window, vector<sf::Text>& vec) {
	for (sf::Text& item : vec)
		window.draw(item);
}
void drawVector(sf::RenderWindow& window, vector<FadeText>& vec) {
	for (FadeText& animation : vec)
		animation.drawAnimation(window);
}
void drawVector(sf::RenderWindow& window, vector<Animation*>& vec) {
	for (Animation* animation : vec)
		animation->drawAnimation(window);
}

int main(){
	srand(time(NULL));
	// Set SFML objects
	sf::ContextSettings windowSettings;
	windowSettings.antialiasingLevel = 8;
	sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Tetris", sf::Style::Close | sf::Style::Titlebar, windowSettings);
	window.setFramerateLimit(FPS);
	window.setKeyRepeatEnabled(false);

	sf::Font font;
	if (!font.loadFromFile("font.ttf")) {
		cout << "Font not found\n";
		throw exception();
	}
	sf::Texture texture;
	if (!texture.loadFromFile("images/tile_hidden.png"))
	{
		cout << "File not found\n";
		throw exception();
	}

	// Title screen sprites
	sf::Text titleText(generateText(font, WHITE, "TETRIS", 150, TITLETEXTPOS, true, false, true));

	sf::CircleShape cursor(15.f, 3); // Triangle shaped cursor
	cursor.rotate(90.f);
	vector<string> menuText = { "Classic Mode", "Sandbox Mode", "PVP Mode", "Settings", "Quit" };
	ClickableMenu gameMenu(font, WHITE, menuText, MENUTEXTSIZE, MENUPOS, MENUSPACING, cursor);

	// Game sprites in a vector and constructed in a separate method to keep main clean
	vector<sf::RectangleShape> gameScreenRectangles = getGameRects(GAMEPOS); 
	vector<sf::FloatRect> gameScreenBounds = getGameBounds(gameScreenRectangles); 
	vector<sf::Text> gameText = getGameText(gameScreenBounds, font);
	vector<sf::RectangleShape> lines = getLines(gameScreenBounds[0]);
	sf::Text linesClearedText = generateText(font, WHITE, "Lines: 0", 25, { GAMEXPOS + GAMEWIDTH + 150, GAMEYPOS });

	// Sandbox mode exclusive sprites
	vector<sf::Text> sandboxText = getSandboxText(font);
	vector<Checkbox*> sandboxes; // { autoFallBox, gravityBox, creativeModeBox, resetBox, quitBox }
	sandboxes.push_back(new Checkbox(TILESIZE, SANDBOXMENUPOS.x + 180, SANDBOXMENUPOS.y, true, font));
	sandboxes.push_back(new IncrementalBox (TILESIZE, SANDBOXMENUPOS.x + 180, SANDBOXMENUPOS.y + MENUSPACING, 1, GRAVITYTIERCOUNT, font));
	sandboxes.push_back(new Checkbox (TILESIZE, SANDBOXMENUPOS.x + 180, SANDBOXMENUPOS.y + MENUSPACING * 2, false, font));
	sandboxes.push_back(new Checkbox (TILESIZE, SANDBOXMENUPOS.x + 180, SANDBOXMENUPOS.y + MENUSPACING * 3, false, font));
	sandboxes.push_back(new Checkbox (TILESIZE, SANDBOXMENUPOS.x + 180, SANDBOXMENUPOS.y + MENUSPACING * 4, false, font));
	bool creativeModeOn = false;

	// Text for loss screen
	vector<sf::Text> lossText = getLossText(font);

	// Initiate animation classes
	vector<FadeText> clearAnimations = getFadeText(GAMEPOS, font);
	vector<FadeText> clearAnimationsP2 = getFadeText(GAMEPOSP2, font); 

	// Get player two assets
	vector<sf::RectangleShape> gameScreenRectanglesP2 = getGameRects(GAMEPOSP2);
	vector<sf::FloatRect> gameScreenBoundsP2 = getGameBounds(gameScreenRectanglesP2);
	vector<sf::RectangleShape> linesP2 = getLines(gameScreenBoundsP2[0]);
	vector<sf::Text> gameTextP2 = getGameText(gameScreenBoundsP2, font);
	gameTextP2[2].setString("PVP Mode"); // This will be the title text used in pvp mode. Hide the other title text
	gameTextP2[2].setPosition(WIDTH, gameTextP2[2].getPosition().y);

	// Initialize controls.
	KeySet playerSoloKeys(LEFT, RIGHT, UP, DOWN, SPINCW, SPINCCW, HOLD);
	KeySet player1Keys(LEFT1, RIGHT1, UP1, DOWN1, SPINCW1, SPINCCW1, HOLD1);
	KeySet player2Keys(LEFT2, RIGHT2, UP2, DOWN2, SPINCW2, SPINCCW2, HOLD2);

	// Initiate DAS keys
	KeyDAS playerSoloDAS(170, 50, &playerSoloKeys);
	KeyDAS player1DAS(170, 50, &player1Keys);
	KeyDAS player2DAS(170, 50, &player2Keys);

	// Initiate piece rng
	pieceBag bag;
	
	// Set up game screen
	Screen* screen = new Screen(window, gameScreenBounds, texture, &clearAnimations, &bag);
	Screen* screenP2 = new Screen(window, gameScreenBoundsP2, texture, &clearAnimationsP2, &bag);
	int currentScreen = MAINMENU;

	// WIP Pause screen sprites
	pauseScreen classicPause(GAMEPOS, font);

	// Set up settings sprites
	SettingsMenu gameSettings;
	gameSettings.addTab(font, "Gameplay");
	gameSettings.addTab(font, "Controls");
	gameSettings.addTab(font, "Graphics");
	gameSettings.selectTab(0);

	vector<sf::Vector2f> settingPositions;
	vector<OptionSelector*> selectors;
	vector<sf::Vector2f> selectorPositions;
	for (float i = 0; i < 7; i++) {
		settingPositions.push_back({ SETTINGXPOS, SETTINGYPOS + SETTINGSPACING * i });
		selectorPositions.push_back({ SETTINGXPOS + SELECTORRIGHTSPACING, SETTINGYPOS + SETTINGSPACING * i });
	}
	selectors.push_back(new SlidingBar(250, { "Easy", "Normal", "Hard", "Custom"}, font));
	gameSettings[0].addSetting("Difficulty", settingPositions[0], selectors[0], selectorPositions[0], font);
	
	selectors.push_back(new SlidingBar(250, { "10", "20", "50", "100" }, font));
	gameSettings[0].addSetting("Lock Delay", settingPositions[1], selectors[1], selectorPositions[1], font);
	
	selectors.push_back(new SlidingBar (250, { "0", "1", "2", "3", "4", "5", "6"}, font));
	gameSettings[0].addSetting("Piece Preview", settingPositions[2], selectors[2], selectorPositions[2], font);

	selectors.push_back(new OnOffSwitch(font));
	gameSettings[0].addSetting("Ghost Piece", settingPositions[3], selectors[3], selectorPositions[3], font);

	ClickableButton defaultSettingsButton({ 230, 40 }, { 150, 700 }, font, "Reset Defaults", BUTTONTEXTSIZE, RED);
	ClickableButton saveQuitSettingsButton({ 230, 40 }, { 400, 700 }, font, "Save & Quit", BUTTONTEXTSIZE, RED);
	ClickableButton discardQuitSettingsButton({ 230, 40 }, { 650, 700 }, font, "Discard & Quit", BUTTONTEXTSIZE, RED);	
	
	map<sf::Keyboard::Key, string> keyStrings = getKeyStrings();

	KeyRecorder ke(&keyStrings, font);
	printLine(&windowSettings);
	printLine(&linesClearedText);
	printLine(&creativeModeOn);
	printLine(&screen);
	printLine(&ke);
	// Game loop
	while (window.isOpen())
	{
		// Run on main menu
		if (currentScreen == MAINMENU) {
			window.clear(BLUE);
			window.draw(titleText);
			window.draw(gameMenu);
			window.draw(ke);

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
					ke.readKey(event.key.code);
					if (event.key.code == sf::Keyboard::Down) 
						gameMenu.moveDown();
					else if (event.key.code == sf::Keyboard::Up) 
						gameMenu.moveUp();
					else if (event.key.code == sf::Keyboard::Z) 
						modeSelected = true;
					break;
				}
				case sf::Event::MouseMoved: {
					gameMenu.updateMouse(event.mouseMove.x, event.mouseMove.y);
					break;
				}
				case sf::Event::MouseButtonPressed: {
					if (gameMenu.updateMouse(event.mouseButton.x, event.mouseButton.y))
						modeSelected = true;
					break;
				}
				case sf::Event::MouseButtonReleased: {
					break;
				}
				case sf::Event::TextEntered: {
					// Prevents entering characters that require shift like '!'. Event.key.shift does not seem to work.
					if (!sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) && !sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
						ke.readKey(event.text.unicode);
				}
				default:
					break;
				}

			}

			// Select menu option if modeSelected is true
			if (modeSelected) {
				switch (gameMenu.getCursorPos())
				{
				case 0: // Classic mode
					currentScreen = CLASSIC;
					screen->setGameMode(CLASSIC);
					gameText[2].setString("Classic Mode");
					screen->setAutoFall(true);
					screen->endCreativeMode();
					bag.resetQueue();
					screen->resetBoard();
					break;
				case 1: // Sandbox mode
					currentScreen = SANDBOX;
					gameText[2].setString("Sandbox Mode");
					screen->setGameMode(SANDBOX);
					// Reset sandbox settings
					sandboxes[0]->setChecked(true);
					sandboxes[1]->setValue(sandboxes[1]->getMin());
					sandboxes[2]->setChecked(false);
					screen->setAutoFall(true);
					screen->endCreativeMode();
					bag.resetQueue();
					screen->resetBoard();
					break;
				case 2: // PVP mode
					currentScreen = MULTIPLAYER;
					window.setSize({ WIDTH * 2, HEIGHT });
					window.setView(sf::View(sf::FloatRect(0, 0, WIDTH * 2, HEIGHT)));
					window.setPosition({ 100, 100 });
					screen->setGameMode(MULTIPLAYER);
					gameText[2].setString("");
					screen->setAutoFall(true);
					screen->endCreativeMode();
					bag.resetQueue();
					screen->resetBoard();

					screenP2->setGameMode(MULTIPLAYER);
					screenP2->resetBoard();
					break;
				case 3: // Settings
					currentScreen = SETTINGSCREEN;
					break;
				case 4: // Quit
					window.close();
					return 0;
				default:
					break;
				}
			}
		}
		// Run on classic mode
		else if (currentScreen == CLASSIC) {
			window.clear(BLUE);
			drawVector(window, gameScreenRectangles);
			drawVector(window, lines);
			screen->drawScreen();
			window.draw(gameScreenRectangles.back()); // Redraw last rectangle
			drawVector(window, gameText);
			drawVector(window, clearAnimations);

			// This is only shown in classic mode
			linesClearedText.setString("Lines: " + to_string(screen->getLinesCleared()));
			window.draw(linesClearedText);
			
			if (screen->getPaused() && !screen->getGameOver())
				window.draw(classicPause);
			// In-game timer events
			screen->doTimeStuff();

			// Check for game over
			if (screen->getGameOver() && screen->isDeathAnimationOver()) {
				currentScreen = LOSESCREEN;
				lossText[0] = generateText(font, WHITE, "YOU LOST", GAMETEXTSIZE * 4, { WIDTH / 2, GAMEYPOS }, true, false, true);
			}

			// Handles movement with auto-repeat (DAS)
			playerSoloDAS.checkKeyPress(screen);

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
					if (event.key.code == playerSoloKeys.getUp())
						screen->movePiece(3);
					else if (event.key.code == playerSoloKeys.getSpinCCW())
						screen->spinPiece(false);
					else if (event.key.code == playerSoloKeys.getSpinCW())
						screen->spinPiece(true);
					else if (event.key.code == playerSoloKeys.getHold())
						screen->holdPiece();
					else if (event.key.code == sf::Keyboard::Escape)
						screen->doPauseResume();
					break;
				}
				case sf::Event::KeyReleased: {
					playerSoloDAS.releaseKey(event.key.code);
				}
				case sf::Event::MouseButtonPressed: {
					break;
				}
				default:
					break;
				}
			}
		}
		// Run on sandbox mode
		else if (currentScreen == SANDBOX) {
			window.clear(BLUE);
			for (int i = 0; i < gameScreenRectangles.size() - 1; i++) 
				window.draw(gameScreenRectangles[i]);
			drawVector(window, gameScreenRectangles);
			drawVector(window, lines);
			screen->drawScreen();
			window.draw(gameScreenRectangles.back()); // Redraw last rectangle
			drawVector(window, gameText);
			drawVector(window, sandboxText);
			for (auto box : sandboxes)
				window.draw(*box);
			
			// In-game timer events
			screen->doTimeStuff();
			// Handles movement with auto-repeat (DAS)
			playerSoloDAS.checkKeyPress(screen);

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
					if (event.key.code == playerSoloKeys.getUp())
						screen->movePiece(3);
					else if (event.key.code == playerSoloKeys.getSpinCCW())
						screen->spinPiece(false);
					else if (event.key.code == playerSoloKeys.getSpinCW())
						screen->spinPiece(true);
					else if (event.key.code == playerSoloKeys.getHold())
						screen->holdPiece();
					else if (event.key.code == sf::Keyboard::Num1)
						screen->spawnPiece(0);
					else if (event.key.code == sf::Keyboard::Num2)
						screen->spawnPiece(1);
					else if (event.key.code == sf::Keyboard::Num3)
						screen->spawnPiece(2);
					else if (event.key.code == sf::Keyboard::Num4)
						screen->spawnPiece(3);
					else if (event.key.code == sf::Keyboard::Num5)
						screen->spawnPiece(4);
					else if (event.key.code == sf::Keyboard::Num6)
						screen->spawnPiece(5);
					else if (event.key.code == sf::Keyboard::Num7)
						screen->spawnPiece(6);
						// Hot keys for sandbox controls
					else if (event.key.code == sf::Keyboard::Q)
						toggleGravity(*sandboxes[0], screen);
					else if (event.key.code == sf::Keyboard::W)
						decrementGravity(*sandboxes[1], *sandboxes[0], screen);
					else if (event.key.code == sf::Keyboard::S)
						incrementGravity(*sandboxes[1], *sandboxes[0], screen);
					else if (event.key.code == sf::Keyboard::E)
						toggleCreative(*sandboxes[2], *sandboxes[0], screen);
					else if (event.key.code == sf::Keyboard::R) {
						bag.resetQueue();
						screen->resetBoard();
					}
					else if (event.key.code == sf::Keyboard::T)
						currentScreen = MAINMENU;
					else if (event.key.code == sf::Keyboard::G) 
						screen->receiveGarbage(1);
					else if (event.key.code == sf::Keyboard::H) 
						screen->receiveGarbage(4);
					else if (event.key.code == sf::Keyboard::Escape)
						screen->doPauseResume();
					break;
				}
				case sf::Event::KeyReleased: {
					playerSoloDAS.releaseKey(event.key.code);
				}
				case sf::Event::MouseButtonPressed: {
					sf::Vector2f clickPos(event.mouseButton.x, event.mouseButton.y);
					if (event.mouseButton.button == sf::Mouse::Left) {
						if (sandboxes[0]->getBounds().contains(clickPos))  // Turn off gravity
							toggleGravity(*sandboxes[0], screen);
						else if (sandboxes[1]->getLeftBound().contains(clickPos))  // Speed arrows
							decrementGravity(*sandboxes[1], *sandboxes[0], screen);
						else if (sandboxes[1]->getRightBound().contains(clickPos))
							incrementGravity(*sandboxes[1], *sandboxes[0], screen);
						else if (sandboxes[2]->getBounds().contains(clickPos)) // Click creative mode
							toggleCreative(*sandboxes[2], *sandboxes[0], screen);
						else if (gameScreenBounds[0].contains(clickPos))  // Creative mode click
							screen->clickBlock(clickPos); // Will do nothing if creative mode isn't on
						else if (sandboxes[3]->getBounds().contains(clickPos)) {
							bag.resetQueue();
							screen->resetBoard();
						}
						else if (sandboxes[4]->getBounds().contains(clickPos)) // Return to menu
							currentScreen = MAINMENU;
					}
					else if (event.mouseButton.button == sf::Mouse::Right && gameScreenBounds[0].contains(clickPos)) {
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
			drawVector(window, gameScreenRectangles);
			drawVector(window, lines);
			screen->drawScreen();
			window.draw(gameScreenRectangles.back()); // Redraw last rectangle
			drawVector(window, gameText);
			drawVector(window, clearAnimations);

			drawVector(window, gameScreenRectanglesP2);
			drawVector(window, linesP2);
			screenP2->drawScreen();
			window.draw(gameScreenRectanglesP2.back());
			drawVector(window, gameTextP2);
			drawVector(window, clearAnimationsP2);


			// In-game timer events
			screen->doTimeStuff();
			screenP2->doTimeStuff();
			// Process garbage exchange
			screen->receiveGarbage(screenP2->getOutGarbage());
			screenP2->receiveGarbage(screen->getOutGarbage());

			// Check for game over
			if (screen->getGameOver()) {
				screenP2->pauseGame();
				if (screen->isDeathAnimationOver()) {
					window.setSize({ WIDTH, HEIGHT });
					window.setView(sf::View(sf::FloatRect(0, 0, WIDTH, HEIGHT)));
					currentScreen = LOSESCREEN;
					lossText[0] = generateText(font, WHITE, "PLAYER 2 WINS!", GAMETEXTSIZE * 4, { WIDTH / 2, GAMEYPOS }, true, false, true);
					if (screenP2->getGameOver() && screenP2->isDeathAnimationOver()) // Rare event if both players lose at the same time
						lossText[0] = generateText(font, WHITE, "DRAW!", GAMETEXTSIZE * 4, { WIDTH / 2, GAMEYPOS }, true, false, true);
				}
			}
			else if (screenP2->getGameOver()) {
				screen->pauseGame();
				if (screenP2->isDeathAnimationOver()) {
					window.setSize({ WIDTH, HEIGHT });
					window.setView(sf::View(sf::FloatRect(0, 0, WIDTH, HEIGHT)));
					currentScreen = LOSESCREEN;
					lossText[0] = generateText(font, WHITE, "PLAYER 1 WINS!", GAMETEXTSIZE * 4, { WIDTH / 2, GAMEYPOS }, true, false, true);
				}
			}

			// Handles movement with auto-repeat (DAS)
			player1DAS.checkKeyPress(screen);
			player2DAS.checkKeyPress(screenP2);

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
					if (event.key.code == player1Keys.getUp())
						screen->movePiece(3);
					else if (event.key.code == player1Keys.getSpinCCW())
						screen->spinPiece(false);
					else if (event.key.code == player1Keys.getSpinCW())
						screen->spinPiece(true);
					else if (event.key.code == player1Keys.getHold())
						screen->holdPiece();
					else if (event.key.code == player2Keys.getUp())
						screenP2->movePiece(3);
					else if (event.key.code == player2Keys.getSpinCCW())
						screenP2->spinPiece(false);
					else if (event.key.code == player2Keys.getSpinCW())
						screenP2->spinPiece(true);
					else if (event.key.code == player2Keys.getHold())
						screenP2->holdPiece();
					else if (event.key.code == sf::Keyboard::Escape) {
						screen->doPauseResume();
						screenP2->doPauseResume();
					}
					break;
				}
				case sf::Event::KeyReleased: {
					player1DAS.releaseKey(event.key.code);
					player2DAS.releaseKey(event.key.code);
				}
				case sf::Event::MouseButtonPressed: {
					break;
				}
				default:
					break;
				}
			}
		}
		else if (currentScreen == LOSESCREEN) {
			window.clear(BLACK);
			drawVector(window, lossText);
			sf::Event event;
			while (window.pollEvent(event)) {
				switch (event.type)
				{
				case sf::Event::Closed: 
					window.close();
					break; 
				case sf::Event::KeyPressed: 
					currentScreen = MAINMENU;
					break;
				case sf::Event::MouseButtonPressed: 
					currentScreen = MAINMENU;
					break;
				default:
					break;
				}
			}
		}
		else if (currentScreen == SETTINGSCREEN) {
			window.clear(BLUE);
			window.draw(gameSettings);
			window.draw(defaultSettingsButton);
			window.draw(saveQuitSettingsButton);
			window.draw(discardQuitSettingsButton);
			sf::Event event;
			while (window.pollEvent(event)) {
				switch (event.type)
				{
				case sf::Event::Closed:
					window.close();
					break;
				case sf::Event::KeyPressed:
					break;
				case sf::Event::MouseMoved:
					gameSettings.processMouseMove(event.mouseMove.x, event.mouseMove.y);
					break;
				case sf::Event::MouseButtonPressed:
					gameSettings.processMouseClick(event.mouseButton.x, event.mouseButton.y);
					if (discardQuitSettingsButton.checkClick(event.mouseButton.x, event.mouseButton.y))
						currentScreen = MAINMENU;
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
	return 0;
}
