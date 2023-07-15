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
sf::RectangleShape generateRectangle(sf::Vector2f dimensions, sf::Color fillColor, sf::Vector2f position, sf::Vector2f origin = sf::Vector2f(0, 0), sf::Color outlineColor = sf::Color(), int outlineThickness = 0)
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
	rects.push_back(generateRectangle(sf::Vector2f(GAMEWIDTH, GAMEHEIGHT), BLACK,
		gamePos, sf::Vector2f(0, 0), WHITE, LINEWIDTH));
	rects.push_back(generateRectangle(sf::Vector2f(GAMEWIDTH / 2.5, GAMEHEIGHT / 5), BLACK,
		sf::Vector2f(gamePos.x - GAMEWIDTH / 2.5 - LINEWIDTH - TILESIZE / 2, gamePos.y + LINEWIDTH), sf::Vector2f(0, 0), WHITE, LINEWIDTH));
	rects.push_back(generateRectangle(sf::Vector2f(GAMEWIDTH / 2.5, GAMEHEIGHT / 9 * NEXTPIECECOUNT),
		BLACK, sf::Vector2f(gamePos.x + GAMEWIDTH + LINEWIDTH, gamePos.y + LINEWIDTH), sf::Vector2f(0, 0), WHITE, LINEWIDTH));
	rects.push_back(generateRectangle(sf::Vector2f(TILESIZE / 2, GAMEHEIGHT - LINEWIDTH), BLACK,
		sf::Vector2f(gamePos.x - TILESIZE / 2 - LINEWIDTH, gamePos.y + LINEWIDTH), sf::Vector2f(0, 0), WHITE, LINEWIDTH));

	// Rectangles to show a couple pixels of the very top row
	rects.push_back(generateRectangle(sf::Vector2f(GAMEWIDTH, TOPROWPIXELS), BLACK,
		sf::Vector2f(gamePos.x, gamePos.y - TOPROWPIXELS), sf::Vector2f(0, 0), WHITE, LINEWIDTH));
	rects.push_back(generateRectangle(sf::Vector2f(GAMEWIDTH + 1, TILESIZE - 9), BLUE,
		sf::Vector2f(gamePos.x - 1, gamePos.y - TILESIZE - 1)));
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
		sf::RectangleShape line(sf::Vector2f(GAMEWIDTH, LINEWIDTH));
		line.setPosition(bounds.left, bounds.top + i * TILESIZE - 1);
		line.setFillColor(SEETHROUGH);
		lines.push_back(line);
	}
	for (int j = 1; j < NUMCOLS; j++) { // Vertical lines
		sf::RectangleShape line(sf::Vector2f(LINEWIDTH, GAMEHEIGHT + TOPROWPIXELS));
		line.setPosition(bounds.left + j * TILESIZE - 1, bounds.top - TOPROWPIXELS);
		line.setFillColor(SEETHROUGH);
		lines.push_back(line);
	}
	return lines;
}

