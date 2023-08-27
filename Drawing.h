#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include "TetrisConstants.h"
#include "Tile.h"
#include "Mechanisms.h"
using namespace TetrisVariables;

#pragma region Sf Sprites At Home
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
#pragma endregion

#pragma region Sandbox Checkboxes
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
	sf::FloatRect getLeftBound() {
		return leftBound;
	}
	sf::FloatRect getRightBound() {
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
#pragma endregion

#pragma region Animations
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
#pragma endregion

// Class to display garbage bin. Actual mechanism is in Mechanisms.h
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
	ClickableMenu(){
		cursorPos = 0;
	}
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
	bool updateMouseClick(int x, int y) {
		for (int i = 0; i < texts.size(); i++) {
			if (texts[i].contains(x, y)) {
				cursorPos = i;
				updateCursor();
				return true;
			}
		}
		return false;
	}
	// Slightly different from mouse click. Needed for sound effect conditionals
	bool updateMouseMove(int x, int y) {
		for (int i = 0; i < texts.size(); i++) {
			if (texts[i].contains(x, y)) {
				if (cursorPos != i) {
					cursorPos = i;
					updateCursor();
					return true;
				}
				else
					return false;
			}
		}
		return false;
	}
	int getCursorPos() {
		return cursorPos;
	}
	void resetCursorPos() {
		cursorPos = 0;
		updateCursor();
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
	ClickableButton() {}
	ClickableButton(sf::Vector2f buttonSize, sf::Vector2f position, sf::Font& font, string message, int textSize, const sf::Color& buttonColor = BLACK, const sf::Color& textColor = WHITE) {
		button = SfRectangleAtHome(buttonColor, { buttonSize.x, buttonSize.y }, { position.x, position.y }, true, BLACK, 1);
		text = SfTextAtHome(font, textColor, message, textSize, position, true, false, true, true);
		text.setOrigin(text.getOrigin().x, 0);
		text.setPosition(position.x, button.getGlobalBounds().top);
	}
	bool checkClick(int mouseX, int mouseY) {
		return button.contains(mouseX, mouseY);
	}
	void setFillColor(const sf::Color& color) {
		button.setFillColor(color);
	}
	void move(float offsetX, float offsetY) {
		button.move(offsetX, offsetY);
		text.move(offsetX, offsetY);
	}
};

#pragma region Option Selectors
// Base class for game setting. All selectors are initialized with position 0 and move as they are added to tabs
struct OptionSelector : public sf::Drawable {
	OptionSelector() {};
	// Return an index used for storing in files and reading
	virtual int getValue() = 0;
	// Set the status/index/position of the selector
	virtual void setIndex(int index) = 0;
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
	int getValue(){
		return cursorIndex;
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
	// Go to a specific node based on its index
	void setIndex(int index) {
		if (index < nodes.size())
		{
			setCursorPressed(true);
			moveCursor(nodes[index].getGlobalBounds().left);
			setCursorPressed(false);
		}
		else
		{
			cout << "Error, invalid index for sliding bar.\n";
			throw ConfigError();
		}
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
class OnOffSwitch : public OptionSelector {
	ClickableButton switchBase;
	SfRectangleAtHome switchCover;
	bool isOn;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		target.draw(switchBase, states);
		target.draw(switchCover, states);
	}
public:
	// Switch is set to true by default
	OnOffSwitch(sf::Font& font) {
		switchBase = ClickableButton({ SWITCHWIDTH, SWITCHHEIGHT }, ORIGIN, font, "OFF ON ", SWITCHHEIGHT * 2 / 3.0f, GREEN);
		switchCover = SfRectangleAtHome(GRAY, { SWITCHWIDTH / 2.0f, SWITCHHEIGHT }, {-SWITCHWIDTH / 4.0f, 0}, true, BLACK, 1);
		move(SWITCHWIDTH / 2.0f, SWITCHHEIGHT / 2.0f); // Initial movement to offset the origin change used to center the text
		isOn = true;
	}
	int getValue() {
		return isOn;
	}
	// Set switch to false (0) or true (1)
	void setIndex(int index) {
		if (index != 0 && index != 1) {
			cout << "Error, invalid index for on/off switch.\n";
			throw ConfigError();
		}
		if ((index == 0 && isOn) || (index == 1 && !isOn))
			clickButton(switchCover.getGlobalBounds().left, switchCover.getGlobalBounds().top);
	}
	// Move all sprites
	virtual void move(float offsetX, float offsetY) {
		switchBase.move(offsetX, offsetY);
		switchCover.move(offsetX, offsetY);
	}
	// Process clicking the button
	bool clickButton(int mouseX, int mouseY) {
		if (switchBase.checkClick(mouseX, mouseY))
		{
			if (isOn) {
				switchBase.setFillColor(RED);
				switchCover.move(switchCover.getGlobalBounds().width, 0);
			}
			else {
				switchBase.setFillColor(GREEN);
				switchCover.move(-switchCover.getGlobalBounds().width, 0);
			}
			isOn = !isOn;
			return true;
		}
		return false;
	}
	virtual bool processMouseMove(int mouseX, int mouseY) {
		return false;
	}
	virtual bool processMouseClick(int mouseX, int mouseY) {
		return clickButton(mouseX, mouseY);
	}
	virtual bool processMouseRelease() {
		return false;
	}
};

// Class to store custom keybinds
class KeyRecorder : public OptionSelector {
	// Sprites
	SfRectangleAtHome rect;
	SfTextAtHome text;

	sf::Keyboard::Key key; // Key object to detect
	map<sf::Keyboard::Key, string>* keyStrings; // Map for nontext key strings
	
	// Selection mechanism
	bool isSelected;
	string tempString; // Used to update the correct string after selecting a recorder

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		target.draw(rect, states);
		target.draw(text, states);
	}
public:
	KeyRecorder(map<sf::Keyboard::Key, string>* keyStrings, sf::Font& font) {
		this->keyStrings = keyStrings;
		rect = SfRectangleAtHome(GRAY, { 128, 32 }, { 16, 16 }, true, BLACK, 1);
		text = SfTextAtHome(font, WHITE, "INVALID KEY", BUTTONTEXTSIZE, { 16, 6 }, true, false, true, true);
		isSelected = false;
		tempString = "";
	}
	// Reads and records the key. Return true upon success
	bool readKey(sf::Keyboard::Key key) {
		if (!isSelected || key == -1) // -1 is any invalid key
			return false;
		auto iter = keyStrings->find(key);
		if (iter != keyStrings->end()) {
			updateString(iter->second);
			isSelected = false;
			this->key = key;
			return true;
		}
		return false;
	}
	int getValue() {
		return key;
	}
	// Set key to key code
	void setIndex(int index) {
		readKey(sf::Keyboard::Key(index));
	}
	sf::Keyboard::Key* getKey() {
		return &key;
	}
	void move(float offsetX, float offsetY) {
		rect.move(offsetX, offsetY);
		text.move(offsetX, offsetY);
	}
	void updateString(string str) {
		text.setString(str);
		text.alignCenter();
		text.setPosition(rect.getPosition().x, rect.getPosition().y - 10);
	}
	bool getSelected() {
		return isSelected;
	}
	void setSelect(bool val) {
		if (isSelected == val)
			return;
		if (val) {
			isSelected = true;
			tempString = text.getString();
			updateString("___");
		}
		else {
			isSelected = false;
			updateString(tempString);
		}
	}
	
	virtual bool processMouseMove(int mouseX, int mouseY) {
		return false;
	}
	virtual bool processMouseClick(int mouseX, int mouseY) {
		// Has to iterate through every recorder to deselect on click.
		setSelect(rect.contains(mouseX, mouseY));
		return false;
	}
	virtual bool processMouseRelease() {
		return false;
	}
};
#pragma endregion

class SettingsTab : public sf::Drawable{
	SfRectangleAtHome tabRect;
	SfTextAtHome tabText;
	sf::FloatRect tabBounds; // Used to align tab rectangle and text
	int index; // First tab has index of 0
	bool tabSelected; // The currently selected tab is violet and taller, otherwise gray

	vector<SfTextAtHome> settingsTexts;
	vector<OptionSelector*> settingSelectors;
	vector<KeyRecorder*> keybinds; // Keybinds stored separately from selectors
	int settingCount;

	vector<sf::Text> extraText; // Any additional text to draw

	SoundManager* soundFX;
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
			for (int i = 0; i < extraText.size(); i++)
				target.draw(extraText[i], states);
		}
		
	}
public:
	SettingsTab(sf::Font& font, string name, int index, SoundManager* soundFX) {
		// Rect origin is set at 0, 0 and text origin is centered
		tabRect = SfRectangleAtHome(GRAY, { 0, 0 }, { 0, 0 }, false, BLACK, 2);
		tabText = SfTextAtHome(font, WHITE, name, MENUTEXTSIZE, { 0, 0 }, true, false, true, true);
		this->index = index;
		tabSelected = false;
		settingCount = 0;
		this->soundFX = soundFX;
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
	vector<int> getValues() {
		vector<int> val;
		for (OptionSelector* selector : settingSelectors)
			val.push_back(selector->getValue());
		return val;
	}
	vector<OptionSelector*>& getSelectors() {
		return settingSelectors;
	}
	// Add a setting option and its selection mechanism.
	void addSetting(string text, sf::Vector2f textPosition, OptionSelector* selector, sf::Vector2f selectorPosition, sf::Font& font) {
		settingsTexts.push_back(SfTextAtHome(font, WHITE, text, MENUTEXTSIZE, textPosition));
		settingSelectors.push_back(selector);
		selector->move(selectorPosition.x, selectorPosition.y);
		settingCount++;
	}
		// Add additional text to draw
	void addExtraText(sf::Text text) {
		extraText.push_back(text);
	}
	// Add a setting option while storing a keybind for extra operations
	void addKeybind(string text, sf::Vector2f textPosition, map<sf::Keyboard::Key, string>* keyStrings, sf::Vector2f selectorPosition, sf::Font& font) {
		keybinds.push_back(new KeyRecorder(keyStrings, font));
		addSetting(text, textPosition, keybinds[keybinds.size() - 1], selectorPosition, font);
	}
	// Reads and records the key
	
	void readKeys(sf::Keyboard::Key key) {
		for (KeyRecorder* keyRec : keybinds) {
			if (keyRec->getSelected() && keyRec->readKey(key)){
				soundFX->play(LIGHTTAP);
				break;
			}
		}
	}
	vector<KeyRecorder*>& getKeybinds() {
		return keybinds;
	}
	void setKey(int index, sf::Keyboard::Key key) {
		keybinds[index]->setSelect(true);
		keybinds[index]->readKey(key);
	}
	
	OptionSelector& operator[](int index) {
		return *settingSelectors[index];
	}
	// Check all mechanisms on mouse click position
	void processMouseMove(int mouseX, int mouseY) {
		for (OptionSelector* selector : settingSelectors)
			if (selector->processMouseMove(mouseX, mouseY)) 
				break; // Break after a successful action. Skips the need to check everything
	}
	void processMouseClick(int mouseX, int mouseY) {
		for (OptionSelector* selector : settingSelectors)
			if (selector->processMouseClick(mouseX, mouseY)) {
				soundFX->play(LIGHTTAP);
				break; // Since no sprites overlap, this saves the need to check all bounds;
			}
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
	SoundManager* soundFX;

	// Draw all tabs
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		for (int i = 0; i < tabCount; i++)
			target.draw(tabs[i], states);
	}
public:
	SettingsMenu(SoundManager* soundFX) {
		tabCount = 0;
		currentTabIndex = 0;
		this->soundFX = soundFX;
	}
	void addTab(sf::Font& font, string name) {
		tabs.push_back(SettingsTab(font, name, tabCount++, soundFX));
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
				soundFX->play(LIGHTTAP);
				break;
			}
	}
	// Process mouse events for current tab
	void processMouseMove(int mouseX, int mouseY) {
		tabs[currentTabIndex].processMouseMove(mouseX, mouseY);
	}
	void processMouseClick(int mouseX, int mouseY) {
		clickTab(mouseX, mouseY);
		tabs[currentTabIndex].processMouseClick(mouseX, mouseY);
	}
	void processMouseRelease() {
		tabs[currentTabIndex].processMouseRelease();
	}
	SettingsTab& operator[](int index) {
		return tabs[index];
	}
	// Get a vector of integers that represent the setting contents
	vector<int> getValues() {
		vector<int> vec;
		for (SettingsTab& tab : tabs)
			for (int val : tab.getValues())
				vec.push_back(val);
		return vec;
	}
	// Configure settings based on int vector from config file
	void applyConfig(vector<int> config) {
		try {
			int tab1Size = tabs[0].getSelectors().size(); // Index to separate tabs
			for (int i = 0; i < tab1Size; i++) // First tab
				tabs[0][i].setIndex(config[i]);
			for (int i = 0; i < tabs[1].getSelectors().size(); i++) { // Second tab
				tabs[1].setKey(i, sf::Keyboard::Key(config.at(i + tab1Size)));
			}
		}
		catch (out_of_range err) {
			cout << "applyConfig out of range. Restoring to default\n";
			applyConfig(DEFAULTSETTINGS);
		}
		catch (ConfigError err) {
			applyConfig(DEFAULTSETTINGS);
		}
	}
};

