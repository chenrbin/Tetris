#include <iostream>
#include <SFML/Graphics.hpp>
#include "Screen.h"
#include "TetrisConstants.h"
using namespace std;
using namespace TetrisVariables;

// Generate centered text entity. Can specify font, color, message, size, position, and style
sf::Text setText(sf::Font& font, sf::Color color, string message, unsigned int textSize, sf::Vector2f coords, bool bold = false, bool underlined = false) {
	sf::Text text;
	text.setFont(font);
	text.setFillColor(color);
	text.setString(message);
	text.setCharacterSize(textSize);
	const sf::FloatRect box = text.getLocalBounds();
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
int main()
{
	// Set SFML objects
	sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Tetris", sf::Style::Close | sf::Style::Titlebar);
	
	sf::RectangleShape rect(sf::Vector2f(WIDTH, HEIGHT));
	rect.setFillColor(BLUE);

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
	sf::RectangleShape topRowRect = generateRectangle(sf::Vector2f(gameBounds.width, TILESIZE - 10), BLUE,
		sf::Vector2f(gameBounds.left, gameBounds.top - TILESIZE));

	sf::RectangleShape topRowRect2 = generateRectangle(sf::Vector2f(gameBounds.width - 2, TOPROWPIXELS), BLACK,
		sf::Vector2f(gameBounds.left + 1, gameBounds.top - TOPROWPIXELS), sf::Vector2f(0, 0), WHITE, 1);

	sf::Font font;
	font.loadFromFile("font.ttf");
	sf::Text holdText = setText(font, WHITE, "Hold", 20, sf::Vector2f(holdBounds.left + holdBounds.width / 2, holdBounds.top - 20), true, false);
	sf::Text nextText = setText(font, WHITE, "Next", 20, sf::Vector2f(queueBounds.left + queueBounds.width / 2, queueBounds.top - 20), true, false);

	vector<sf::RectangleShape> lines;
	for (int i = 1; i < NUMROWS; i++) {
		sf::RectangleShape line(sf::Vector2f(GAMEWIDTH, LINEWIDTH));
		line.setPosition(gameBounds.left, gameBounds.top + i * TILESIZE - 1);
		line.setFillColor(SEETHROUGH);
		lines.push_back(line);
	}
	for (int j = 1; j < NUMCOLS; j++) {
		sf::RectangleShape line(sf::Vector2f(LINEWIDTH, GAMEHEIGHT + TOPROWPIXELS));
		line.setPosition(gameBounds.left + j * TILESIZE - 1, gameBounds.top - TOPROWPIXELS);
		line.setFillColor(SEETHROUGH);
		lines.push_back(line);
	}



	sf::Texture texture;
	if (!texture.loadFromFile("images/tile_hidden.png"))
	{
		cout << "File not found\n";
		throw exception();
	}

	Screen screen(window, gameBounds, texture, holdBounds, queueBounds);
	window.setFramerateLimit(FPS);
	
	int timer = 0;
	// Game loop
	while (window.isOpen())
	{
		window.draw(rect);
		window.draw(gameRect);
		window.draw(holdRect);
		window.draw(queueRect);
		window.draw(holdText);
		window.draw(nextText);
		window.draw(topRowRect2);
		screen.doTimeStuff();

		for (int i = 0; i < lines.size(); i++) {
			window.draw(lines[i]);
		}
		screen.drawScreen();
		window.draw(topRowRect);


		// Event handler
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
		window.display();
	}
	return 0;
}