// Generate all text on menu screen
vector<sf::Text> getMenuText(sf::Font& font) {
	vector<sf::Text> textboxes;
	textboxes.push_back(generateText(font, WHITE, "TETRIS", 150, TITLETEXTPOS, true, false, true));
	vector<string> menuItems = { "Classic Mode", "Sandbox Mode", "PVP Mode", "Settings", "Quit" };
	for (int i = 0; i < menuItems.size(); i++)
		textboxes.push_back(generateText(font, WHITE, menuItems[i], MENUTEXTSIZE, sf::Vector2f(MENUXPOS, MENUYPOS + MENUSPACING * i)));
	return textboxes;
}
// Generate all static text on game screen
vector<sf::Text> getGameText(vector<sf::FloatRect>& bounds, sf::Font& font) {
	// Redefine bounds for cleaner code
	sf::FloatRect& gameBounds = bounds[0], holdBounds = bounds[1], queueBounds = bounds[2];
	vector<sf::Text> textboxes;
	textboxes.push_back(generateText(font, WHITE, "Hold", GAMETEXTSIZE, sf::Vector2f(holdBounds.left + holdBounds.width / 2, holdBounds.top - GAMETEXTSIZE), true, false, true));
	textboxes.push_back(generateText(font, WHITE, "Next", GAMETEXTSIZE, sf::Vector2f(queueBounds.left + queueBounds.width / 2, queueBounds.top - GAMETEXTSIZE), true, false, true));
	textboxes.push_back(generateText(font, WHITE, "Classic Mode", GAMETEXTSIZE * 2, sf::Vector2f(gameBounds.left + gameBounds.width / 2, gameBounds.top - GAMETEXTSIZE * 2), true, false, true));
	return textboxes;
}
// Generate text exclusive to sandbox mode
vector<sf::Text> getSandboxText(sf::Font& font) {
	vector<sf::Text> textboxes;
	vector<string> menuItems = { "Auto-fall", "Fall speed", "Creative", "Reset", "Quit" };
	for (int i = 0; i < menuItems.size(); i++)
		textboxes.push_back(generateText(font, WHITE, menuItems[i], MENUTEXTSIZE, sf::Vector2f(SANDBOXMENUPOS.x, SANDBOXMENUPOS.y + MENUSPACING * i)));
	return textboxes;
}
// Generate text for loss screen
vector<sf::Text> getLossText(sf::Font& font) {
	vector<sf::Text> textboxes;
	textboxes.push_back(generateText(font, WHITE, "YOU LOST", GAMETEXTSIZE * 4, sf::Vector2f(WIDTH / 2, GAMEYPOS), true, false, true));
	textboxes.push_back(generateText(font, WHITE, "Press any key to return to Main Menu", GAMETEXTSIZE, sf::Vector2f(WIDTH / 2, HEIGHT / 2), true, false, true));

	return textboxes;
}
// Generate fadeText animations
vector<FadeText> getFadeText(sf::Vector2f gamePos, sf::Font& font) {
	FadeText speedupText(generateText(font, WHITE, "SPEED UP", GAMETEXTSIZE * 2, sf::Vector2f(gamePos.x + GAMEWIDTH / 2, gamePos.y), true, false, true), 1, 1);
	FadeText clearText(generateText(font, WHITE, "T-spin Triple", CLEARTEXTSIZE, sf::Vector2f(gamePos.x + GAMEWIDTH + LINEWIDTH * 2, gamePos.y + GAMEHEIGHT / 1.5)), 0, 2.5);
	FadeText b2bText(generateText(font, WHITE, "Back-to-Back", CLEARTEXTSIZE, sf::Vector2f(gamePos.x + GAMEWIDTH + LINEWIDTH * 2, gamePos.y + GAMEHEIGHT / 1.5 + MENUSPACING)), 0, 2.5);
	FadeText comboText(generateText(font, WHITE, "2X Combo", CLEARTEXTSIZE, sf::Vector2f(gamePos.x + GAMEWIDTH + LINEWIDTH * 2, gamePos.y + GAMEHEIGHT / 1.5 + MENUSPACING * 2)), 0, 2.5);
	FadeText allClearText(generateText(font, WHITE, "All Clear", CLEARTEXTSIZE, sf::Vector2f(gamePos.x + GAMEWIDTH + LINEWIDTH * 2, gamePos.y + GAMEHEIGHT / 1.5 + MENUSPACING * 3)), 0, 2.5);
	return { speedupText, clearText, b2bText, comboText, allClearText };
}
// Get collision bounds for checkboxes
vector<sf::FloatRect> getBoxBounds(vector<sf::RectangleShape>& rects) {
	vector<sf::FloatRect> bounds;
	for (auto& rect : rects)
		bounds.push_back(rect.getGlobalBounds());
	return bounds;
}

// Following functions are made for reuseability and requires specific parameters passed in.
// Functionality of the toggles in sandbox mode. 
void toggleGravity(Checkbox& autoFallBox, Screen& screen) {
	if (autoFallBox.getChecked()) {
		autoFallBox.setChecked(false);
		screen.setAutoFall(false);
	}
	else {
		autoFallBox.setChecked(true);
		screen.setAutoFall(true);
	}
}
void decrementGravity(IncrementalBox& gravityBox, Checkbox& autoFallBox, Screen& screen) {
	gravityBox.decrement();
	screen.setGravity(GRAVITYSPEEDS[gravityBox.getCurrentNum() - 1]);
}
void incrementGravity(IncrementalBox& gravityBox, Checkbox& autoFallBox, Screen& screen) {
	gravityBox.increment();
	screen.setGravity(GRAVITYSPEEDS[gravityBox.getCurrentNum() - 1]);
}
void toggleCreative(Checkbox& creativeModeBox, Checkbox& autoFallBox, Screen& screen) {
	if (creativeModeBox.getChecked()) { // If box is already checked, turn off
		creativeModeBox.setChecked(false);
		autoFallBox.setChecked(true);
		screen.setAutoFall(true);
		screen.endCreativeMode();
	}
	else { // If box isn't checked, turn on
		creativeModeBox.setChecked(true);
		autoFallBox.setChecked(false);
		screen.setAutoFall(false);
		screen.startCreativeMode();
	}
}

