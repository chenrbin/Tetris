#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include "TetrisConstants.h"
#include "Screen.h"
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
	sf::Clock startTime;
	float duration;
public:
	// Draws the animation to the window
	virtual void draw(sf::RenderWindow& window) = 0;
	virtual ~Animation() {};
	// Turns on animation
	virtual void restart() = 0;
	virtual void setString(string str) = 0;
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

// Class to handle auto repeat / DAS
class KeyTimer {
	sf::Clock startTimer, holdTimer;
	int startDelay, holdDelay; // In milliseconds
	bool startOn, holdOn;
public:
	KeyTimer(int startDelay = 0, int holdDelay = 0) {
		this->startDelay = startDelay;
		this->holdDelay = holdDelay;
		startOn = false;
		holdOn = false;
	}
	// Handles action while key is pressed. Returns true if an action should be performed
	bool press() {
		if (holdOn) { // If startOn and holdOn are true, check for auto repeat
			if (holdTimer.getElapsedTime().asMilliseconds() >= holdDelay) {
				holdTimer.restart();
				return true;
			}
			return false;
		}
		// If holdOn isn't true, check startOn for startTimer
		if (!startOn) {
			startOn = true;
			startTimer.restart();
			return true;
		} 
		else {
			if (startTimer.getElapsedTime().asMilliseconds() >= startDelay) {
				holdOn = true;
				holdTimer.restart();
			}
			return false;
		}
	}
	// Key is released. Reset boolean values
	void release() {
		holdOn = false;
		startOn = false;
	}
};

// Controls for each player
class KeySet {
	sf::Keyboard::Key left;
	sf::Keyboard::Key right;
	sf::Keyboard::Key up;
	sf::Keyboard::Key down;
	sf::Keyboard::Key spinCW;
	sf::Keyboard::Key spinCCW;
	sf::Keyboard::Key hold;
public:
	KeySet(sf::Keyboard::Key left, sf::Keyboard::Key right, sf::Keyboard::Key up,
		sf::Keyboard::Key down, sf::Keyboard::Key spinCW, sf::Keyboard::Key spinCCW, sf::Keyboard::Key hold) {
		this->left = left;
		this->right = right;
		this->up = up;
		this->down = down;
		this->spinCW = spinCW;
		this->spinCCW = spinCCW;
		this->hold = hold;
	}
	void setLeft(sf::Keyboard::Key key) {
		left = key;
	}
	void setRight(sf::Keyboard::Key key) {
		right = key;
	}
	void setUp(sf::Keyboard::Key key) {
		up = key;
	}
	void setDown(sf::Keyboard::Key key) {
		down = key;
	}
	void setSpinCW(sf::Keyboard::Key key) {
		spinCW = key;
	}
	void setSpinCCW(sf::Keyboard::Key key) {
		spinCCW = key;
	}
	void setHold(sf::Keyboard::Key key) {
		hold = key;
	}
	sf::Keyboard::Key getLeft() {
		return left;
	}
	sf::Keyboard::Key getRight() {
		return right;
	}
	sf::Keyboard::Key getUp() {
		return up;
	}
	sf::Keyboard::Key getDown() {
		return down;
	}
	sf::Keyboard::Key getSpinCW() {
		return spinCW;
	}
	sf::Keyboard::Key getSpinCCW() {
		return spinCCW;
	}
	sf::Keyboard::Key getHold() {
		return hold;
	}
};

// Organize DAS for each player
class KeyDAS {
	KeyTimer leftKey;
	KeyTimer rightKey;
	KeyTimer downKey;
	KeySet* keySet;
public:
	KeyDAS(int startDelay = 0, int holdDelay = 0, KeySet* keySet = nullptr) {
		leftKey = KeyTimer(startDelay, holdDelay);
		rightKey = KeyTimer(startDelay, holdDelay);
		downKey = KeyTimer(startDelay, holdDelay);
		this->keySet = keySet;
	}
	template<typename T>
	// Handles movement with auto-repeat (DAS). 
	void checkKeyPress(T& screen) { // Doesn't work if parameter type is Screen& for some reason. Needs template
		if (sf::Keyboard::isKeyPressed(keySet->getLeft())) {
			if (leftKey.press())
				screen.movePiece(0);
		}
		else if (sf::Keyboard::isKeyPressed(keySet->getRight()) && (rightKey.press()))
			screen.movePiece(2);
		// The code above prioritizes the left key if both left and right are held.
		if (sf::Keyboard::isKeyPressed(keySet->getDown()) && (downKey.press()))
			screen.movePiece(1);
	}
	void releaseKey(sf::Keyboard::Key& event) {
		if (event == keySet->getLeft())
			leftKey.release();
		else if (event == keySet->getDown())
			downKey.release();
		else if (event == keySet->getRight())
			rightKey.release();
	}
};

// Class to store random piece order to it is consistent across all players
class pieceBag {
	vector<char> pieceQueue; // Complete piece order
	vector<unsigned int> positions; // Position in the queue for each player
public:
	pieceBag() {
		addBatch();
	}
	// Adds a bag of 7 numbers to the queue when needed.
	void addBatch() {
		vector<char> queueBatch{ 0, 1, 2, 3, 4, 5, 6 };
		random_shuffle(queueBatch.begin(), queueBatch.end());
		for (char num : queueBatch) {
			pieceQueue.push_back(num);
		}
	}
	// Adds a player who is accessing the queue. Returns the player index
	int addPlayer() {
		positions.push_back(0);
		return positions.size() - 1;
	}
	// Resets the queue
	// NOTE: resetQueue must be called only once and before resetPosition 
	// during each reset for a proper starting bag for all players
	void resetQueue() {
		pieceQueue.clear();
		addBatch();
	}
	void resetPosition(int playerIndex) {
		positions[playerIndex] = 0;
	}
	// Get piece and increment position
	int getPiece(int playerIndex) {
		positions[playerIndex]++;
		if (pieceQueue.size() < positions[playerIndex] + 7)
			addBatch(); // Replenish queue
		return pieceQueue[positions[playerIndex] - 1];
	}
	// Returns the queue of next pieces to display
	vector<int> getNextPieces(int playerIndex, int pieceCount) {
		vector<int> queue;
		for (int i = 0; i < pieceCount; i++)
			queue.push_back(pieceQueue[positions[playerIndex] + i]);
		return queue;
	}
};