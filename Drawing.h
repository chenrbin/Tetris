#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include "TetrisConstants.h"
#include "Tile.h"
#include "Mechanisms.h"
using namespace TetrisVariables;

// Class to organize drawable sprites of a checkbox. Purely visual functionality.
class Checkbox {
protected:
	sf::RectangleShape box;
	sf::FloatRect bounds;
	sf::Text check;
	sf::Text hoverCheck;
	bool checked, hovering;
public:
	Checkbox(float size, float left, float top, bool checked, sf::Font& font) {
		// White outline box
		box = sf::RectangleShape(sf::Vector2f(size, size));
		box.setPosition(left, top);
		box.setOutlineColor(WHITE);
		box.setFillColor(BLUE);
		box.setOutlineThickness(LINEWIDTH);
		bounds = box.getGlobalBounds();

		// "X" to toggle
		check.setFont(font);
		check.setString("X");
		check.setCharacterSize(size);
		check.setFillColor(WHITE);
		check.setStyle(sf::Text::Bold);
		sf::FloatRect textBounds = check.getGlobalBounds();
		check.setOrigin(textBounds.width / 2, textBounds.height);
		check.setPosition(bounds.left + bounds.width / 2, bounds.top + bounds.height / 2);
		this->checked = checked;

		hoverCheck = check;
		hoverCheck.setFillColor(HOVERCHECKBOX);
		hovering = false;
	}
	virtual void draw(sf::RenderWindow& window) {
		window.draw(box);
		if (checked)
			window.draw(check);
		else if (hovering)
			window.draw(hoverCheck);
	}
	void setChecked(bool val) {
		checked = val;
	}
	void setHovering(bool value) {
		hovering = value;
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
	IncrementalBox(float size, float left, float top, int min, int max, sf::Font& font) : Checkbox(size, left, top, true, font) {
		// Clickable left and right arrows
		leftArrow = sf::CircleShape(size / 2, 3);
		leftArrow.setOrigin(size / 2, 0);
		leftArrow.rotate(270);
		leftArrow.setPosition(bounds.left - 30, bounds.top + bounds.height / 2);

		rightArrow = sf::CircleShape(size / 2, 3);
		rightArrow.rotate(90);
		rightArrow.setPosition(bounds.left + bounds.width + 30, bounds.top);

		leftBound = leftArrow.getGlobalBounds();
		rightBound = rightArrow.getGlobalBounds();

		// Text is numbers min-max
		this->min = min;
		this->max = max;
		currentNum = min;
		updateString();
	}
	void draw(sf::RenderWindow& window) {
		window.draw(box);
		window.draw(check);
		window.draw(leftArrow);
		window.draw(rightArrow);
	}
	void updateString() {
		check.setString(to_string(currentNum));
	}
	sf::FloatRect& getLeftBound() {
		return leftBound;
	}
	sf::FloatRect& getRightBound() {
		return rightBound;
	}
	int getCurrentNum() {
		return currentNum;
	}
	int getMin() {
		return min;
	}
	void setValue(int num) {
		if (num >= min && num <= max)
			currentNum = num;
		updateString();
	}
	void increment() {
		if (currentNum < max)
			currentNum++;
		else
			currentNum = min;
		updateString();
	}
	void decrement() {
		if (currentNum > min)
			currentNum--;
		else
			currentNum = max;
		updateString();
	}
};

// Animations are meant to be each created once and restarted when played
class Animation { 
protected:
	sfClockAtHome startTime;
	float duration;
public:
	// Draws the animation to the window
	virtual void draw(sf::RenderWindow& window) = 0;
	virtual ~Animation() {};
	// Turns on animation
	virtual void restart() = 0;
	// Return true if animation has expired
	virtual bool isOver() {
		return startTime.getTimeSeconds() > duration;
	}
};

// Text that fades after a set duration
class FadeText : public Animation {
	float fadeDuration; // In seconds
	sf::Text text; // Text to display. Properties are created using setText when constructed.
	sf::Color textColor;
public:
	FadeText(sf::Text text, float duration, float fadeDuration) {
		this->text = text;
		this->duration = duration;
		this->fadeDuration = fadeDuration;
		startTime.restart();
		textColor = text.getFillColor();
		textColor.a = 0; // This disables the animation when constructed
		this->text.setFillColor(textColor);
	}
	// Draws the animation to the window
	void draw(sf::RenderWindow& window) {
		float elapsedTime = startTime.getElapsedTime().asSeconds();
		if (elapsedTime > duration + fadeDuration)
			return; // Returns nothing if animation is past duration
		if (elapsedTime > duration && textColor.a > 0) { // Begin fading
			textColor.a = ((1 - (elapsedTime - duration) / fadeDuration) * 255);
			text.setFillColor(textColor);
		}
		window.draw(text);
	}
	// Turns on animation
	void restart() { 
		startTime.restart();
		textColor.a = 255;
		text.setFillColor(textColor);
	}
	// Change the text to display
	void setString(string str) {
		text.setString(str);
	}
};

// Class to display a player's death screen
class DeathAnimation : public Animation {
	float endDuration; // Delay at the end of animation
	vector<vector<Tile>> board;
public:
	DeathAnimation(sf::Vector2f gamePos, float duration, float endDuration, sf::Texture& blockTexture) {
		this->duration = duration;
		this->endDuration = endDuration;
		startTime.restart();
		// Generate board
		for (int i = 0; i < REALNUMROWS; i++) {
			vector<Tile> row;
			for (int j = 0; j < NUMCOLS; j++) {
				Tile tile(blockTexture, gamePos.x + j * TILESIZE, gamePos.y + (i - 2) * TILESIZE);
				tile.setBlock(true, GRAY);
				row.push_back(tile);
			}
			board.push_back(row);
		}
	}
	// Draws the animation to the window
	void draw(sf::RenderWindow& window) {
		// Does not execute if animation duration is over
		if (startTime.getTimeSeconds() > duration + endDuration)
			return;
		for (int i = 0; i < NUMROWS; i++)
			if (startTime.getTimeSeconds() / duration * NUMROWS >= i - 1)
				for (int j = 0; j < NUMCOLS; j++)
					board[REALNUMROWS - i - 1][j].draw(&window);
	}
	// Turns on animation
	void restart(){
		startTime.restart();
	}
	// Return true if animation has expired
	virtual bool isOver() {
		return startTime.getTimeSeconds() > duration + endDuration;
	}
};

// Class to display garbage bin
class garbageStack {
	vector<sf::RectangleShape> stack;
public:
	// Construct a stack of rectangles at position relative to gamePos
	garbageStack(sf::Vector2f gamePos) {
		for (int i = 0; i < NUMROWS; i++) {
			sf::RectangleShape rec(sf::Vector2f(TILESIZE / 2, TILESIZE - 1));
			rec.setPosition(gamePos.x - TILESIZE / 2, gamePos.y + GAMEHEIGHT - (i + 1) * TILESIZE + LINEWIDTH + 1);
			rec.setOutlineColor(WHITE);
			rec.setOutlineThickness(1);
			stack.push_back(rec);
		}
	}
	void draw(sf::RenderWindow& window) {
		for (sf::RectangleShape& rec : stack) {
			window.draw(rec);
		}
	}
	// Update stack visuals based on a vector of garbage timers
	void updateStack(vector<float> vec) {
		for (int i = 0; i < 20; i++)
		{
			// (255 * vec[i] + 255) / 2 sets the red value to 127-255 based on timer progress
			if (vec.size() > i) {
				stack[i].setFillColor(sf::Color((255 * vec[i] + 255) / 2, 0, 0));
				if (stack[i].getFillColor() == RED)
					stack[i].setOutlineColor(RED);
				else 
					stack[i].setOutlineColor(WHITE);
			}
			else {
				stack[i].setFillColor(BLACK);
				stack[i].setOutlineColor(WHITE);
			}
		}
	}
};

// Class for easy sf::Text generation. Replaces the setText function
class sfTextAtHome : public sf::Text{
public:
	sfTextAtHome(sf::Font& font, sf::Color color, string message, unsigned int textSize, sf::Vector2f coords, bool bold = true, bool underlined = false, bool centered = false) {
		setFont(font);
		setFillColor(color);
		setString(message);
		setCharacterSize(textSize);
		if (centered)
			alignCenter();
		setPosition(coords);
		if (bold)
			setStyle(sf::Text::Bold);
		if (underlined)
			setStyle(sf::Text::Underlined);
	}
	void alignCenter() {
		const sf::FloatRect box = getLocalBounds();
		setOrigin(box.width / 2.0f, box.height / 2.0f);
	}
	bool contains(float x, float y) {
		return getGlobalBounds().contains(x, y);
	}
};

// Class for a text that can be navigated and clicked
class clickableMenu {
	vector<sfTextAtHome> texts;
	sf::CircleShape cursor; // Takes in a shape as the cursor. Must preset attributes.
	int cursorPos;
public:
	clickableMenu(sf::Font& font, sf::Color color, vector<string>& menuText, int textSize, sf::Vector2f startPos, int spacing, sf::CircleShape& cursor) {
		for (int i = 0; i < menuText.size(); i++)
			texts.push_back(sfTextAtHome(font, color, menuText[i], textSize, sf::Vector2f(startPos.x, startPos.y + spacing * i)));
		this->cursor = cursor;
		cursor.setPosition(startPos.x - 5, startPos.y);
		cursorPos = 0;
		updateCursor();
	}
	void moveUp() {
		cursorPos--;
		if (cursorPos < 0)
			cursorPos = texts.size() - 1;
		updateCursor();
	}
	void moveDown() {
		cursorPos++;
		if (cursorPos >= texts.size())
			cursorPos = 0;
		updateCursor();
	}
	// Update cursor position to the left of the text it is pointing to
	void updateCursor() {
		cursor.setPosition(texts[cursorPos].getPosition().x - 5, texts[cursorPos].getPosition().y);
	}
	// Draw all menu components
	void draw(sf::RenderWindow& window) {
		for (sfTextAtHome& text : texts)
			window.draw(text);
		window.draw(cursor);
	}
	// Update cursor based on mouse position. Return true if a text is select
	bool updateMouse(float x, float y) {
		for (int i = 0; i < texts.size(); i++) {
			if (texts[i].contains(x, y)) {
				cursorPos = i;
				updateCursor();
				return true;
			}
		}
		return false;
	}
	int getCursorPos() {
		return cursorPos;
	}
};

