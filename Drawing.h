#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include "TetrisConstants.h"
using namespace TetrisVariables;

// Class to organize drawable sprites of a checkbox. Purely visual functionality.
class Checkbox {
protected:
	sf::RectangleShape box;
	sf::FloatRect bounds;
	sf::Text check;
	bool checked;
public:
	Checkbox(float size, float left, float top, bool checked, sf::Font& font) {
		box = sf::RectangleShape(sf::Vector2f(size, size));
		box.setPosition(left, top);
		box.setOutlineColor(WHITE);
		box.setFillColor(BLUE);
		box.setOutlineThickness(LINEWIDTH);
		bounds = box.getGlobalBounds();
		check.setFont(font);
		check.setString("X");
		check.setCharacterSize(size);
		check.setFillColor(WHITE);
		check.setStyle(sf::Text::Bold);
		sf::FloatRect textBounds = check.getGlobalBounds();
		check.setOrigin(textBounds.width / 2, textBounds.height);
		check.setPosition(bounds.left + bounds.width / 2, bounds.top + bounds.height / 2);
		this->checked = checked;
	}
	void draw(sf::RenderWindow& window) {
		window.draw(box);
		if (checked)
			window.draw(check);
	}
	void setChecked(bool val) {
		checked = val;
	}
	bool getChecked() {
		return checked;
	}
	sf::FloatRect& getBounds() {
		return bounds;
	}
};

// Inherited from checkbox. Has two clickable arrows. Text is a range of numbers
class IncrementalBox : public Checkbox {
	sf::CircleShape leftArrow;
	sf::FloatRect leftBound;
	sf::CircleShape rightArrow;
	sf::FloatRect rightBound;
	int min, max, currentNum;
public:
	IncrementalBox(float size, float left, float top, sf::Font& font) : Checkbox(size, left, top, true, font) {
		leftArrow = sf::CircleShape(size / 2, 3);
		leftArrow.setOrigin(size / 2, 0);
		leftArrow.rotate(270);
		leftBound = leftArrow.getGlobalBounds();

		rightArrow = sf::CircleShape(size / 2, 3);
		rightArrow.rotate(90);
		rightBound = rightArrow.getGlobalBounds();

		leftArrow.setPosition(bounds.left - 30, bounds.top + bounds.height / 2);
		rightArrow.setPosition(bounds.left + bounds.width + 30, bounds.top);
	}
	void draw(sf::RenderWindow& window) {
		window.draw(box);
		window.draw(check);
		window.draw(leftArrow);
		window.draw(rightArrow);
	}
};
