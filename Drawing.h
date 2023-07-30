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
		box = sf::RectangleShape({ size, size });
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
	Animation() {
		duration = 0;
	}
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
			sf::RectangleShape rec({ TILESIZE / 2.0f, TILESIZE - 1 });
			rec.setPosition(gamePos.x - TILESIZE / 2.0f, gamePos.y + GAMEHEIGHT - (i + 1) * TILESIZE + LINEWIDTH + 1);
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
	sfTextAtHome() {}
	sfTextAtHome(sf::Font& font, sf::Color color, string message, unsigned int textSize, sf::Vector2f position, bool bold = true, bool underlined = false, bool centered = false, bool outline = false) {
		setFont(font);
		setFillColor(color);
		setString(message);
		setCharacterSize(textSize);
		if (centered)
			alignCenter();
		setPosition(position);
		if (bold)
			setStyle(sf::Text::Bold);
		if (underlined)
			setStyle(sf::Text::Underlined);
		if (outline)
		{
			setOutlineColor(BLACK);
			setOutlineThickness(1);
		}
	}
	void alignCenter() {
		const sf::FloatRect box = getLocalBounds();
		setOrigin(box.width / 2.0f, box.height / 2.0f);
	}
	bool contains(float x, float y) {
		return getGlobalBounds().contains(x, y);
	}
};

