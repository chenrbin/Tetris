#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <fstream>
#include "TetrisConstants.h"
#include "Screen.h"
#include <map>
#include "Mechanisms.h"
#include "Drawing.h"

using namespace std;
using namespace TetrisVariables;
// Ruobin Chen
// Tetris game made with SFML 2.5.1
// Sound effects and music made with IOS Garage Band
// Line count as of 8/31/2023: 4001

// Generate centered text entity. Can specify font, color, message, size, position, and style
sf::Text generateText(sf::Font& font, sf::Color color, string message, unsigned int textSize, sf::Vector2f coords, bool bold = true, bool underlined = false, bool centered = false) {
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
// Generate rectangle entity. Can specify dimensions, color, position, origin, outline
sf::RectangleShape generateRectangle(sf::Vector2f dimensions, sf::Color fillColor, sf::Vector2f position, sf::Vector2f origin = { 0, 0 }, sf::Color outlineColor = sf::Color(), float outlineThickness = 0)
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
vector<sf::RectangleShape> getGameRects(sf::Vector2f gamePos) {
	// Rectangles for game screen, hold, queue, garbage bin, and top row, 
	vector<sf::RectangleShape> rects;
	rects.push_back(generateRectangle({ GAMEWIDTH, GAMEHEIGHT }, BLACK,
		gamePos, { 0, 0 }, WHITE, LINEWIDTH));
	rects.push_back(generateRectangle({ TILESIZE * 4, TILESIZE * 4  }, BLACK,
		{ gamePos.x - TILESIZE * 4.5f - LINEWIDTH, gamePos.y + LINEWIDTH }, { 0,  0 }, WHITE, LINEWIDTH));
	rects.push_back(generateRectangle({ TILESIZE * 4, GAMEHEIGHT / 9 * NEXTPIECECOUNT },
		BLACK, { gamePos.x + GAMEWIDTH + LINEWIDTH, gamePos.y + LINEWIDTH }, { 0, 0 }, WHITE, LINEWIDTH));
	rects.push_back(generateRectangle({ TILESIZE / 2, GAMEHEIGHT - LINEWIDTH }, BLACK,
		{ gamePos.x - TILESIZE / 2 - LINEWIDTH, gamePos.y + LINEWIDTH }, { 0, 0 }, WHITE, LINEWIDTH));

	// Rectangles to show a couple pixels of the very top row
	rects.push_back(generateRectangle({ GAMEWIDTH, TOPROWPIXELS }, BLACK,
		{ gamePos.x, gamePos.y - TOPROWPIXELS }, { 0, 0 }, WHITE, LINEWIDTH));
	rects.push_back(generateRectangle({ GAMEWIDTH + 1, TILESIZE - 9 }, BLUE,
		{ gamePos.x - 1, gamePos.y - TILESIZE - 1 }));
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
		sf::RectangleShape line({ GAMEWIDTH, LINEWIDTH });
		line.setPosition(bounds.left, bounds.top + i * TILESIZE - 1);
		line.setFillColor(SEETHROUGH);
		lines.push_back(line);
	}
	for (int j = 1; j < NUMCOLS; j++) { // Vertical lines
		sf::RectangleShape line({ LINEWIDTH, GAMEHEIGHT + TOPROWPIXELS });
		line.setPosition(bounds.left + j * TILESIZE - 1, bounds.top - TOPROWPIXELS);
		line.setFillColor(SEETHROUGH);
		lines.push_back(line);
	}
	return lines;
}

// Generate all static text on game screen
vector<sf::Text> getGameText(vector<sf::FloatRect>& bounds, sf::Font& font) {
	// Redefine bounds for cleaner code
	sf::FloatRect& gameBounds = bounds[0], holdBounds = bounds[1], queueBounds = bounds[2];
	vector<sf::Text> textboxes;
	textboxes.push_back(generateText(font, WHITE, "Hold", GAMETEXTSIZE, { holdBounds.left + holdBounds.width / 2, holdBounds.top - GAMETEXTSIZE }, true, false, true));
	textboxes.push_back(generateText(font, WHITE, "Next", GAMETEXTSIZE, { queueBounds.left + queueBounds.width / 2, queueBounds.top - GAMETEXTSIZE }, true, false, true));
	textboxes.push_back(generateText(font, WHITE, "Classic Mode", GAMETEXTSIZE * 2, { gameBounds.left + gameBounds.width / 2, gameBounds.top - GAMETEXTSIZE * 2 }, true, false, true));
	return textboxes;
}
// Generate text exclusive to sandbox mode
vector<sf::Text> getSandboxText(sf::Font& font) {
	vector<sf::Text> textboxes;
	vector<string> menuItems = { "Auto-fall", "Fall speed", "Creative", "Reset", "Quit" };
	for (int i = 0; i < menuItems.size(); i++)
		textboxes.push_back(generateText(font, WHITE, menuItems[i], MENUTEXTSIZE, { SANDBOXMENUPOS.x, SANDBOXMENUPOS.y + MENUSPACING * i }));
	return textboxes;
}
// Generate text for loss screen
vector<sf::Text> getLossText(sf::Font& font) {
	vector<sf::Text> textboxes;
	textboxes.push_back(generateText(font, WHITE, "YOU LOST", GAMETEXTSIZE * 4, { WIDTH / 2, GAMEYPOS }, true, false, true));
	textboxes.push_back(generateText(font, WHITE, "Press any key to return to Main Menu", GAMETEXTSIZE, { WIDTH / 2, HEIGHT / 2 }, true, false, true));

	return textboxes;
}
// Generate fadeText animations
vector<FadeText> getFadeText(sf::Vector2f gamePos, sf::Font& font) {
	FadeText speedupText(generateText(font, WHITE, "SPEED UP", GAMETEXTSIZE * 2, { gamePos.x + GAMEWIDTH / 2, gamePos.y }, true, false, true), 1, 1);
	FadeText clearText(generateText(font, WHITE, "T-spin Triple", CLEARTEXTSIZE, { gamePos.x + GAMEWIDTH + LINEWIDTH * 2, gamePos.y + GAMEHEIGHT / 1.5f }), 0, 2.5f);
	FadeText b2bText(generateText(font, WHITE, "Back-to-Back", CLEARTEXTSIZE, { gamePos.x + GAMEWIDTH + LINEWIDTH * 2, gamePos.y + GAMEHEIGHT / 1.5f + MENUSPACING }), 0, 2.5f);
	FadeText comboText(generateText(font, WHITE, "2X Combo", CLEARTEXTSIZE, { gamePos.x + GAMEWIDTH + LINEWIDTH * 2, gamePos.y + GAMEHEIGHT / 1.5f + MENUSPACING * 2 }), 0, 2.5f);
	FadeText allClearText(generateText(font, WHITE, "All Clear", CLEARTEXTSIZE, { gamePos.x + GAMEWIDTH + LINEWIDTH * 2, gamePos.y + GAMEHEIGHT / 1.5f + MENUSPACING * 3 }), 0, 2.5f);
	return { speedupText, clearText, b2bText, comboText, allClearText };
}
// Get collision bounds for checkboxes
vector<sf::FloatRect> getBoxBounds(vector<sf::RectangleShape>& rects) {
	vector<sf::FloatRect> bounds;
	for (auto& rect : rects)
		bounds.push_back(rect.getGlobalBounds());
	return bounds;
}

