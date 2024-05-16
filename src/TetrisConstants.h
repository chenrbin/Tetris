#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include <map>
using namespace std;
namespace TetrisVariables {
	// Todo: online, Full screen compatibility, audio settings
	// Known bugs: 
	// DAS does not pause (Not Fixing)
	
	// Set constant variables

	// Item sizes and spacings
	const int TILESIZE = 32, LEFTMARGIN = 180, GAMETEXTSIZE = 20, MENUTEXTSIZE = 24, CLEARTEXTSIZE = 30;
	const int LINEWIDTH = 2, TOPROWPIXELS = 10, MENUSPACING = 40;
	const float SCALE = 0.8f; // Smaller display for hold and next pieces

	// Screen dimensions
	const int WIDTH = 800, HEIGHT = 800, NUMROWS = 20, NUMCOLS = 10;
	const int REALNUMROWS = NUMROWS + 2; // Actual number of rows. NUMROWS is the rows visible.
	const int GAMEWIDTH = TILESIZE * NUMCOLS, GAMEHEIGHT = TILESIZE * NUMROWS;

	// Game mechanic related variables
	const int FPS = 60; // Frame limit of the game
	const float LOCKDELAY = 0.5f; // Delay before a piece sets in seconds
	const float SUPERLOCKDELAY = 3; // Lock delay to prevent infinites
	const int NEXTPIECECOUNT = 6; // Number of next pieces visible. Will crash if above 7.
	const int GRAVITYTIERLINES[] = { 0, 40, 80, 100, 120, 160, 220 }; // Lines required to set speed of the same index
	const float GRAVITYSPEEDS[] = { 1, 0.6f, 0.25f, 0.1f, 0.05f, 0.02f, 0.01f }; // Time needed for a piece to fall once
	const float DEFAULTGRAVITY = GRAVITYSPEEDS[0]; // Time between gravity movements in seconds
	const int GRAVITYTIERCOUNT = 7;
	const float BASEGARBAGETIMER = 3; 
	const float DEFAULTGARBAGEPROBABILITY = 0.6f; // Chance for a guaranteed repeat garbage column
	const float DEFAULTGARBAGEMULTIPLIER = 1;
	vector<float> GARBAGETIMERS = { 5, 3, 1, 0 };
	vector<float> GARBAGEMULTIPLIERS = { 0.5, 1, 1.5 };
	vector<float> GARBAGEREPEATPROBABILITIES = { 0.9, 0.6, 0.2 };

	const float DASDELAY = 170, DASSPEED = 50;
	vector<float> DASDELAYVALUES{ 340, 170, 50, 0 };
	vector<float> DASSPEEDVALUES{ 100, 50, 25, 0 };

	// Rectangle positions
	const float MENUXPOS = WIDTH / 1.7f, MENUYPOS = HEIGHT / 2 - 40;
	const float GAMEXPOS = LEFTMARGIN, GAMEYPOS = ((HEIGHT - GAMEHEIGHT) / 2);
	const sf::Vector2f TITLETEXTPOS(WIDTH / 2, 100);
	const sf::Vector2f MENUPOS(MENUXPOS, MENUYPOS);
	const sf::Vector2f GAMEPOS(GAMEXPOS, GAMEYPOS);
	const sf::Vector2f GAMEPOSP2(GAMEXPOS + WIDTH, GAMEYPOS);
	const sf::Vector2f SANDBOXMENUPOS(GAMEXPOS + GAMEWIDTH + LINEWIDTH * 2, GAMEYPOS + GAMEHEIGHT / 1.5f);
	const sf::Vector2f ORIGIN(0, 0);
	// Game screen state codes
	const int MAINMENU = 1, CLASSIC = 2, SANDBOX = 3, MULTIPLAYER = 4, LOSESCREEN = 5, SETTINGSCREEN = 6;
	// Set color constants for easy use and passing to functions
	const sf::Color WHITE(255, 255, 255);
	const sf::Color BLACK(0, 0, 0);
	const sf::Color GRAY(128, 128, 128);
	const sf::Color RED(255, 0, 0); 
	const sf::Color ORANGE(255, 165, 0);
	const sf::Color YELLOW(255, 255, 0);
	const sf::Color GREEN(0, 255, 0);
	const sf::Color BLUE(0, 0, 255);
	const sf::Color CYAN(0, 255, 255);
	const sf::Color VIOLET(148, 0, 211);
	const sf::Color SEETHROUGH(255, 255, 255, 100);
	const sf::Color HOVERCHECKBOX(255, 255, 255, 150);
	const int PREVIEWTRANSPARENCY = 120;
	const sf::Color INVISIBLE(255, 255, 255, 0);

