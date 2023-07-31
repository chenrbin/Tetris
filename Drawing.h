#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include "TetrisConstants.h"
#include "Tile.h"
#include "Mechanisms.h"
using namespace TetrisVariables;

// Class for easy sf::Text generation. Replaces the setText function
class SfTextAtHome : public sf::Text {
public:
	SfTextAtHome() {}
	SfTextAtHome(sf::Font& font, sf::Color color, string message, unsigned int textSize, sf::Vector2f position, bool bold = true, bool underlined = false, bool centered = false, bool outline = false) {
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
class SfRectangleAtHome : public sf::RectangleShape {
public:
	SfRectangleAtHome() {}
	SfRectangleAtHome(sf::Color color, sf::Vector2f size, sf::Vector2f position, bool centered = false, sf::Color outlineColor = sf::Color(), float outlineThickness = 0) {
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

// Class to organize drawable sprites of a checkbox. Purely visual functionality.
class Checkbox : public sf::Drawable{
protected:
	SfRectangleAtHome box;
	sf::FloatRect bounds;
	SfTextAtHome check, hoverCheck;
	bool checked, hovering;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		target.draw(box, states);
		if (checked)
			target.draw(check, states);
		else if (hovering)
			target.draw(hoverCheck, states);
	}
public:
	Checkbox(float size, float left, float top, bool checked, sf::Font& font) {
		// White outline box
		box = SfRectangleAtHome(BLUE, { size, size }, { left, top }, false, WHITE, LINEWIDTH);
		bounds = box.getGlobalBounds();

		// "X" to toggle
		check = SfTextAtHome(font, WHITE, "X", size, { bounds.left + bounds.width / 2, bounds.top + bounds.height / 2 }, true, false, true);
		check.move(0, -check.getGlobalBounds().height / 2);
		sf::FloatRect textBounds = check.getGlobalBounds();
		this->checked = checked;

		hoverCheck = check;
		hoverCheck.setFillColor(HOVERCHECKBOX);
		hovering = false;
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

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		target.draw(box, states);
		target.draw(check, states);
		target.draw(leftArrow, states);
		target.draw(rightArrow, states);
	}
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
	virtual void drawAnimation(sf::RenderWindow& window) = 0;
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
	void drawAnimation(sf::RenderWindow& window) {
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

// Class to display a player's death screen. Made from an empty board of tiles
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
	void drawAnimation(sf::RenderWindow& window) {
		// Does not execute if animation duration is over
		if (startTime.getTimeSeconds() > duration + endDuration)
			return;
		for (int i = 0; i < NUMROWS; i++)
			if (startTime.getTimeSeconds() / duration * NUMROWS >= i - 1)
				for (int j = 0; j < NUMCOLS; j++)
					window.draw(board[REALNUMROWS - i - 1][j]);
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
class GarbageStack : public sf::Drawable{
	vector<sf::RectangleShape> stack;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		for (int i = 0; i < stack.size(); i++) {
			target.draw(stack[i], states);
		}
	}
public:
	// Construct a stack of rectangles at position relative to gamePos
	GarbageStack(sf::Vector2f gamePos) {
		for (int i = 0; i < NUMROWS; i++) {
			sf::RectangleShape rec({ TILESIZE / 2.0f, TILESIZE - 1 });
			rec.setPosition(gamePos.x - TILESIZE / 2.0f, gamePos.y + GAMEHEIGHT - (i + 1) * TILESIZE + LINEWIDTH + 1);
			rec.setOutlineColor(WHITE);
			rec.setOutlineThickness(1);
			stack.push_back(rec);
		}
	}
	// Update stack visuals based on a vector of garbage timers
	void updateStack(vector<float> vec) {
		for (int i = 0; i < 20; i++)
		{
			// (255 * vec[i] + 255) / 2 sets the red value to 127-255 based on timer progress
			if (vec.size() > i) {
				stack[i].setFillColor(sf::Color( (255 * vec[i] + 255) / 2, 0, 0 ));
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

// Class for a text that can be navigated and clicked
class ClickableMenu : public sf::Drawable{
	vector<SfTextAtHome> texts;
	sf::CircleShape cursor; // Takes in a shape as the cursor. Must preset attributes.
	int cursorPos;

	// Draw all menu components
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		for (int i = 0; i < texts.size(); i++)
			target.draw(texts[i], states);
		target.draw(cursor, states);
	}
public:
	ClickableMenu(sf::Font& font, sf::Color color, vector<string>& menuText, int textSize, sf::Vector2f startPos, int spacing, sf::CircleShape& cursor) {
		for (int i = 0; i < menuText.size(); i++)
			texts.push_back(SfTextAtHome(font, color, menuText[i], textSize, { startPos.x, startPos.y + spacing * i }));
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
class ClickableButton : public sf::Drawable{
	SfRectangleAtHome button;
	SfTextAtHome text;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		target.draw(button, states);
		target.draw(text, states);
	}
public:
	ClickableButton(sf::Vector2f buttonSize, sf::Vector2f position, sf::Font& font, string message, const sf::Color& buttonColor = BLACK, const sf::Color& textColor = WHITE) {
		button = SfRectangleAtHome(buttonColor, { buttonSize.x, buttonSize.y }, { position.x, position.y }, true, BLACK, 1);
		text = SfTextAtHome(font, textColor, message, buttonSize.y, position, true, false, true);
		text.move(0, -text.getGlobalBounds().height / 2);
	}
};

// Base class for game setting
struct OptionSelector : public sf::Drawable {
	OptionSelector() {};
	// Values return as a string and will need to be manually converted to numbers.
	virtual string getValue() = 0;
	// Move all sprites
	virtual void move(float offsetX, float offsetY) = 0;

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

private: 
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const = 0;
};

// A sliding bar to select values
class SlidingBar : public OptionSelector
{
	// Sprites to draw
	SfRectangleAtHome bar;
	vector<SfRectangleAtHome> nodes;
	vector<SfTextAtHome> valuesText;
	sf::CircleShape cursor;

	// Data to handle
	vector<string> values;
	int nodeCount;
	int cursorIndex;
	bool cursorPressed;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		target.draw(bar, states);
		for (int i = 0; i < nodeCount; i++) {
			target.draw(nodes[i], states);
			target.draw(valuesText[i], states);
		}
		target.draw(cursor);
	}
public:
	SlidingBar(float length, vector<string> values, sf::Font& font) {
		nodeCount = values.size();
		this->values = values;

		// Create bar. Width is specified but height is fixed.
		bar = SfRectangleAtHome(WHITE, { length, BARHEIGHT }, { 0, 0 - BARHEIGHT / 2 }, false, BLACK, 1);

		// Create nodes
		for (int i = 0; i < nodeCount; i++) {
			nodes.push_back(SfRectangleAtHome(WHITE, { NODEWIDTH, NODEHEIGHT }, { 0 + i * length / (nodeCount - 1), 0 }, true, BLACK, 1));
			valuesText.push_back(SfTextAtHome(font, WHITE, values[i], MENUTEXTSIZE, { 0 + i * length / (nodeCount - 1), NODEHEIGHT }, true, false, true, true));
		}

		// Create circle cursor
		// TODO sfCircleAtHome
		cursor.setRadius(BAR_CURSOR_RADIUS);
		cursor.setFillColor(WHITE);
		cursor.setOrigin(BAR_CURSOR_RADIUS, BAR_CURSOR_RADIUS);
		cursor.setOutlineColor(BLACK);
		cursor.setOutlineThickness(1);
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
		for (SfRectangleAtHome& node : nodes)
			node.move(offsetX, offsetY);
		for (SfTextAtHome& text : valuesText)
			text.move(offsetX, offsetY);
		cursor.move(offsetX, offsetY);
	}
	// Click nodes to select option
	bool clickNodes(int mouseX, int mouseY) {
		for (SfRectangleAtHome node : nodes) {
			node.setScale(3, 2); // Scale the copy of the node for a bigger collision box
			if (node.contains(mouseX, mouseY)) {
				setCursorPressed(true);
				return moveCursor(mouseX);
			}
		}
		return false;
	}
	bool processMouseMove(int mouseX, int mouseY) {
		return moveCursor(mouseX);
	}
	bool processMouseClick(int mouseX, int mouseY) {
		if (getCursorBounds().contains(mouseX, mouseY)) 
			return setCursorPressed(true);
		return clickNodes(mouseX, mouseY);
	}
	bool processMouseRelease() {
		return setCursorPressed(false);
	}
};

// A switch that can be turned on or off
class onOffSwitch {

};

class SettingsTab : public sf::Drawable{
	SfRectangleAtHome tabRect;
	SfTextAtHome tabText;
	sf::FloatRect tabBounds; // Used to align tab rectangle and text
	int index; // First tab has index of 0
	bool tabSelected; // The currently selected tab is violet and taller, otherwise gray

	vector<SfTextAtHome> settingsTexts;
	vector<OptionSelector*> settingSelectors;
	int settingCount;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		// Always draw the tab itself
		target.draw(tabRect, states);
		target.draw(tabText, states);

		// Draw contents if current tab is selected
		if (tabSelected) {
			for (int i = 0; i < settingCount; i++)
			{
				target.draw(settingsTexts[i], states);
				target.draw(*settingSelectors[i], states);
			}
		}
	}
public:
	SettingsTab(sf::Font& font, string name, int index) {
		// Rect origin is set at 0, 0 and text origin is centered
		tabRect = SfRectangleAtHome(GRAY, { 0, 0 }, { 0, 0 }, false, BLACK, 2);
		tabText = SfTextAtHome(font, WHITE, name, MENUTEXTSIZE, { 0, 0 }, true, false, true, true);
		this->index = index;
		tabSelected = false;
		settingCount = 0;
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
	void addSetting(string text, OptionSelector* selector, bool selectorPositionRight, sf::Font& font) {
		// Determine sprite position based on setting count
		float xPos, yPos;
		if (settingCount < 7)
			xPos = SETTINGXPOS;
		else // Next column
			xPos = SETTINGXPOS + 300;
		yPos = SETTINGYPOS + settingCount * SETTINGSPACING;
		
		settingsTexts.push_back(SfTextAtHome(font, WHITE, text, MENUTEXTSIZE, { xPos, yPos }));
		settingSelectors.push_back(selector);
		if (selectorPositionRight)
			selector->move(xPos + SELECTORRIGHTSPACING, yPos);
		else
			selector->move(xPos, yPos + SELECTORDOWNSPACING);
		settingCount++;
	}
	// Check all mechanisms on mouse click position
	void processMouseMove(int mouseX, int mouseY) {
		for (OptionSelector* selector : settingSelectors)
			if (selector->processMouseMove(mouseX, mouseY)) 
				break; // Break after a successful action. Skips the need to check everything
	}
	void processMouseClick(int mouseX, int mouseY) {
		for (OptionSelector* selector : settingSelectors)
			if (selector->processMouseClick(mouseX, mouseY))
				break; // Since no sprites overlap, this saves the need to check all bounds;
	}
	void processMouseRelease() {
		for (OptionSelector* selector : settingSelectors)
			selector->processMouseRelease();
	}
	
};
class SettingsMenu : public sf::Drawable{
	vector<SettingsTab> tabs;
	int tabCount;
	int currentTabIndex;

	// Draw all tabs
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		for (int i = 0; i < tabCount; i++)
			target.draw(tabs[i], states);
	}
public:
	SettingsMenu() {
		tabCount = 0;
		currentTabIndex = 0;
	}
	void addTab(sf::Font& font, string name) {
		tabs.push_back(SettingsTab(font, name, tabCount++));
		alignTabs();
	}
	// Align tab positions across top of screen based on number of tabs
	void alignTabs() {
		for (int i = 0; i < tabs.size(); i++)
			tabs[i].setBounds(WIDTH * i / tabs.size() + 1.0f, TABTOP, WIDTH / tabs.size(), TABHEIGHT);
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
	SettingsTab& operator[](int index) {
		return tabs[index];
	}
};

// Class for pause screen sprites