// Generate a map between nontext keys that can be used as keybinds and their strings 
map<sf::Keyboard::Key, string> getKeyStrings() {
	map<sf::Keyboard::Key, string> keyStrings;
	keyStrings[sf::Keyboard::Menu] = "Menu";
	keyStrings[sf::Keyboard::LBracket] = "[";
	keyStrings[sf::Keyboard::RBracket] = "]";
	keyStrings[sf::Keyboard::Semicolon] = ";";
	keyStrings[sf::Keyboard::Comma] = ",";
	keyStrings[sf::Keyboard::Period] = ".";
	keyStrings[sf::Keyboard::Quote] = "'";
	keyStrings[sf::Keyboard::Slash] = "/";
	keyStrings[sf::Keyboard::Backslash] = "\\";
	keyStrings[sf::Keyboard::Tilde] = "`";
	keyStrings[sf::Keyboard::Equal] = "=";
	keyStrings[sf::Keyboard::Hyphen] = "-";
	keyStrings[sf::Keyboard::LShift] = "LShift";
	keyStrings[sf::Keyboard::RShift] = "RShift";
	keyStrings[sf::Keyboard::Left] = "Left";
	keyStrings[sf::Keyboard::Up] = "Up";
	keyStrings[sf::Keyboard::Down] = "Down";
	keyStrings[sf::Keyboard::Right] = "Right";
	keyStrings[sf::Keyboard::LControl] = "LControl";
	keyStrings[sf::Keyboard::RControl] = "RControl";
	keyStrings[sf::Keyboard::LAlt] = "LAlt";
	keyStrings[sf::Keyboard::RAlt] = "RAlt";
	keyStrings[sf::Keyboard::Tab] = "Tab";
	keyStrings[sf::Keyboard::Enter] = "Enter";
	keyStrings[sf::Keyboard::Space] = "Space";
	keyStrings[sf::Keyboard::Backspace] = "Backspace";
	keyStrings[sf::Keyboard::PageUp] = "PageUp";
	keyStrings[sf::Keyboard::PageDown] = "PageDown";
	keyStrings[sf::Keyboard::End] = "End";
	keyStrings[sf::Keyboard::Home] = "Home";
	keyStrings[sf::Keyboard::Insert] = "Insert";
	keyStrings[sf::Keyboard::Delete] = "Delete";
	// Procedurally generate alphabet, num, numpad, and Fnum maps for cleaner code
	for (int i = 0; i <= 25; i++)
		keyStrings[sf::Keyboard::Key(i)] = string(1, i + 65);
	for (int i = 26; i <= 35; i++)
		keyStrings[sf::Keyboard::Key(i)] = "Num" + to_string(i - 26);
	for (int i = 75; i <= 84; i++)
		keyStrings[sf::Keyboard::Key(i)] = "Numpad" + to_string(i - 75);
	for (int i = 85; i <= 99; i++)
		keyStrings[sf::Keyboard::Key(i)] = "F" + to_string(i - 84);
	return keyStrings;
}

// Following functions are made for reuseability and requires specific parameters passed in.
// Functionality of the toggles in sandbox mode. 
void toggleGravity(Checkbox* autoFallBox, Screen* screen) {
	if (autoFallBox->getChecked()) {
		autoFallBox->setChecked(false);
		screen->setAutoFall(false);
	}
	else {
		autoFallBox->setChecked(true);
		screen->setAutoFall(true);
	}
}
void decrementGravity(IncrementalBox* gravityBox, Checkbox* autoFallBox, Screen* screen) {
	gravityBox->decrement();
	screen->setGravity(GRAVITYSPEEDS[gravityBox->getCurrentNum() - 1]);
}
void incrementGravity(IncrementalBox* gravityBox, Checkbox* autoFallBox, Screen* screen) {
	gravityBox->increment();
	screen->setGravity(GRAVITYSPEEDS[gravityBox->getCurrentNum() - 1]);
}
void toggleCreative(Checkbox* creativeModeBox, Checkbox* autoFallBox, Screen* screen) {
	if (creativeModeBox->getChecked()) { // If box is already checked, turn off
		creativeModeBox->setChecked(false);
		autoFallBox->setChecked(true);
		screen->setAutoFall(true);
		screen->endCreativeMode();
	}
	else { // If box isn't checked, turn on
		creativeModeBox->setChecked(true);
		autoFallBox->setChecked(false);
		screen->setAutoFall(false);
		screen->startCreativeMode();
	}
}