// Class for easy sf::RectangleShape generation.
class sfRectangleAtHome : public sf::RectangleShape{
public:
	sfRectangleAtHome() {}
	sfRectangleAtHome(sf::Color color, sf::Vector2f size, sf::Vector2f position, bool centered = false, sf::Color outlineColor = sf::Color(), float outlineThickness = 0) {
		setSize(size);
		setFillColor(color);
		if (centered)
			alignCenter();
		setPosition(position);
		setOutlineColor(outlineColor);
		setOutlineThickness(outlineThickness);
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
			texts.push_back(sfTextAtHome(font, color, menuText[i], textSize, { startPos.x, startPos.y + spacing * i }));
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
	// Update cursor based on mouse position. Return true if a text is selected
	bool updateMouse(int x, int y) {
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

// Class for a button that can be clicked
class clickableButton {
	sfRectangleAtHome button;
	sfTextAtHome text;
public:
	clickableButton(sf::Vector2f buttonSize, sf::Vector2f position, string message, const sf::Color& buttonColor = BLACK, const sf::Color& textColor = WHITE) {
		button = sfRectangleAtHome(buttonColor, { buttonSize.x, buttonSize.y }, { position.x, position.y }, true, BLACK, 1);
	}
};

// Base class for game setting
struct optionSelector {
	optionSelector() {};
	// Values return as a string and will need to be manually converted to numbers.
	virtual string getValue() = 0;
	// Move all sprites
	virtual void move(float offsetX, float offsetY) = 0;
	virtual void draw(sf::RenderWindow& window) = 0;

	// React to mouse movement, click, and release. 
	// Returns whether a successful action is performed for efficiency purposes
	virtual bool processMouseMove(int mouseX, int mouseY) {
		return false;
	}
	virtual bool processMouseClick(int mouseX, int mouseY) {
		return false;
	}
	virtual bool processMouseRelease() {
		return false;
	}

};

// A sliding bar to select values
class slidingBar : public optionSelector
{
	// Sprites to draw
	sfRectangleAtHome bar;
	vector<sfRectangleAtHome> nodes;
	vector<sfTextAtHome> valuesText;
	sf::CircleShape cursor;

	// Data to handle
	vector<string> values;
	int nodeCount;
	int cursorIndex;
	bool cursorPressed;
public:
	slidingBar(float length, vector<string> values, sf::Font& font) {
		nodeCount = values.size();
		this->values = values;

		// Create bar. Width is specified but height is fixed.
		bar = sfRectangleAtHome(WHITE, { length, BARHEIGHT }, { 0, 0 - BARHEIGHT / 2 }, false, BLACK, 1);

		// Create nodes
		for (int i = 0; i < nodeCount; i++) {
			nodes.push_back(sfRectangleAtHome(WHITE, { NODEWIDTH, NODEHEIGHT }, { 0 + i * length / (nodeCount - 1), 0 }, true, BLACK, 1));
			valuesText.push_back(sfTextAtHome(font, WHITE, values[i], MENUTEXTSIZE, { 0 + i * length / (nodeCount - 1), NODEHEIGHT }, true, false, true, true));
		}

		// Create circle cursor
		// TODO sfCircleAtHome
		cursor.setRadius(BAR_CURSOR_RADIUS);
		cursor.setFillColor(WHITE);
		cursor.setOrigin(BAR_CURSOR_RADIUS, BAR_CURSOR_RADIUS);
		cursor.setOutlineColor(BLACK);
		cursor.setOutlineThickness(1);
	}
	// Draw all sprites
	void draw(sf::RenderWindow& window) {
		window.draw(bar);
		for (int i = 0; i < nodeCount; i++) {
			window.draw(nodes[i]);
			window.draw(valuesText[i]);
		}
		window.draw(cursor);
	}
	// Returns the value at the cursor
	string getValue(){
		return values[cursorIndex];
	}
	// Move the cursor based on mouse x position to select a node
	bool moveCursor(float xPosition) {
		if (!cursorPressed)
			return false;
		for (int i = 0; i < nodeCount; i++) {
			sf::FloatRect nodePos = nodes[i].getGlobalBounds();
			// Collision detections extends two node widths out both sides
			if (xPosition > nodePos.left - NODEWIDTH * 2 && xPosition < nodePos.left + NODEWIDTH * 3) {
				cursorIndex = i;
				cursor.setPosition(nodePos.left + NODEWIDTH / 2.0f, nodePos.top + NODEHEIGHT / 2.0f);
				return true;
			}
		}
		return false;
	}
	bool setCursorPressed(bool val) {
		if (val == cursorPressed) // Do nothing if variable does not need to be changed
			return false;
		cursorPressed = val;
		// Visual indicator that a cursor has been clicked
		if (val) { 
			cursor.setRadius(BAR_CURSOR_RADIUS + BAR_CURSOR_GROWTH);
			cursor.move(-BAR_CURSOR_GROWTH, -BAR_CURSOR_GROWTH);
			return true;
		}
		else {
			cursor.setRadius(BAR_CURSOR_RADIUS);
			cursor.move(BAR_CURSOR_GROWTH, BAR_CURSOR_GROWTH);
			return false;
		}
	}
	sf::FloatRect getCursorBounds() {
		return cursor.getGlobalBounds();
	}
	// Move all sprites
	void move(float offsetX, float offsetY) {
		bar.move(offsetX, offsetY);
		for (sfRectangleAtHome& node : nodes)
			node.move(offsetX, offsetY);
		for (sfTextAtHome& text : valuesText)
			text.move(offsetX, offsetY);
		cursor.move(offsetX, offsetY);
	}
	bool processMouseMove(int mouseX, int mouseY) {
		return moveCursor(mouseX);
	}
	bool processMouseClick(int mouseX, int mouseY) {
		if (getCursorBounds().contains(mouseX, mouseY))
			return setCursorPressed(true);
		return false;
	}
	bool processMouseRelease() {
		return setCursorPressed(false);
	}
};

// A switch that can be turned on or off
class onOffSwitch {

};

class settingsTab {
	sfRectangleAtHome tabRect;
	sfTextAtHome tabText;
	sf::FloatRect tabBounds; // Used to align tab rectangle and text
	int index; // First tab has index of 0
	bool tabSelected; // The currently selected tab is violet and taller, otherwise gray

	vector<sfTextAtHome> settingsTexts;
	vector<optionSelector*> settingSelectors;
	int settingCount;
public:
	settingsTab(sf::Font& font, string name, int index) {
		// Rect origin is set at 0, 0 and text origin is centered
		tabRect = sfRectangleAtHome(GRAY, { 0, 0 }, { 0, 0 }, false, BLACK, 2);
		tabText = sfTextAtHome(font, WHITE, name, MENUTEXTSIZE, { 0, 0 }, true, false, true, true);
		this->index = index;
		tabSelected = false;
		settingCount = 0;
	}
	void draw(sf::RenderWindow& window) {
		// Always draw the tab itself
		window.draw(tabRect);
		window.draw(tabText);

		// Draw contents if current tab is selected
		if (tabSelected) {
			for (int i = 0; i < settingCount; i++)
			{
				window.draw(settingsTexts[i]);
				settingSelectors[i]->draw(window);
			}
		}
	}
	// Set tab size and position based on float rect
	void setBounds(float left, float top, float width, float height) {
		tabBounds = sf::FloatRect(left, top, width, height);
		tabRect.setSize({ width, height });
		tabRect.setPosition(left, top);
		tabText.setPosition(left + width / 2, top + height / 2);
	}
	void setRectColor(const sf::Color& color) {
		tabRect.setFillColor(color);
	}
	void setSelected(bool val) {
		if (tabSelected == val)
			return;
		tabSelected = val;
		// Change tab visuals accordingly
		if (tabSelected) {
			tabRect.setFillColor(VIOLET);
			// Change width slightly to align outlines
			setBounds(tabBounds.left, tabBounds.top, tabBounds.width - 2, tabBounds.height + TABHEIGHTGROWTH);
		}
		else {
			tabRect.setFillColor(GRAY);
			setBounds(tabBounds.left, tabBounds.top, tabBounds.width + 2, tabBounds.height - TABHEIGHTGROWTH);
		}
	}
	int getIndex() {
		return index;
	}
	sf::FloatRect getTabBounds() {
		return tabBounds;
	}
	// Add a setting option and its selection mechanism. True to place mechanism to the right of text, false for below.
	void addSetting(string text, optionSelector* selector, bool selectorPositionRight, sf::Font& font) {
		// Determine sprite position based on setting count
		float xPos, yPos;
		if (settingCount < 7)
			xPos = SETTINGXPOS;
		else // Next column
			xPos = SETTINGXPOS + 300;
		yPos = SETTINGYPOS + settingCount * SETTINGSPACING;
		
		settingsTexts.push_back(sfTextAtHome(font, WHITE, text, MENUTEXTSIZE, { xPos, yPos }));
		settingSelectors.push_back(selector);
		if (selectorPositionRight)
			selector->move(xPos + SELECTORRIGHTSPACING, yPos);
		else
			selector->move(xPos, yPos + SELECTORDOWNSPACING);
		settingCount++;
	}
	// Check all mechanisms on mouse click position
	void processMouseMove(int mouseX, int mouseY) {
		for (optionSelector* selector : settingSelectors)
			if (selector->processMouseMove(mouseX, mouseY)) 
				break; // Break after a successful action. Skips the need to check everything
	}
	void processMouseClick(int mouseX, int mouseY) {
		for (optionSelector* selector : settingSelectors)
			if (selector->processMouseClick(mouseX, mouseY))
				break; // Since no sprites overlap, this saves the need to check all bounds;
	}
	void processMouseRelease() {
		for (optionSelector* selector : settingSelectors)
			selector->processMouseRelease();
	}
	
};
class settingsMenu {
	vector<settingsTab> tabs;
	int tabCount;
	int currentTabIndex;
public:
	settingsMenu() {
		tabCount = 0;
		currentTabIndex = 0;
	}
	void addTab(sf::Font& font, string name) {
		tabs.push_back(settingsTab(font, name, tabCount++));
		alignTabs();
	}
	// Align tab positions across top of screen based on number of tabs
	void alignTabs() {
		for (int i = 0; i < tabs.size(); i++)
			tabs[i].setBounds(WIDTH * i / tabs.size() + 1.0f, TABTOP, WIDTH / tabs.size(), TABHEIGHT);
	}
	// Draw all tabs
	void draw(sf::RenderWindow& window) {
		for (settingsTab& tab : tabs)
			tab.draw(window);
	}
	// Select specific tab. Should be called once after creating tabs to select default
	void selectTab(int index) {
		for (int i = 0; i < tabCount; i++)
			if (i == index)
				tabs[i].setSelected(true);
			else
				tabs[i].setSelected(false);
	}
	// Process clicking to select a tab
	void clickTab(float xPos, float yPos) {
		for (int i = 0; i < tabCount; i++)
			if (tabs[i].getTabBounds().contains(xPos, yPos)) {
				selectTab(i);
				currentTabIndex = i;
				break;
			}
	}
	// Process mouse events for current tab
	void processMouseMove(int mouseX, int mouseY) {
		tabs[currentTabIndex].processMouseMove(mouseX, mouseY);
	}
	void processMouseClick(int mouseX, int mouseY) {
		tabs[currentTabIndex].processMouseClick(mouseX, mouseY);
	}
	void processMouseRelease() {
		tabs[currentTabIndex].processMouseRelease();
	}
	settingsTab& operator[](int index) {
		return tabs[index];
	}

};