	const sf::Color TPIECECOLOR = VIOLET; // Defining T piece color for detecting T-spins
	
	// Default keybinds
	
	// Single player
	sf::Keyboard::Key UP = sf::Keyboard::Up;
	sf::Keyboard::Key LEFT = sf::Keyboard::Left;
	sf::Keyboard::Key DOWN = sf::Keyboard::Down;
	sf::Keyboard::Key RIGHT = sf::Keyboard::Right;
	sf::Keyboard::Key SPINCW = sf::Keyboard::X;
	sf::Keyboard::Key SPINCCW = sf::Keyboard::Z;
	sf::Keyboard::Key HOLD = sf::Keyboard::LShift;

	// PvP player 1
	sf::Keyboard::Key UP1 = sf::Keyboard::W;
	sf::Keyboard::Key LEFT1 = sf::Keyboard::A;
	sf::Keyboard::Key DOWN1 = sf::Keyboard::S;
	sf::Keyboard::Key RIGHT1 = sf::Keyboard::D;
	sf::Keyboard::Key SPINCW1 = sf::Keyboard::V;
	sf::Keyboard::Key SPINCCW1 = sf::Keyboard::C;
	sf::Keyboard::Key HOLD1 = sf::Keyboard::LShift;

	// Pvp player 2
	sf::Keyboard::Key UP2 = sf::Keyboard::Up;
	sf::Keyboard::Key LEFT2 = sf::Keyboard::Left;
	sf::Keyboard::Key DOWN2 = sf::Keyboard::Down;
	sf::Keyboard::Key RIGHT2 = sf::Keyboard::Right;
	sf::Keyboard::Key SPINCW2 = sf::Keyboard::Period;
	sf::Keyboard::Key SPINCCW2 = sf::Keyboard::Comma;
	sf::Keyboard::Key HOLD2 = sf::Keyboard::RShift;

	const vector<int> DEFAULTSETTINGS{ 1, 6, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	UP, LEFT, DOWN, RIGHT, SPINCW, SPINCCW, HOLD,
	UP1, LEFT1, DOWN1, RIGHT1, SPINCW1, SPINCCW1, HOLD1,
	UP2, LEFT2, DOWN2, RIGHT2, SPINCW2, SPINCCW2, HOLD2 };
	const string CONFIGFILEPATH = "assets/config.cfg";
	const string SOUNDFXFILEPATH = "assets/sound-effects.ogg";
	const string FONTFILEPATH = "assets/font.ttf";
	const string BLOCKFILEPATH = "assets/tile_hidden.png";
	const string BGMFILEPATH = "assets/tetris-theme.ogg";
	
	// Audio timestamps for tetris-effects.ogg
	const float CLIPDURATION = 0.5;
	const float LIGHTTAP = 3.655, MEDIUMBEEP = 1.8, HIGHBEEP = 3.628;
	const float HIGHHIGHBEEP = 5.4, LOWBEEP = 7.2, LOWTHUD = 9;


	// Constants for sprite classes

	// Sliding Bar
	const int BARHEIGHT = 6;
	const int NODEWIDTH = 4, NODEHEIGHT = 15;
	const int BAR_CURSOR_RADIUS = 10, BAR_CURSOR_GROWTH = 2;

	// Settings Tab
	const int TABTOP = 0, TABHEIGHT = 50, TABHEIGHTGROWTH = 10;
	// Tab items
	const int SETTINGXPOS = 50, SETTINGYPOS = 100, SETTINGSPACING = 50;
	const int SELECTORRIGHTSPACING = 300, SELECTORDOWNSPACING = 40;
	// Clickable button
	const int BUTTONTEXTSIZE = 22;
	// On-Off Switch
	const int SWITCHWIDTH = 90, SWITCHHEIGHT = 30;
	// BulletListSelector
	const int BULLETNODERADIUS = 10, BULLETCURSORRADIUS = 6;

	void initializeKeyStrings(map<sf::Keyboard::Key, string>& keyStrings){
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
	}

	// Print stuff for debug
	template <typename T>
	void print(T var) {
		cout << "Value is: " << var << " ";
	}
	template <typename T>
	void printLine(T var) {
		cout << "Value is: " << var << endl;
	}
}