// Draw all objects in a vector
void drawVector(sf::RenderWindow& window, vector<sf::RectangleShape>& vec) {
	for (sf::RectangleShape& item : vec)
		window.draw(item);
}
void drawVector(sf::RenderWindow& window, vector<sf::Text>& vec) {
	for (sf::Text& item : vec)
		window.draw(item);
}
void drawVector(sf::RenderWindow& window, vector<FadeText>& vec) {
	for (FadeText& animation : vec)
		animation.drawAnimation(window);
}
void drawVector(sf::RenderWindow& window, vector<Animation*>& vec) {
	for (Animation* animation : vec)
		animation->drawAnimation(window);
}

// Function to contain contents of settings menu
void generateSettingsContent(SettingsMenu& gameSettings, sf::Font& font) {
	gameSettings.addTab(font, "Gameplay");
	gameSettings.addTab(font, "Controls");
	gameSettings.addTab(font, "Graphics (WIP)");
	gameSettings.selectTab(0);

	vector<sf::Vector2f> settingPositions;
	vector<OptionSelector*> selectors;
	vector<sf::Vector2f> selectorPositions;
	vector<string> settingText;

	// Set up text, selectors, and positions
	for (float i = 0; i < 11; i++) { // Positions 
		settingPositions.push_back({ SETTINGXPOS, SETTINGYPOS + SETTINGSPACING * i });
		selectorPositions.push_back({ SETTINGXPOS + SELECTORRIGHTSPACING, SETTINGYPOS + SETTINGSPACING * i });
	}
	settingText = { "Starting Speed","Next Piece Count", "Piece Holding", "Ghost Piece", "Auto Shift Delay", "Auto Shift Speed",
		"Piece RNG", "Rotation Style", "Garbage Timer", "Garbage Multiplier", "Garbage RNG"};

	selectors.push_back(new SlidingBar(270, { "Easy", "Normal", "Hard" }, font));
	selectors.push_back(new SlidingBar(270, { "0", "1", "2", "3", "4", "5", "6" }, font));
	selectors.push_back(new OnOffSwitch(font));
	selectors.push_back(new OnOffSwitch(font));
	selectors.push_back(new SlidingBar(270, { "Long", "Normal", "Short", "Instant" }, font));
	selectors.push_back(new SlidingBar(270, { "Slow", "Normal", "Fast", "Instant" }, font));
	selectors.push_back(new SlidingBar(150, { "Random", "7-Bag"}, font));
	selectors.push_back(new SlidingBar(150, { "Classic", "Modern" }, font));
	selectors.push_back(new SlidingBar(270, { "5s", "3s", "1s", "Instant" }, font));
	selectors.push_back(new SlidingBar(270, { "0.5x", "1x", "1.5x" }, font));
	selectors.push_back(new SlidingBar(270, { "Easy", "Normal", "Hard" }, font));

	// Add settings to tab
	for (int i = 0; i < selectors.size(); i++)
		gameSettings[0].addSetting(settingText[i], settingPositions[i], selectors[i], selectorPositions[i], font);

}

// Generate player control keybinds on second setting tab 
void generateKeybinds(vector<KeySet*>& keySets, SettingsTab& tab, map<sf::Keyboard::Key, string>& keyStrings, sf::Font& font) {
	// Generate positions for 21 keybinds
	vector<sf::Vector2f> keybindPositions;
	for (int i = 0; i < keySets.size(); i++)
		for (int j = 0; j < 7; j++)
			keybindPositions.push_back(sf::Vector2f(SETTINGXPOS + SELECTORRIGHTSPACING / 1.5f * (i + 1), SETTINGYPOS + SETTINGSPACING * (j + 1)));
	// Add key recorders to settings tab
	for (sf::Vector2f& vec : keybindPositions)
		tab.addKeybind("", ORIGIN, &keyStrings, vec, font);
	vector<string> controlsText = { "Hard Drop", "Left", "Down", "Right", "SpinCW", "SpinCCW", "Hold", "Solo", "PVP P1", "PVP P2" };
	for (int i = 0; i < 7; i++) 
		tab.addExtraText(generateText(font, WHITE, controlsText[i], MENUTEXTSIZE, sf::Vector2f(SETTINGXPOS, SETTINGYPOS + SETTINGSPACING * (i + 1))));
	for (int i = 7; i < 10; i++)
		tab.addExtraText(generateText(font, WHITE, controlsText[i], MENUTEXTSIZE, sf::Vector2f(SETTINGXPOS + SELECTORRIGHTSPACING / 1.5f * (i - 6), SETTINGYPOS), true, false, true));
}

// Update gameplay settings for a single player
void updateGameplaySettings(vector<int> values, Screen* screen) {
	// Starting speed. Will not take effect in sandbox mode
	screen->setStartingGravity(GRAVITYSPEEDS[values[0]]); 
	screen->setNextPieceCount(values[1]); // Next piece count
	screen->setHoldEnabled(values[2]);	// Piece holding
	screen->setGhostPieceEnabled(values[3]); // Ghost piece
	screen->setBagEnabled(values[6]); // 7-bag
	screen->setSRS(values[7]); // Rotation style
	screen->setGarbageTimer(GARBAGETIMERS[values[8]]); // Garbage timer
	screen->setGarbageMultiplier(GARBAGEMULTIPLIERS[values[9]]); // Garbage multiplier
	screen->setGarbRepeatProbability(GARBAGEREPEATPROBABILITIES[values[10]]); // Garbage repeat probability
}

// Update keybind configurations from setting tab entries
void updateKeybinds(vector<KeySet*>& keySets, vector<KeyRecorder*> newKeybinds) {
	for (int i = 0; i < newKeybinds.size(); i++) // Update keybind controls
		*keySets[i / 7]->getSet()[i % 7] = *newKeybinds[i]->getKey();
}