// Get player two assets, which are a copy of single player assets but one width to the right
vector<sf::Text> getPlayer2Text(vector<sf::Text> vec) {
	for (sf::Text& text : vec)
		text.move(WIDTH, 0);
	return vec;
}

// Draw all objects in a vector
void drawVector(sf::RenderWindow& window, vector<sf::RectangleShape>& vec) {
	for (auto item : vec)
		window.draw(item);
}
void drawVector(sf::RenderWindow& window, vector<sf::Text>& vec) {
	for (auto item : vec)
		window.draw(item);
}
void drawVector(sf::RenderWindow& window, vector<FadeText>& vec) {
	for (FadeText& animation : vec)
		animation.draw(window);
}
void drawVector(sf::RenderWindow& window, vector<Animation*>& vec) {
	for (Animation* animation : vec)
		animation->draw(window);
}

int main(){
	srand(time(NULL));
	// Set SFML objects
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;
	sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Tetris", sf::Style::Close | sf::Style::Titlebar, settings);
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
	vector<sf::Text> menuText = getMenuText(font);
	int cursorPos = 0;
	sf::CircleShape cursor(15.f, 3); // Triangle shaped cursor
	cursor.setPosition(sf::Vector2f(MENUXPOS - 5, MENUYPOS));
	cursor.rotate(90.f);

	// Game sprites in a vector and constructed in a separate method to keep main clean
	vector<sf::RectangleShape> gameScreenRectangles = getGameRects(GAMEPOS); 
	vector<sf::FloatRect> gameScreenBounds = getGameBounds(gameScreenRectangles); 
	vector<sf::Text> gameText = getGameText(gameScreenBounds, font);
	vector<sf::RectangleShape> lines = getLines(gameScreenBounds[0]);
	sf::Text linesClearedText = generateText(font, WHITE, "Lines: 0", 25, sf::Vector2f(GAMEXPOS + GAMEWIDTH + 150, GAMEYPOS));

	// Sandbox mode exclusive sprites
	vector<sf::Text> sandboxText = getSandboxText(font);
	Checkbox autoFallBox(TILESIZE, SANDBOXMENUPOS.x + 180, SANDBOXMENUPOS.y, true, font);
	IncrementalBox gravityBox(TILESIZE, SANDBOXMENUPOS.x + 180, SANDBOXMENUPOS.y + MENUSPACING, 1, GRAVITYTIERCOUNT, font);
	Checkbox creativeModeBox(TILESIZE, SANDBOXMENUPOS.x + 180, SANDBOXMENUPOS.y + MENUSPACING * 2, false, font);
	Checkbox resetBox(TILESIZE, SANDBOXMENUPOS.x + 180, SANDBOXMENUPOS.y + MENUSPACING * 3, false, font);
	Checkbox quitBox(TILESIZE, SANDBOXMENUPOS.x + 180, SANDBOXMENUPOS.y + MENUSPACING * 4, false, font);
	vector<Checkbox*> sandboxes = { &autoFallBox, &gravityBox, &creativeModeBox, &resetBox, &quitBox };
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
	Screen screen(window, gameScreenBounds, texture, &clearAnimations, &bag);
	Screen screenP2(window, gameScreenBoundsP2, texture, &clearAnimationsP2, &bag);
	int currentScreen = MAINMENU;

	// Game loop
	while (window.isOpen())
	{
		// Run on main menu
		if (currentScreen == MAINMENU) {
			window.clear(BLUE);
			drawVector(window, menuText);
			window.draw(cursor);

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
						if (cursorPos < menuText.size() - 2)
							cursorPos++;
					}
					else if (event.key.code == sf::Keyboard::Up) {
						if (cursorPos > 0)
							cursorPos--;
					}
					else if (event.key.code == sf::Keyboard::Z) {
						switch (cursorPos)
						{
						case 0: // Classic mode
							currentScreen = CLASSIC;
							screen.setGameMode(CLASSIC);
							gameText[2].setString("Classic Mode");
							screen.setAutoFall(true);
							screen.endCreativeMode();
							bag.resetQueue();
							screen.resetBoard();
							break;
						case 1: // Sandbox mode
							currentScreen = SANDBOX;
							gameText[2].setString("Sandbox Mode");
							screen.setGameMode(SANDBOX);
							// Reset sandbox settings
							autoFallBox.setChecked(true);
							gravityBox.setValue(gravityBox.getMin());
							creativeModeBox.setChecked(false);
							screen.setAutoFall(true);
							screen.endCreativeMode();
							bag.resetQueue();
							screen.resetBoard();
							break;
						case 2: // PVP mode
							currentScreen = MULTIPLAYER;
							window.setSize(sf::Vector2u(WIDTH * 2, HEIGHT));
							window.setView(sf::View(sf::FloatRect(0, 0, WIDTH * 2, HEIGHT)));
							window.setPosition(sf::Vector2i(100, 100));
							screen.setGameMode(MULTIPLAYER);
							gameText[2].setString("");
							screen.setAutoFall(true);
							screen.endCreativeMode();
							bag.resetQueue();
							screen.resetBoard();

							screenP2.setGameMode(MULTIPLAYER);
							screenP2.resetBoard();
							
							break;
						case 3: // Settings
							window.close();
							return 0;
						case 4: // Quit
							window.close();
							return 0;
						default:
							break;
						}
					}
					break;
				}
				case sf::Event::MouseButtonPressed: {

				}
				default:
					break;
				}
				cursor.setPosition(sf::Vector2f(MENUXPOS - 5, MENUYPOS + cursorPos * MENUSPACING));
			}
		}
		// Run on classic mode
		else if (currentScreen == CLASSIC) {
			window.clear(BLUE);
			drawVector(window, gameScreenRectangles);
			drawVector(window, lines);
			screen.drawScreen();
			window.draw(gameScreenRectangles.back()); // Redraw last rectangle
			drawVector(window, gameText);
			linesClearedText.setString("Lines: " + to_string(screen.getLinesCleared()));
			window.draw(linesClearedText);
			drawVector(window, clearAnimations);

			// In-game timer events
			screen.doTimeStuff();

			// Check for game over
			if (screen.getGameOver() && screen.isDeathAnimationOver()) {
				currentScreen = CLASSICLOSS;
				lossText[0] = generateText(font, WHITE, "YOU LOST", GAMETEXTSIZE * 4, sf::Vector2f(WIDTH / 2, GAMEYPOS), true, false, true);
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
						screen.movePiece(3);
					else if (event.key.code == playerSoloKeys.getSpinCCW())
						screen.spinPiece(false);
					else if (event.key.code == playerSoloKeys.getSpinCW())
						screen.spinPiece(true);
					else if (event.key.code == playerSoloKeys.getHold())
						screen.holdPiece();
					else if (event.key.code == sf::Keyboard::Escape)
						screen.doPauseResume();
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
			screen.drawScreen();
			window.draw(gameScreenRectangles.back()); // Redraw last rectangle
			drawVector(window, gameText);
			drawVector(window, sandboxText);
			for (auto box : sandboxes)
				box->draw(window);
			
			// In-game timer events
			screen.doTimeStuff();
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
						screen.movePiece(3);
					else if (event.key.code == playerSoloKeys.getSpinCCW())
						screen.spinPiece(false);
					else if (event.key.code == playerSoloKeys.getSpinCW())
						screen.spinPiece(true);
					else if (event.key.code == playerSoloKeys.getHold())
						screen.holdPiece();
					else if (event.key.code == sf::Keyboard::Num1)
						screen.spawnPiece(0);
					else if (event.key.code == sf::Keyboard::Num2)
						screen.spawnPiece(1);
					else if (event.key.code == sf::Keyboard::Num3)
						screen.spawnPiece(2);
					else if (event.key.code == sf::Keyboard::Num4)
						screen.spawnPiece(3);
					else if (event.key.code == sf::Keyboard::Num5)
						screen.spawnPiece(4);
					else if (event.key.code == sf::Keyboard::Num6)
						screen.spawnPiece(5);
					else if (event.key.code == sf::Keyboard::Num7)
						screen.spawnPiece(6);
						// Hot keys for sandbox controls
					else if (event.key.code == sf::Keyboard::Q)
						toggleGravity(autoFallBox, screen);
					else if (event.key.code == sf::Keyboard::W)
						decrementGravity(gravityBox, autoFallBox, screen);
					else if (event.key.code == sf::Keyboard::S)
						incrementGravity(gravityBox, autoFallBox, screen);
					else if (event.key.code == sf::Keyboard::E)
						toggleCreative(creativeModeBox, autoFallBox, screen);
					else if (event.key.code == sf::Keyboard::R) {
						bag.resetQueue();
						screen.resetBoard();
					}
					else if (event.key.code == sf::Keyboard::T)
						currentScreen = MAINMENU;
					else if (event.key.code == sf::Keyboard::G) {
						screen.receiveGarbage(1);
					}
					else if (event.key.code == sf::Keyboard::H) {
						screen.receiveGarbage(4);
					}
					else if (event.key.code == sf::Keyboard::Escape)
						screen.doPauseResume();
					break;
				}
				case sf::Event::KeyReleased: {
					playerSoloDAS.releaseKey(event.key.code);
				}
				case sf::Event::MouseButtonPressed: {
					sf::Vector2f clickPos(event.mouseButton.x, event.mouseButton.y);
					if (event.mouseButton.button == sf::Mouse::Left) {
						if (autoFallBox.getBounds().contains(clickPos))  // Turn off gravity
							toggleGravity(autoFallBox, screen);
						else if (gravityBox.getLeftBound().contains(clickPos))  // Speed arrows
							decrementGravity(gravityBox, autoFallBox, screen);
						else if (gravityBox.getRightBound().contains(clickPos))
							incrementGravity(gravityBox, autoFallBox, screen);
						else if (creativeModeBox.getBounds().contains(clickPos))
							toggleCreative(creativeModeBox, autoFallBox, screen);
						else if (gameScreenBounds[0].contains(clickPos))  // Creative mode click
							screen.clickBlock(clickPos); // Will do nothing if creative mode isn't on
						else if (resetBox.getBounds().contains(clickPos)) {
							bag.resetQueue();
							screen.resetBoard();
						}
						else if (quitBox.getBounds().contains(clickPos)) // Return to menu
							currentScreen = MAINMENU;
					}
					else if (event.mouseButton.button == sf::Mouse::Right && gameScreenBounds[0].contains(clickPos)) {
						screen.clickRow(clickPos); // Creative mode right click
					}
					break;
				}
				case sf::Event::MouseMoved: { // Shows a transparent X when hovering over unchecked boxes
					sf::Vector2f mousePos(event.mouseMove.x, event.mouseMove.y);
					autoFallBox.setHovering(autoFallBox.getBounds().contains(mousePos));
					creativeModeBox.setHovering(creativeModeBox.getBounds().contains(mousePos));
					resetBox.setHovering(resetBox.getBounds().contains(mousePos));
					quitBox.setHovering(quitBox.getBounds().contains(mousePos));
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
			screen.drawScreen();
			window.draw(gameScreenRectangles.back()); // Redraw last rectangle
			drawVector(window, gameText);
			drawVector(window, clearAnimations);

			drawVector(window, gameScreenRectanglesP2);
			drawVector(window, linesP2);
			screenP2.drawScreen();
			window.draw(gameScreenRectanglesP2.back());
			drawVector(window, gameTextP2);
			drawVector(window, clearAnimationsP2);


			// In-game timer events
			screen.doTimeStuff();
			screenP2.doTimeStuff();
			// Process garbage exchange
			screen.receiveGarbage(screenP2.getOutGarbage());
			screenP2.receiveGarbage(screen.getOutGarbage());

			// Check for game over
			

			if (screen.getGameOver()) {
				screenP2.pauseGame();
				if (screen.isDeathAnimationOver()) {
					window.setSize(sf::Vector2u(WIDTH, HEIGHT));
					window.setView(sf::View(sf::FloatRect(0, 0, WIDTH, HEIGHT)));
					currentScreen = CLASSICLOSS;
					lossText[0] = generateText(font, WHITE, "PLAYER 2 WINS!", GAMETEXTSIZE * 4, sf::Vector2f(WIDTH / 2, GAMEYPOS), true, false, true);
				}
			}
			else if (screenP2.getGameOver()) {
				screen.pauseGame();
				if (screenP2.isDeathAnimationOver()) {
					window.setSize(sf::Vector2u(WIDTH, HEIGHT));
					window.setView(sf::View(sf::FloatRect(0, 0, WIDTH, HEIGHT)));
					currentScreen = CLASSICLOSS;
					lossText[0] = generateText(font, WHITE, "PLAYER 1 WINS!", GAMETEXTSIZE * 4, sf::Vector2f(WIDTH / 2, GAMEYPOS), true, false, true);
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
						screen.movePiece(3);
					else if (event.key.code == player1Keys.getSpinCCW())
						screen.spinPiece(false);
					else if (event.key.code == player1Keys.getSpinCW())
						screen.spinPiece(true);
					else if (event.key.code == player1Keys.getHold())
						screen.holdPiece();
					else if (event.key.code == player2Keys.getUp())
						screenP2.movePiece(3);
					else if (event.key.code == player2Keys.getSpinCCW())
						screenP2.spinPiece(false);
					else if (event.key.code == player2Keys.getSpinCW())
						screenP2.spinPiece(true);
					else if (event.key.code == player2Keys.getHold())
						screenP2.holdPiece();
					else if (event.key.code == sf::Keyboard::Escape) {
						screen.doPauseResume();
						screenP2.doPauseResume();
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
		else if (currentScreen == CLASSICLOSS) {
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
		window.display();
	}
	return 0;
}
