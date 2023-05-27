#include <iostream>
#include <SFML/Graphics.hpp>
#include "Screen.h"
#include "TetrisConstants.h"
#include <map>
#include "Drawing.h"
using namespace std;
using namespace TetrisVariables;

// Generate centered text entity. Can specify font, color, message, size, position, and style
sf::Text setText(sf::Font& font, sf::Color color, string message, unsigned int textSize, sf::Vector2f coords, bool bold = true, bool underlined = false, bool centered = false) {
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
vector<sf::RectangleShape> getGameRects() {
	// Rectangles for game screen, hold, queue, and top row
	vector<sf::RectangleShape> rects;
	rects.push_back(generateRectangle(sf::Vector2f(GAMEWIDTH, GAMEHEIGHT), BLACK,
		GAMEPOS, sf::Vector2f(0, 0), WHITE, LINEWIDTH));
	rects.push_back(generateRectangle(sf::Vector2f(GAMEWIDTH / 2.5, GAMEHEIGHT / 5), BLACK,
		sf::Vector2f(GAMEXPOS - GAMEWIDTH / 2.5 - LINEWIDTH, GAMEYPOS + LINEWIDTH), sf::Vector2f(0, 0), WHITE, LINEWIDTH));
	rects.push_back(generateRectangle(sf::Vector2f(GAMEWIDTH / 2.5, GAMEHEIGHT / 9 * NEXTPIECECOUNT),
		BLACK, sf::Vector2f(GAMEXPOS + GAMEWIDTH + LINEWIDTH, GAMEYPOS + LINEWIDTH), sf::Vector2f(0, 0), WHITE, LINEWIDTH));

	// Rectangles to show a couple pixels of the very top row
	rects.push_back(generateRectangle(sf::Vector2f(GAMEWIDTH, TOPROWPIXELS), BLACK,
		sf::Vector2f(GAMEXPOS, GAMEYPOS - TOPROWPIXELS), sf::Vector2f(0, 0), WHITE, LINEWIDTH));
	rects.push_back(generateRectangle(sf::Vector2f(GAMEWIDTH + 1, TILESIZE - 9), BLUE,
		sf::Vector2f(GAMEXPOS - 1, GAMEYPOS - TILESIZE - 1)));
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
	textboxes.push_back(setText(font, WHITE, "TETRIS", 150, TITLETEXTPOS, true, false, true));
	vector<string> menuItems = { "Classic Mode", "Sandbox Mode", "PVP Mode", "Settings", "Quit" };
	for (int i = 0; i < menuItems.size(); i++)
		textboxes.push_back(setText(font, WHITE, menuItems[i], MENUTEXTSIZE, sf::Vector2f(MENUXPOS, MENUYPOS + MENUSPACING * i)));
	return textboxes;
}
// Generate all text on game screen
vector<sf::Text> getGameText(vector<sf::FloatRect>& bounds, sf::Font& font) {
	// Redefine bounds for cleaner code
	sf::FloatRect& gameBounds = bounds[0], holdBounds = bounds[1], queueBounds = bounds[2];
	vector<sf::Text> textboxes;
	textboxes.push_back(setText(font, WHITE, "Hold", GAMETEXTSIZE, sf::Vector2f(holdBounds.left + holdBounds.width / 2, holdBounds.top - GAMETEXTSIZE), true, false, true));
	textboxes.push_back(setText(font, WHITE, "Next", GAMETEXTSIZE, sf::Vector2f(queueBounds.left + queueBounds.width / 2, queueBounds.top - GAMETEXTSIZE), true, false, true));
	textboxes.push_back(setText(font, WHITE, "Classic Mode", GAMETEXTSIZE * 2, sf::Vector2f(gameBounds.left + gameBounds.width / 2, gameBounds.top - GAMETEXTSIZE * 2), true, false, true));
	return textboxes;
}

// Generate text exclusive to sandbox mode
vector<sf::Text> getSandboxText(sf::Font& font) {
	vector<sf::Text> textboxes;
	vector<string> menuItems = { "Auto-fall", "Fall speed", "Creative", "Reset", "Quit" };
	for (int i = 0; i < menuItems.size(); i++)
		textboxes.push_back(setText(font, WHITE, menuItems[i], MENUTEXTSIZE, sf::Vector2f(SANDBOXMENUPOS.x, SANDBOXMENUPOS.y + MENUSPACING * i)));
	return textboxes;
}
// Get collision bounds for checkboxes
vector<sf::FloatRect> getBoxBounds(vector<sf::RectangleShape>& rects) {
	vector<sf::FloatRect> bounds;
	for (auto& rect : rects)
		bounds.push_back(rect.getGlobalBounds());
	return bounds;
}

// Following functions are made for reuseability and requires specific parameters passed in.
// Functionality of the Auto-fall box in sandbox mode. 
void toggleGravity(Checkbox& autoFallBox, Screen& screen, float& currentGravity) {
	if (autoFallBox.getChecked()) {
		autoFallBox.setChecked(false);
		screen.setGravity(0);
	}
	else {
		autoFallBox.setChecked(true);
		screen.setGravity(currentGravity);
	}
}
void decrementGravity(IncrementalBox& gravityBox, Checkbox& autoFallBox, Screen& screen, float& currentGravity, vector<float>& gravitySpeeds) {
	gravityBox.decrement();
	currentGravity = gravitySpeeds[gravityBox.getCurrentNum() - 1];
	if (autoFallBox.getChecked())
		screen.setGravity(currentGravity);
}
void incrementGravity(IncrementalBox& gravityBox, Checkbox& autoFallBox, Screen& screen, float& currentGravity, vector<float>& gravitySpeeds) {
	gravityBox.increment();
	currentGravity = gravitySpeeds[gravityBox.getCurrentNum() - 1];
	if (autoFallBox.getChecked())
		screen.setGravity(currentGravity);
}
void toggleCreative(Checkbox& creativeModeBox, Checkbox& autoFallBox, Screen& screen, float& currentGravity) {
	if (creativeModeBox.getChecked()) { // If box is already checked, turn off
		creativeModeBox.setChecked(false);
		autoFallBox.setChecked(true);
		screen.setGravity(currentGravity);
		screen.endCreativeMode();
	}
	else { // If box isn't checked, turn on
		creativeModeBox.setChecked(true);
		autoFallBox.setChecked(false);
		screen.startCreativeMode();
	}
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

int main(){
	
	// Set SFML objects
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;
	sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Tetris", sf::Style::Close | sf::Style::Titlebar, settings);
	sf::Font font;
	if (!font.loadFromFile("font.ttf")) {
		throw exception();
	}

	// Store these objects in a vector and construct them with a separate method to keep main clean
	// Title screen sprites
	vector<sf::Text> menuText = getMenuText(font);
	int cursorPos = 0;
	sf::CircleShape cursor(15.f, 3); // Triangle shaped cursor
	cursor.setPosition(sf::Vector2f(MENUXPOS - 5, MENUYPOS));
	cursor.rotate(90.f);

	// Game sprites
	vector<sf::RectangleShape> gameScreenRectangles = getGameRects(); 
	vector<sf::FloatRect> gameScreenBounds = getGameBounds(gameScreenRectangles); 
	vector<sf::Text> gameText = getGameText(gameScreenBounds, font);
	vector<sf::RectangleShape> lines = getLines(gameScreenBounds[0]);
	sf::Text linesClearedText = setText(font, WHITE, "Lines: 0", 25, sf::Vector2f(GAMEXPOS + GAMEWIDTH + 150, GAMEYPOS));
	
	vector<float> gravitySpeeds{ 1, 0.75, 0.5, 0.25, 0.1, 0.05, 0.01 };
	int currentScreen = MENUSCREEN, gravityCount = gravitySpeeds.size();
	float currentGravity = 1;
	int linesCleared = 0; // Store a separate variable of lines cleared in main to update. Optimize later
	bool creativeModeOn = false;

	// Sandbox mode exclusive sprites
	vector<sf::Text> sandboxText = getSandboxText(font);
	Checkbox autoFallBox(TILESIZE, SANDBOXMENUPOS.x + 180, SANDBOXMENUPOS.y, true, font);
	IncrementalBox gravityBox(TILESIZE, SANDBOXMENUPOS.x + 180, SANDBOXMENUPOS.y + MENUSPACING, 1, gravityCount, font);
	Checkbox creativeModeBox(TILESIZE, SANDBOXMENUPOS.x + 180, SANDBOXMENUPOS.y + MENUSPACING * 2, false, font);
	Checkbox resetBox(TILESIZE, SANDBOXMENUPOS.x + 180, SANDBOXMENUPOS.y + MENUSPACING * 3, false, font);
	Checkbox quitBox(TILESIZE, SANDBOXMENUPOS.x + 180, SANDBOXMENUPOS.y + MENUSPACING * 4, false, font);
	vector<Checkbox*> sandboxes = { &autoFallBox, &gravityBox, &creativeModeBox, &resetBox, &quitBox };

	sf::Texture texture;
	if (!texture.loadFromFile("images/tile_hidden.png"))
	{
		cout << "File not found\n";
		throw exception();
	}

	Screen screen(window, gameScreenBounds[0], texture, gameScreenBounds[1], gameScreenBounds[2]);
	window.setFramerateLimit(FPS);

	// Game loop
	while (window.isOpen())
	{
		if (currentScreen == MENUSCREEN) {
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
							currentScreen = GAMESCREEN;
							break;
						case 1: // Sandbox mode
							currentScreen = SANDBOXSCREEN;
							gameText[2].setString("Sandbox Mode");
							break;
						case 2: // PVP mode
							currentScreen = MULTIPLAYERSCREEN;
							// break;
						case 3: // Settings
							window.close();
							return 0;
						case 4: // Quit
							window.close();
							return 0;
						default:
							break;
						}
						screen.resetBoard();
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
		else if (currentScreen == GAMESCREEN) {
			window.clear(BLUE);
			drawVector(window, gameScreenRectangles);
			drawVector(window, lines);
			screen.drawScreen();
			window.draw(gameScreenRectangles.back()); // Redraw last rectangle
			drawVector(window, gameText);
			window.draw(linesClearedText);
			if (linesCleared != screen.getLinesCleared()) 
				// Only update string if number is different. 
				// Don't know if this is faster than updating every frame.
				linesClearedText.setString("Lines: " + to_string(screen.getLinesCleared()));
			

			// In-game timer events
			screen.doTimeStuff();

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
					if (event.key.code == sf::Keyboard::Left)
						screen.movePiece(0);
					else if (event.key.code == sf::Keyboard::Down)
						screen.movePiece(1);
					else if (event.key.code == sf::Keyboard::Right)
						screen.movePiece(2);
					else if (event.key.code == sf::Keyboard::Up)
						screen.movePiece(3);
					else if (event.key.code == sf::Keyboard::Z)
						screen.spinPiece(false);
					else if (event.key.code == sf::Keyboard::X)
						screen.spinPiece(true);
					else if (event.key.code == sf::Keyboard::LShift)
						screen.holdPiece();
					break;
				}
				case sf::Event::MouseButtonPressed: {
					break;
				}
				default:
					break;
				}
			}
		}
		else if (currentScreen == SANDBOXSCREEN) {
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
					if (event.key.code == sf::Keyboard::Left)
						screen.movePiece(0);
					else if (event.key.code == sf::Keyboard::Down)
						screen.movePiece(1);
					else if (event.key.code == sf::Keyboard::Right)
						screen.movePiece(2);
					else if (event.key.code == sf::Keyboard::Up)
						screen.movePiece(3);
					else if (event.key.code == sf::Keyboard::Z)
						screen.spinPiece(false);
					else if (event.key.code == sf::Keyboard::X)
						screen.spinPiece(true);
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
					else if (event.key.code == sf::Keyboard::LShift)
						screen.holdPiece();
					// Hot keys for sandbox controls
					else if (event.key.code == sf::Keyboard::Q)
						toggleGravity(autoFallBox, screen, currentGravity);
					else if (event.key.code == sf::Keyboard::W)
						decrementGravity(gravityBox, autoFallBox, screen, currentGravity, gravitySpeeds);
					else if (event.key.code == sf::Keyboard::S)
						incrementGravity(gravityBox, autoFallBox, screen, currentGravity, gravitySpeeds);
					else if (event.key.code == sf::Keyboard::E)
						toggleCreative(creativeModeBox, autoFallBox, screen, currentGravity);
					else if (event.key.code == sf::Keyboard::R)
						screen.resetBoard();
					else if (event.key.code == sf::Keyboard::T)
						currentScreen = MENUSCREEN;
					break;
				}
				case sf::Event::MouseButtonPressed: {
					sf::Vector2f clickPos(event.mouseButton.x, event.mouseButton.y);
					if (event.mouseButton.button == sf::Mouse::Left) {
						if (autoFallBox.getBounds().contains(clickPos)) { // Turn off gravity
							toggleGravity(autoFallBox, screen, currentGravity);
						}
						else if (gravityBox.getLeftBound().contains(clickPos)) { // Speed arrows
							decrementGravity(gravityBox, autoFallBox, screen, currentGravity, gravitySpeeds);
						}
						else if (gravityBox.getRightBound().contains(clickPos)) {
							incrementGravity(gravityBox, autoFallBox, screen, currentGravity, gravitySpeeds);
						}
						else if (creativeModeBox.getBounds().contains(clickPos)) {
							toggleCreative(creativeModeBox, autoFallBox, screen, currentGravity);
						}
						else if (gameScreenBounds[0].contains(clickPos))  // Creative mode click
							screen.clickBlock(clickPos); // Will do nothing if creative mode isn't on
						else if (resetBox.getBounds().contains(clickPos)) 
							screen.resetBoard();
						else if (quitBox.getBounds().contains(clickPos)) 
							currentScreen = MENUSCREEN;
					}
					else if (event.mouseButton.button == sf::Mouse::Right) {
						if (gameScreenBounds[0].contains(clickPos)) { // Creative mode right click
							screen.clickRow(clickPos);
						}
					}
					break;
				}
				default:
					break;
				}
			}
		}
		window.display();
	}
	return 0;
}