// Convert settings menu contents to game variables
void updateAllSettings(SettingsMenu& gameSettings, vector<Screen*> screens, vector<KeyDAS*> dasSets, vector<vector<sf::RectangleShape>*> gameRects, vector<KeySet*> keySets) {
	vector<int> gameplaySettings = gameSettings[0].getValues();
	for (int i = 0; i < screens.size(); i++)
		updateGameplaySettings(gameplaySettings, screens[i]);
	for (int i = 0; i < gameRects.size(); i++)
	{
		// [1] / [1] hides the outline when rectangle has zero size
		gameRects[i]->at(1).setSize(sf::Vector2f{ TILESIZE * 4.0f * gameplaySettings[2], TILESIZE * 4.0f * gameplaySettings[2] });
		gameRects[i]->at(2).setSize(sf::Vector2f{ TILESIZE * 4.0f * gameplaySettings[1] / gameplaySettings[1], GAMEHEIGHT / 9.0f * gameplaySettings[1] });
	}
	for (int i = 0; i < dasSets.size(); i++) {
		dasSets[i]->setStartDelay(DASDELAYVALUES[gameplaySettings[4]]); // DAS delay
		dasSets[i]->setHoldDelay(DASSPEEDVALUES[gameplaySettings[5]]); // DAS speed
	}

	updateKeybinds(keySets, gameSettings[1].getKeybinds());
}

// Play a sound starting at a time point
SoundManager* generateSoundManager(){
	SoundManager* soundFX = new SoundManager("files/sound-effects.ogg");
	soundFX->addEffect(MEDIUMBEEP);
	soundFX->addEffect(HIGHBEEP);
	soundFX->addEffect(LIGHTTAP);
	soundFX->addEffect(HIGHHIGHBEEP);
	soundFX->addEffect(LOWBEEP);
	soundFX->addEffect(LOWTHUD);
	soundFX->setVolume(20);
	return soundFX;
}

// Read config file and return a vector of integers
vector<int> readConfigFile(string fileName) {
	ifstream inFile(fileName);
	try{
		if (!inFile.is_open()) 
			throw ConfigError();
		vector<int> configValues;
		string line;
		while (getline(inFile, line))
			configValues.push_back(stoi(line));
		return configValues;
	}
	catch (ConfigError err) {
		cout << "File does not exist. Creating default file.\n";
		return DEFAULTSETTINGS;
	}
	catch (exception err) {
		cout << "File reading error. Restoring to defaults.\n";
		return DEFAULTSETTINGS;
	}
	inFile.close();
}

// Write setting menu contents to a file
void writeConfigFile(string fileName, vector<int> values) {
	ofstream outFile(fileName);
	if (!outFile.is_open()) {
		cout << "Failed to write file";
		throw exception();
	}
	for (int val : values)
		outFile << to_string(val) << endl;
	outFile.close();
}