// Class for pause screen sprites. Varies based on mode
class pauseScreen : public sf::Drawable {
	// All Rectangles and shapes needed to construct the screen
	vector<SfRectangleAtHome> rectangles;
	vector<SfTextAtHome> texts;
	ClickableMenu menu;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		for (int i = 0; i < rectangles.size(); i++)
			target.draw(rectangles[i], states);
		for (int i = 0; i < texts.size(); i++)
			target.draw(texts[i], states);
		target.draw(menu, states);
	}
public:
	pauseScreen(sf::Vector2f gamePos, vector<string>& menuText, sf::Font& font) {
		rectangles.push_back(SfRectangleAtHome(BLACK, { GAMEWIDTH, GAMEHEIGHT + TOPROWPIXELS }, {gamePos.x, gamePos.y - TOPROWPIXELS}, false, WHITE, LINEWIDTH));
		rectangles.push_back(SfRectangleAtHome(BLACK, { TILESIZE * 4 - LINEWIDTH, TILESIZE * 4 }, { gamePos.x - TILESIZE * 4.5f - LINEWIDTH, gamePos.y + LINEWIDTH }, false, WHITE, LINEWIDTH));
		rectangles.push_back(SfRectangleAtHome(BLACK, { TILESIZE * 4, GAMEHEIGHT / 9 * NEXTPIECECOUNT }, { gamePos.x + GAMEWIDTH + LINEWIDTH, gamePos.y + LINEWIDTH }, false, WHITE, LINEWIDTH));

		// Offscreen rectangles for player two. Will have no text
		rectangles.push_back(SfRectangleAtHome(BLACK, { GAMEWIDTH, GAMEHEIGHT + TOPROWPIXELS }, { gamePos.x + WIDTH, gamePos.y - TOPROWPIXELS }, false, WHITE, LINEWIDTH));
		rectangles.push_back(SfRectangleAtHome(BLACK, { TILESIZE * 4 - LINEWIDTH, TILESIZE * 4 }, { gamePos.x + WIDTH - TILESIZE * 4.5f - LINEWIDTH, gamePos.y + LINEWIDTH }, false, WHITE, LINEWIDTH));
		rectangles.push_back(SfRectangleAtHome(BLACK, { TILESIZE * 4, GAMEHEIGHT / 9 * NEXTPIECECOUNT }, { gamePos.x + WIDTH + GAMEWIDTH + LINEWIDTH, gamePos.y + LINEWIDTH }, false, WHITE, LINEWIDTH));


		texts.push_back(SfTextAtHome(font, WHITE, "PAUSED", 40, { GAMEXPOS + GAMEWIDTH / 2, GAMEYPOS + GAMEWIDTH / 3 }, true, false, true));
	
		sf::CircleShape* cursor = new sf::CircleShape(15.f, 3); // Triangle shaped cursor
		cursor->rotate(90.f);
		menu = ClickableMenu(font, WHITE, menuText, MENUTEXTSIZE, { gamePos.x + GAMEWIDTH / 4, gamePos.y + GAMEWIDTH * 2 / 3 }, MENUSPACING, *cursor);
	}
	// Return a reference to the menu
	ClickableMenu& getMenu() {
		return menu;
	}
};
