#include <iostream>
#include <SFML/Graphics.hpp>
#include "Screen.h"
#include "TetrisConstants.h"
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

void generateGameScreenRectanglesAndBounds(vector<sf::RectangleShape>& rects, vector<sf::FloatRect>& bounds) {
	sf::RectangleShape gameRect = generateRectangle(sf::Vector2f(GAMEWIDTH, GAMEHEIGHT), BLACK,
		sf::Vector2f(LEFTMARGIN, HEIGHT / 2), sf::Vector2f(0, GAMEHEIGHT / 2), WHITE, 1);
	sf::FloatRect gameBounds = gameRect.getGlobalBounds();

	sf::RectangleShape holdRect = generateRectangle(sf::Vector2f(GAMEWIDTH / 2.5, GAMEHEIGHT / 5), BLACK,
		sf::Vector2f(gameBounds.left - GAMEWIDTH / 2.5, gameBounds.top + 1), sf::Vector2f(0, 0), WHITE, 1);
	sf::FloatRect holdBounds = holdRect.getGlobalBounds();

	sf::RectangleShape queueRect = generateRectangle(sf::Vector2f(GAMEWIDTH / 2.5, GAMEHEIGHT / (1.8 / 5 * NEXTPIECECOUNT)),
		BLACK, sf::Vector2f(gameBounds.left + gameBounds.width, gameBounds.top + 1), sf::Vector2f(0, 0), WHITE, 1);
	sf::FloatRect queueBounds = queueRect.getGlobalBounds();

	// Mechanism to show a couple pixels of the very top row
	sf::RectangleShape topRowBigRect = generateRectangle(sf::Vector2f(gameBounds.width, TILESIZE - 10), BLUE,
		sf::Vector2f(gameBounds.left, gameBounds.top - TILESIZE));

	sf::RectangleShape topRowSmallRect = generateRectangle(sf::Vector2f(gameBounds.width - 2, TOPROWPIXELS), BLACK,
		sf::Vector2f(gameBounds.left + 1, gameBounds.top - TOPROWPIXELS), sf::Vector2f(0, 0), WHITE, 1);
	
	rects = { gameRect, holdRect, queueRect, topRowSmallRect, topRowBigRect};
	bounds = { gameBounds, holdBounds, queueBounds };
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

// Generate a vector to store menu text sprites
void generateMenuText(vector<sf::Text>& menuText, sf::Font& font) {
	vector<string> menuItems = { "Classic Mode", "Sandbox Mode", "PVP Mode", "Quit" };
	for (int i = 0; i < menuItems.size(); i++)
		menuText.push_back(setText(font, WHITE, menuItems[i], MENUTEXTSIZE, sf::Vector2f(MENUXPOS, MENUYPOS + MENUSPACING * i)));
}

int main()
{
	int currentScreen = MENUSCREEN;
	// Set SFML objects
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;
	sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Tetris", sf::Style::Close | sf::Style::Titlebar, settings);

	// Store these objects in a vector and construct them with a separate method to keep main clean
	vector<sf::RectangleShape> gameScreenRectangles; // { gameRect, holdRect, queueRect, topRowSmallRect, topRowBigRect}
	vector<sf::FloatRect> gameScreenRectangleBounds; // { gameBounds, holdBounds, queueBounds }
	generateGameScreenRectanglesAndBounds(gameScreenRectangles, gameScreenRectangleBounds);

	sf::Font font;
	font.loadFromFile("font.ttf");

	// Redefine bounds for cleaner code
	sf::FloatRect& gameBounds = gameScreenRectangleBounds[0], holdBounds = gameScreenRectangleBounds[1], queueBounds = gameScreenRectangleBounds[2];
	
	sf::Text holdText = setText(font, WHITE, "Hold", GAMETEXTSIZE, sf::Vector2f(holdBounds.left + holdBounds.width / 2, holdBounds.top - 20), true, false, true);
	sf::Text nextText = setText(font, WHITE, "Next", GAMETEXTSIZE, sf::Vector2f(queueBounds.left + queueBounds.width / 2, queueBounds.top - 20), true, false, true);

	vector<sf::RectangleShape> lines = getLines(gameBounds);

	sf::Text titleText = setText(font, WHITE, "TETRIS", 150, sf::Vector2f(WIDTH / 2, 100), true, false, true);
	vector<sf::Text> menuText;
	generateMenuText(menuText, font);

	int cursorPos = 0;
	sf::CircleShape cursor(15.f, 3);
	cursor.setPosition(sf::Vector2f(MENUXPOS - 5, MENUYPOS));
	cursor.rotate(90.f);

	sf::Texture texture;
	if (!texture.loadFromFile("images/tile_hidden.png"))
	{
		cout << "File not found\n";
		throw exception();
	}

	Screen screen(window, gameBounds, texture, holdBounds, queueBounds);
	window.setFramerateLimit(FPS);

	// Game loop
	while (window.isOpen())
	{
		if (currentScreen == MENUSCREEN) {
			window.clear(BLUE);
			for (sf::Text& text : menuText)
				window.draw(text);
			window.draw(cursor);
			window.draw(titleText);

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
						if (cursorPos < menuText.size() - 1)
							cursorPos++;
					}
					else if (event.key.code == sf::Keyboard::Up) {
						if (cursorPos > 0)
							cursorPos--;
					}
					else if (event.key.code == sf::Keyboard::Z) {
						switch (cursorPos)
						{
						case 0:
							currentScreen = GAMESCREEN;
							break;
						case 1:
							currentScreen = SANDBOXSCREEN;
							// break;
						case 2:
							currentScreen = MULTIPLAYERSCREEN;
							// break;
						default:
							window.close();
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
			for (int i = 0; i < gameScreenRectangles.size() - 1; i++) // Do not draw topRowBigRect
				window.draw(gameScreenRectangles[i]);
			window.draw(holdText);
			window.draw(nextText);
			for (int i = 0; i < lines.size(); i++) {
				window.draw(lines[i]);
			}
			screen.drawScreen();

			// In-game timer events
			screen.doTimeStuff();

			window.draw(gameScreenRectangles.back()); // Draw topRowBigRect


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
					// else if (event.key.code == sf::Keyboard::C)
						// screen.spawnPiece();
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
		window.display();
	}
	return 0;
}