int main(){
	srand(time(NULL));
#pragma region SFML Setup
	// Set SFML objects
	sf::Font font;

	if (!font.loadFromFile("files/font.ttf"))
		return -1;
	sf::Texture texture;
	if (!texture.loadFromFile("files/tile_hidden.png"))
		return -1;
	SoundManager* soundFX = generateSoundManager();
	sf::Music bgm;
	if (!bgm.openFromFile("files/tetris-theme.ogg"))
		return -1;
	bgm.setVolume(5);
	bgm.setLoop(true);

	sf::ContextSettings windowSettings;
	windowSettings.antialiasingLevel = 8;
	sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Tetris", sf::Style::Close | sf::Style::Titlebar, windowSettings);
	window.setFramerateLimit(FPS);
	window.setKeyRepeatEnabled(false);
#pragma endregion

#pragma region Basic Assets
	// Title screen sprites
	sf::Text titleText(generateText(font, WHITE, "TETRIS", 150, TITLETEXTPOS, true, false, true));
	sf::CircleShape* cursor = new sf::CircleShape(15.f, 3); // Triangle shaped cursor
	cursor->rotate(90.f);
	vector<string> menuText = { "Classic Mode", "Sandbox Mode", "PVP Mode", "Settings", "Quit" };
	ClickableMenu gameMenu(font, WHITE, menuText, MENUTEXTSIZE, MENUPOS, MENUSPACING, *cursor);

	// Game sprites in a vector and constructed in a separate method to keep main clean
	vector<sf::RectangleShape> gameScreenRectangles = getGameRects(GAMEPOS);
	vector<sf::FloatRect> gameScreenBounds = getGameBounds(gameScreenRectangles);
	vector<sf::Text> gameText = getGameText(gameScreenBounds, font);
	vector<sf::RectangleShape> lines = getLines(gameScreenBounds[0]);
	sf::Text linesClearedText = generateText(font, WHITE, "Lines: 0", 25, { GAMEXPOS + GAMEWIDTH + 150, GAMEYPOS });

	// Get player two assets
	vector<sf::RectangleShape> gameScreenRectanglesP2 = getGameRects(GAMEPOSP2);
	vector<sf::FloatRect> gameScreenBoundsP2 = getGameBounds(gameScreenRectanglesP2);
	vector<sf::RectangleShape> linesP2 = getLines(gameScreenBoundsP2[0]);
	vector<sf::Text> gameTextP2 = getGameText(gameScreenBoundsP2, font);
	gameTextP2[2].setString("PVP Mode"); // This will be the title text used in pvp mode. Hide the other title text
	gameTextP2[2].setPosition(WIDTH, gameTextP2[2].getPosition().y);


#pragma endregion

#pragma region Extra Assets
	// Sandbox mode exclusive sprites
	vector<sf::Text> sandboxText = getSandboxText(font);
	Checkbox* autoFallBox = new Checkbox(TILESIZE, SANDBOXMENUPOS.x + 180, SANDBOXMENUPOS.y, true, font);
	IncrementalBox* gravityBox = new IncrementalBox(TILESIZE, SANDBOXMENUPOS.x + 180, SANDBOXMENUPOS.y + MENUSPACING, 1, GRAVITYTIERCOUNT, font);
	Checkbox* creativeModeBox = new Checkbox(TILESIZE, SANDBOXMENUPOS.x + 180, SANDBOXMENUPOS.y + MENUSPACING * 2, false, font);
	Checkbox* resetBox = new Checkbox(TILESIZE, SANDBOXMENUPOS.x + 180, SANDBOXMENUPOS.y + MENUSPACING * 3, false, font);
	Checkbox* quitBox = new Checkbox(TILESIZE, SANDBOXMENUPOS.x + 180, SANDBOXMENUPOS.y + MENUSPACING * 4, false, font);
	vector<Checkbox*> sandboxes = { autoFallBox, gravityBox, creativeModeBox, resetBox, quitBox };
	bool creativeModeOn = false;

	// Pause screen sprites
	vector<string> pauseMenuText = { "Continue", "Restart", "Quit" };
	pauseScreen pause(GAMEPOS, pauseMenuText, font);

	// Text for loss screen
	vector<sf::Text> lossText = getLossText(font);

	// Initiate animation classes
	vector<FadeText> clearAnimations = getFadeText(GAMEPOS, font);
	vector<FadeText> clearAnimationsP2 = getFadeText(GAMEPOSP2, font);
#pragma endregion

#pragma region Controller Classes
	// Initialize controls with default keybinds
	KeySet* playerSoloKeys = new KeySet(LEFT, RIGHT, UP, DOWN, SPINCW, SPINCCW, HOLD);
	KeySet* player1Keys = new KeySet(LEFT1, RIGHT1, UP1, DOWN1, SPINCW1, SPINCCW1, HOLD1);
	KeySet* player2Keys = new KeySet(LEFT2, RIGHT2, UP2, DOWN2, SPINCW2, SPINCCW2, HOLD2);
	vector<KeySet*> keySets = { playerSoloKeys, player1Keys, player2Keys };

	// Initiate DAS keys
	KeyDAS* playerSoloDAS = new KeyDAS(DASDELAY, DASSPEED, playerSoloKeys);
	KeyDAS* player1DAS = new KeyDAS(DASDELAY, DASSPEED, player1Keys);
	KeyDAS* player2DAS = new KeyDAS(DASDELAY, DASSPEED, player2Keys);
#pragma endregion

	// Initiate piece rng
	pieceBag bag;
	
	// Set up game screen
	Screen* screen = new Screen(window, gameScreenBounds, texture, &clearAnimations, &bag, soundFX);
	Screen* screenP2 = new Screen(window, gameScreenBoundsP2, texture, &clearAnimationsP2, &bag, soundFX);
	int currentScreen = MAINMENU;

	// Set up settings sprites
	SettingsMenu gameSettings(soundFX);
	generateSettingsContent(gameSettings, font);

	vector<ClickableButton> clickableButtons; // { Reset, Discard, Save }
	clickableButtons.push_back(ClickableButton({ 230, 40 }, { 150, 725 }, font, "Reset Defaults", BUTTONTEXTSIZE, RED));
	clickableButtons.push_back(ClickableButton({ 230, 40 }, { 400, 725 }, font, "Discard & Quit", BUTTONTEXTSIZE, RED));
	clickableButtons.push_back(ClickableButton({ 230, 40 }, { 650, 725 }, font, "Save & Quit", BUTTONTEXTSIZE, RED));
	
	// Key recorders need a pointer to keyStrings stored in main
	map<sf::Keyboard::Key, string> keyStrings = getKeyStrings();
	generateKeybinds(keySets, gameSettings[1], keyStrings, font);

	// Read settings file
	vector<int> configValues = readConfigFile(CONFIGFILENAME); // Stores a copy of file contents
	gameSettings.applyConfig(configValues);
	// Rewrite settings file to fix invalid configurations from previous two lines.
	writeConfigFile(CONFIGFILENAME, gameSettings.getValues());

	updateAllSettings(gameSettings, { screen, screenP2 },
		{ playerSoloDAS, player1DAS, player2DAS }, { &gameScreenRectangles, &gameScreenRectanglesP2 }, keySets);
	
	// Game loop
	while (window.isOpen())
	{
		// Manage audio across all screens
		soundFX->checkTimers(); 

		// Run on main menu
		if (currentScreen == MAINMENU) {
			window.clear(BLUE);
			window.draw(titleText);
			window.draw(gameMenu);

			bool modeSelected = false;
			
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
						soundFX->play(LIGHTTAP);
						gameMenu.moveDown();
					}
					else if (event.key.code == sf::Keyboard::Up) {
						soundFX->play(LIGHTTAP);
						gameMenu.moveUp();
					}
					else if (event.key.code == sf::Keyboard::Z)
						modeSelected = true;
					break;
				}
				case sf::Event::MouseMoved: {
					if (gameMenu.updateMouseMove(event.mouseMove.x, event.mouseMove.y))
						soundFX->play(LIGHTTAP);
					break;
				}
				case sf::Event::MouseButtonPressed: {
					if (gameMenu.updateMouseClick(event.mouseButton.x, event.mouseButton.y)) 
						modeSelected = true;
					
					break;
				}
				case sf::Event::MouseButtonReleased: {
					break;
				}
				case sf::Event::TextEntered: {
					break;
				}
				default:
					break;
				}
			}

			// Select menu option if modeSelected is true
			if (modeSelected) {
				soundFX->play(HIGHBEEP);
				switch (gameMenu.getCursorPos())
				{
				case 0: // Classic mode
					currentScreen = CLASSIC;
					screen->setGameMode(CLASSIC);
					gameText[2].setString("Classic Mode");
					screen->setAutoFall(true);
					screen->endCreativeMode();
					bag.resetQueue();
					screen->resetBoard();
					bgm.play();
					break;
				case 1: // Sandbox mode
					currentScreen = SANDBOX;
					gameText[2].setString("Sandbox Mode");
					screen->setGameMode(SANDBOX);
					// Reset sandbox settings
					autoFallBox->setChecked(true);
					gravityBox->setValue(gravityBox->getMin());
					creativeModeBox->setChecked(false);
					screen->setAutoFall(true);
					screen->endCreativeMode();
					bag.resetQueue();
					screen->resetBoard();
					bgm.play();
					break;
				case 2: // PVP mode
					currentScreen = MULTIPLAYER;
					window.setSize({ WIDTH * 2, HEIGHT });
					window.setView(sf::View(sf::FloatRect(0, 0, WIDTH * 2, HEIGHT)));
					window.setPosition({ 100, 100 });
					screen->setGameMode(MULTIPLAYER);
					gameText[2].setString("");
					screen->setAutoFall(true);
					screen->endCreativeMode();
					bag.resetQueue();
					screen->resetBoard();

					screenP2->setGameMode(MULTIPLAYER);
					screenP2->resetBoard();
					bgm.play();
					break;
				case 3: // Settings
					gameSettings.selectTab(0);
					currentScreen = SETTINGSCREEN;
					break;
				case 4: // Quit
					window.close();
					break;
				default:
					break;
				}
				gameMenu.resetCursorPos();
			}
		}
		// Run on classic mode
		else if (currentScreen == CLASSIC) {
			window.clear(BLUE);
			drawVector(window, gameScreenRectangles);
			drawVector(window, lines);
			screen->drawScreen();
			window.draw(gameScreenRectangles.back()); // Redraw last rectangle
			drawVector(window, gameText);
			drawVector(window, clearAnimations);

			// This is only shown in classic mode
			linesClearedText.setString("Lines: " + to_string(screen->getLinesCleared()));
			window.draw(linesClearedText);
			
			if (screen->getPaused() && !screen->getGameOver())
				window.draw(pause);
			bool modeSelected = false; // For pause screen

			// Manage audio
			soundFX->checkTimers();

			// In-game timer events
			screen->doTimeStuff();

			// Check for game over
			if (screen->getGameOver()) {
				bgm.stop();
				if (screen->isDeathAnimationOver()) {
					currentScreen = LOSESCREEN;
					lossText[0] = generateText(font, WHITE, "YOU LOST", GAMETEXTSIZE * 4, { WIDTH / 2, GAMEYPOS }, true, false, true);
				}
			}

			// Handles movement with auto-repeat (DAS)
			playerSoloDAS->checkKeyPress(screen);

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
					if (screen->getPaused()) { // Pause screen
						if (event.key.code == sf::Keyboard::Down) {
							soundFX->play(LIGHTTAP);
							pause.getMenu().moveDown();
						}
						else if (event.key.code == sf::Keyboard::Up) {
							soundFX->play(LIGHTTAP);
							pause.getMenu().moveUp();
						}
						else if (event.key.code == sf::Keyboard::Z)
							modeSelected = true;
					}
					else { // Game controls
						if (event.key.code == playerSoloKeys->getUp())
							screen->movePiece(3);
						else if (event.key.code == playerSoloKeys->getSpinCCW())
							screen->spinPiece(false);
						else if (event.key.code == playerSoloKeys->getSpinCW())
							screen->spinPiece(true);
						else if (event.key.code == playerSoloKeys->getHold())
							screen->holdPiece();
					}
					if (event.key.code == sf::Keyboard::Escape) {
						screen->doPauseResume();
						pause.getMenu().resetCursorPos();
						soundFX->play(MEDIUMBEEP);
					}
					break;
				}
				case sf::Event::KeyReleased: {
					playerSoloDAS->releaseKey(event.key.code);
				}
				case sf::Event::MouseButtonPressed: {
					if (screen->getPaused() && pause.getMenu().updateMouseClick(event.mouseButton.x, event.mouseButton.y))
						modeSelected = true;
					break;
				}
				case sf::Event::MouseMoved: {
					if (screen->getPaused() && pause.getMenu().updateMouseMove(event.mouseMove.x, event.mouseMove.y))
						soundFX->play(LIGHTTAP);
					break;
				}
				default:
					break;
				}
			}
			// Select menu option if modeSelected is true
			if (modeSelected) {
				switch (pause.getMenu().getCursorPos())
				{
				case 0: // Continue
					screen->doPauseResume();
					soundFX->play(MEDIUMBEEP);
					break;
				case 1: // Restart
					bag.resetQueue();
					screen->resetBoard();
					soundFX->play(HIGHBEEP);
					break;
				case 2: // Quit
					bgm.stop();
					currentScreen = MAINMENU;
					soundFX->play(HIGHBEEP);
					break;
				default:
					break;
				}
				modeSelected = false;
				pause.getMenu().resetCursorPos();
			}
		}
		// Run on sandbox mode
		else if (currentScreen == SANDBOX) {
			window.clear(BLUE);
			for (int i = 0; i < gameScreenRectangles.size() - 1; i++) 
				window.draw(gameScreenRectangles[i]);
			drawVector(window, gameScreenRectangles);
			drawVector(window, lines);
			screen->drawScreen();
			window.draw(gameScreenRectangles.back()); // Redraw last rectangle
			drawVector(window, gameText);
			drawVector(window, sandboxText);
			for (auto box : sandboxes)
				window.draw(*box);
			
			// Manage audio
			soundFX->checkTimers();

			// In-game timer events
			screen->doTimeStuff();
			// Handles movement with auto-repeat (DAS)
			playerSoloDAS->checkKeyPress(screen);

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
					if (event.key.code == playerSoloKeys->getUp())
						screen->movePiece(3);
					else if (event.key.code == playerSoloKeys->getSpinCCW())
						screen->spinPiece(false);
					else if (event.key.code == playerSoloKeys->getSpinCW())
						screen->spinPiece(true);
					else if (event.key.code == playerSoloKeys->getHold())
						screen->holdPiece();
					else if (event.key.code == sf::Keyboard::Num1)
						screen->spawnPiece(0);
					else if (event.key.code == sf::Keyboard::Num2)
						screen->spawnPiece(1);
					else if (event.key.code == sf::Keyboard::Num3)
						screen->spawnPiece(2);
					else if (event.key.code == sf::Keyboard::Num4)
						screen->spawnPiece(3);
					else if (event.key.code == sf::Keyboard::Num5)
						screen->spawnPiece(4);
					else if (event.key.code == sf::Keyboard::Num6)
						screen->spawnPiece(5);
					else if (event.key.code == sf::Keyboard::Num7)
						screen->spawnPiece(6);
						// Hot keys for sandbox controls
					else if (event.key.code == sf::Keyboard::Q || event.key.code == sf::Keyboard::Escape)
						toggleGravity(autoFallBox, screen);
					else if (event.key.code == sf::Keyboard::W)
						decrementGravity(gravityBox, autoFallBox, screen);
					else if (event.key.code == sf::Keyboard::S)
						incrementGravity(gravityBox, autoFallBox, screen);
					else if (event.key.code == sf::Keyboard::E)
						toggleCreative(creativeModeBox, autoFallBox, screen);
					else if (event.key.code == sf::Keyboard::R) {
						bag.resetQueue();
						screen->resetBoard();
					}
					else if (event.key.code == sf::Keyboard::T) {
						bgm.stop();
						currentScreen = MAINMENU;
					}
					else if (event.key.code == sf::Keyboard::G) 
						screen->receiveGarbage(1);
					else if (event.key.code == sf::Keyboard::H) 
						screen->receiveGarbage(4);
					break;
				}
				case sf::Event::KeyReleased: {
					playerSoloDAS->releaseKey(event.key.code);
				}
				case sf::Event::MouseButtonPressed: {
					sf::Vector2f clickPos(event.mouseButton.x, event.mouseButton.y);
					if (event.mouseButton.button == sf::Mouse::Left) {
						if (autoFallBox->getBounds().contains(clickPos))  // Turn off gravity
							toggleGravity(autoFallBox, screen);
						else if (gravityBox->getLeftBound().contains(clickPos))  // Speed arrows
							decrementGravity(gravityBox, autoFallBox, screen);
						else if (gravityBox->getRightBound().contains(clickPos))
							incrementGravity(gravityBox, autoFallBox, screen);
						else if (creativeModeBox->getBounds().contains(clickPos))
							toggleCreative(creativeModeBox, autoFallBox, screen);
						else if (gameScreenBounds[0].contains(clickPos))  // Creative mode click
							screen->clickBlock(clickPos); // Will do nothing if creative mode isn't on
						else if (resetBox->getBounds().contains(clickPos)) {
							bag.resetQueue();
							screen->resetBoard();
						}
						else if (quitBox->getBounds().contains(clickPos)) { // Return to menu
							bgm.stop();
							currentScreen = MAINMENU;
						}
					}
					else if (event.mouseButton.button == sf::Mouse::Right && gameScreenBounds[0].contains(clickPos)) {
						screen->clickRow(clickPos); // Creative mode right click
					}
					break;
				}
				case sf::Event::MouseMoved: { // Shows a transparent X when hovering over unchecked boxes
					for (Checkbox* box : sandboxes)
						box->setHovering(box->getBounds().contains((float)event.mouseMove.x, (float)event.mouseMove.y));
				}
				default:
					break;
				}
			}
		}
		// Run on PVP mode
		else if (currentScreen == MULTIPLAYER) {
			window.clear(BLUE);
			drawVector(window, gameScreenRectangles);
			drawVector(window, lines);
			screen->drawScreen();
			window.draw(gameScreenRectangles.back()); // Redraw last rectangle
			drawVector(window, gameText);
			drawVector(window, clearAnimations);

			drawVector(window, gameScreenRectanglesP2);
			drawVector(window, linesP2);
			screenP2->drawScreen();
			window.draw(gameScreenRectanglesP2.back());
			drawVector(window, gameTextP2);
			drawVector(window, clearAnimationsP2);

			if (screen->getPaused() && !screen->getGameOver() && !screenP2->getGameOver())
				window.draw(pause);
			bool modeSelected = false; // For pause screen

			// Manage audio
			soundFX->checkTimers();

			// In-game timer events
			screen->doTimeStuff();
			screenP2->doTimeStuff();
			// Process garbage exchange
			screen->receiveGarbage(screenP2->getOutGarbage());
			screenP2->receiveGarbage(screen->getOutGarbage());

			// Check for game over
			if (screen->getGameOver()) {
				bgm.stop();
				screenP2->pauseGame();
				if (screen->isDeathAnimationOver()) {
					window.setSize({ WIDTH, HEIGHT });
					window.setView(sf::View(sf::FloatRect(0, 0, WIDTH, HEIGHT)));
					currentScreen = LOSESCREEN;
					lossText[0] = generateText(font, WHITE, "PLAYER 2 WINS!", GAMETEXTSIZE * 4, { WIDTH / 2, GAMEYPOS }, true, false, true);
					if (screenP2->getGameOver() && screenP2->isDeathAnimationOver()) // Rare event if both players lose at the same time
						lossText[0] = generateText(font, WHITE, "DRAW!", GAMETEXTSIZE * 4, { WIDTH / 2, GAMEYPOS }, true, false, true);
				}
			}
			else if (screenP2->getGameOver()) {
				bgm.stop();
				screen->pauseGame();
				if (screenP2->isDeathAnimationOver()) {
					window.setSize({ WIDTH, HEIGHT });
					window.setView(sf::View(sf::FloatRect(0, 0, WIDTH, HEIGHT)));
					currentScreen = LOSESCREEN;
					lossText[0] = generateText(font, WHITE, "PLAYER 1 WINS!", GAMETEXTSIZE * 4, { WIDTH / 2, GAMEYPOS }, true, false, true);
				}
			}

			// Handles movement with auto-shift (DAS)
			player1DAS->checkKeyPress(screen);
			player2DAS->checkKeyPress(screenP2);

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
					if (screen->getPaused()) { // Pause screen
						if (event.key.code == sf::Keyboard::Down) {
							soundFX->play(LIGHTTAP);
							pause.getMenu().moveDown();
						}
						else if (event.key.code == sf::Keyboard::Up) {
							soundFX->play(LIGHTTAP);
							pause.getMenu().moveUp();
						}
						else if (event.key.code == sf::Keyboard::Z)
							modeSelected = true;
					}
					else {
						if (event.key.code == player1Keys->getUp())
							screen->movePiece(3);
						else if (event.key.code == player1Keys->getSpinCCW())
							screen->spinPiece(false);
						else if (event.key.code == player1Keys->getSpinCW())
							screen->spinPiece(true);
						else if (event.key.code == player1Keys->getHold())
							screen->holdPiece();
						else if (event.key.code == player2Keys->getUp())
							screenP2->movePiece(3);
						else if (event.key.code == player2Keys->getSpinCCW())
							screenP2->spinPiece(false);
						else if (event.key.code == player2Keys->getSpinCW())
							screenP2->spinPiece(true);
						else if (event.key.code == player2Keys->getHold())
							screenP2->holdPiece();
					}
					if (event.key.code == sf::Keyboard::Escape) {
						// Pause game and show menu. Disable during death animation for bug fix
						if (!screen->getGameOver() && !screenP2->getGameOver()) {
							soundFX->play(MEDIUMBEEP);
							screen->doPauseResume();
							screenP2->doPauseResume();
							pause.getMenu().resetCursorPos();
						}
					}
					break;
				}
				case sf::Event::KeyReleased: {
					player1DAS->releaseKey(event.key.code);
					player2DAS->releaseKey(event.key.code);
				}
				case sf::Event::MouseButtonPressed: {
					if (screen->getPaused() && pause.getMenu().updateMouseClick(event.mouseButton.x, event.mouseButton.y))
						modeSelected = true;
					break;
				}
				case sf::Event::MouseMoved: {
					if (screen->getPaused() && pause.getMenu().updateMouseMove(event.mouseMove.x, event.mouseMove.y))
						soundFX->play(LIGHTTAP);
					break;
				}
				default:
					break;
				}
			}

			// Select menu option if modeSelected is true
			if (modeSelected) {
				switch (pause.getMenu().getCursorPos())
				{
				case 0: // Continue
					screen->doPauseResume();
					screenP2->doPauseResume();
					soundFX->play(MEDIUMBEEP);
					break;
				case 1: // Restart
					bag.resetQueue();
					screen->resetBoard();
					screenP2->resetBoard();
					soundFX->play(HIGHBEEP);
					break;
				case 2: // Quit
					window.setSize({ WIDTH, HEIGHT });
					window.setView(sf::View(sf::FloatRect(0, 0, WIDTH, HEIGHT)));
					bgm.stop();
					currentScreen = MAINMENU;
					soundFX->play(HIGHBEEP);
					break;
				default:
					break;
				}
				modeSelected = false;
				pause.getMenu().resetCursorPos();
			}
		}
		else if (currentScreen == LOSESCREEN) {
			window.clear(BLACK);
			drawVector(window, lossText);

			// Manage audio
			soundFX->checkTimers();

			sf::Event event;
			while (window.pollEvent(event)) {
				switch (event.type)
				{
				case sf::Event::Closed: 
					window.close();
					break; 
				case sf::Event::KeyPressed: 
					currentScreen = MAINMENU;
					soundFX->play(MEDIUMBEEP);
					break;
				case sf::Event::MouseButtonPressed: 
					currentScreen = MAINMENU;
					soundFX->play(MEDIUMBEEP);
					break;
				default:
					break;
				}
			}
		}
		else if (currentScreen == SETTINGSCREEN) {
			window.clear(BLUE);
			window.draw(gameSettings);
			for (ClickableButton& button : clickableButtons)
				window.draw(button);

			// Manage audio
			soundFX->checkTimers();

			sf::Event event;
			while (window.pollEvent(event)) {
				switch (event.type)
				{
				case sf::Event::Closed:
					window.close();
					break;
				case sf::Event::KeyPressed:
					gameSettings[1].readKeys(event.key.code);
					break;
				case sf::Event::MouseMoved:
					gameSettings.processMouseMove(event.mouseMove.x, event.mouseMove.y);
					break;
				case sf::Event::MouseButtonPressed:
					gameSettings.processMouseClick(event.mouseButton.x, event.mouseButton.y);
					// Restore to default settings
					if (clickableButtons[0].checkClick(event.mouseButton.x, event.mouseButton.y)) {
						gameSettings.applyConfig(DEFAULTSETTINGS);
						soundFX->play(MEDIUMBEEP);
					}
					// Discard & quit
					else if (clickableButtons[1].checkClick(event.mouseButton.x, event.mouseButton.y)) {
						gameSettings.applyConfig(configValues);
						currentScreen = MAINMENU;
						soundFX->play(HIGHBEEP);
					}
					// Save & quit
					else if (clickableButtons[2].checkClick(event.mouseButton.x, event.mouseButton.y)) {
						// Update gameplay settings for two screens and three DAS sets
						updateAllSettings(gameSettings, { screen, screenP2 },
							{ playerSoloDAS, player1DAS, player2DAS }, { &gameScreenRectangles, &gameScreenRectanglesP2 }, keySets);
						writeConfigFile(CONFIGFILENAME, gameSettings.getValues());
						// Refresh file contents copy without having to open the file again
						configValues = gameSettings.getValues(); 

						currentScreen = MAINMENU;
						soundFX->play(HIGHBEEP);
					}
					break;
				case sf::Event::MouseButtonReleased:
					gameSettings.processMouseRelease();
					break;
				default:
					break;
				}
			}
		}
		window.display();
	}

	// Cleanup
	for (Checkbox* box : sandboxes)
		delete box;
	delete playerSoloKeys;
	delete player1Keys;
	delete player2Keys;
	delete playerSoloDAS;
	delete player1DAS;
	delete player2DAS;
	delete screen;
	delete screenP2;
	delete soundFX;
	delete cursor;
	return 0;
}
