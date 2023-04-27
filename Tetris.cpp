#include <iostream>
#include <SFML/Graphics.hpp>
#include "Screen.h"
#include "TetrisConstants.h"
using namespace std;
using namespace TetrisVariables;

// Generate centered text entity. Can specify font, color, message, size, position, and style
template<typename T>
sf::Text setText(sf::Font& font, T& color, string message, unsigned int textSize, sf::Vector2f coords, bool bold = false, bool underlined = false) {
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


int main()
{
	// Set SFML objects
	sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Tetris", sf::Style::Close | sf::Style::Titlebar);
	
	sf::RectangleShape rect(sf::Vector2f(WIDTH, HEIGHT));
	rect.setFillColor(BLUE);

	sf::RectangleShape gameRect(sf::Vector2f(GAMEWIDTH, GAMEHEIGHT));
	gameRect.setFillColor(BLACK);
	gameRect.setOrigin(0, GAMEHEIGHT / 2);
	gameRect.setPosition(LEFTMARGIN, HEIGHT / 2);
	sf::FloatRect gameBounds = gameRect.getGlobalBounds();

	sf::RectangleShape holdRect(sf::Vector2f(GAMEWIDTH / 2.5, GAMEHEIGHT / 5));
	holdRect.setFillColor(BLACK);
	holdRect.setOutlineColor(WHITE);
	holdRect.setOutlineThickness(1);
	holdRect.setOrigin(holdRect.getGlobalBounds().width, 0);
	holdRect.setPosition(gameBounds.left, gameBounds.top);
	sf::FloatRect holdBounds = holdRect.getGlobalBounds();

	sf::RectangleShape queueRect(sf::Vector2f(GAMEWIDTH / 2.5, GAMEHEIGHT / 1.8));
	queueRect.setFillColor(BLACK);
	queueRect.setOutlineColor(WHITE);
	queueRect.setOutlineThickness(1);
	queueRect.setPosition(gameBounds.left + gameBounds.width, gameBounds.top);
	sf::FloatRect queueBounds = queueRect.getGlobalBounds();

	vector<sf::RectangleShape> lines;
	for (int i = 1; i < NUMROWS; i++) {
		sf::RectangleShape line(sf::Vector2f(GAMEWIDTH, LINEWIDTH));
		line.setPosition(gameBounds.left, gameBounds.top + i * TILESIZE - 1);
		line.setFillColor(SEETHROUGH);
		lines.push_back(line);
	}
	for (int j = 1; j < NUMCOLS; j++) {
		sf::RectangleShape line(sf::Vector2f(LINEWIDTH, GAMEHEIGHT));
		line.setPosition(gameBounds.left + j * TILESIZE - 1, gameBounds.top);
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
	sf::Font font;
	if (!font.loadFromFile("font.ttf"))
	{
		cout << "Font file not found.";
	}
	
	int timer = 0;
	// Game loop
	while (window.isOpen())
	{
		window.draw(rect);
		window.draw(gameRect);
		window.draw(holdRect);
		window.draw(queueRect);
		screen.doTimeStuff();

		for (int i = 0; i < lines.size(); i++) {
			window.draw(lines[i]);
		}
		screen.drawScreen();

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