#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <map>
using namespace std;
namespace TetrisVariables {
	// Todo: setting file, BGM, online, default settings in online matches
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
	const int GRAVITYTIERLINES[] = { 0, 20, 40, 70, 100, 140, 180 };
	const float GRAVITYSPEEDS[] = { 1, 0.5f, 0.25f, 0.1f, 0.05f, 0.02f, 0.01f };
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
	sf::Keyboard::Key LEFT = sf::Keyboard::Left;
	sf::Keyboard::Key RIGHT = sf::Keyboard::Right;
	sf::Keyboard::Key UP = sf::Keyboard::Up;
	sf::Keyboard::Key DOWN = sf::Keyboard::Down;
	sf::Keyboard::Key SPINCW = sf::Keyboard::X;
	sf::Keyboard::Key SPINCCW = sf::Keyboard::Z;
	sf::Keyboard::Key HOLD = sf::Keyboard::LShift;

	// PvP player 1
	sf::Keyboard::Key LEFT1 = sf::Keyboard::A;
	sf::Keyboard::Key RIGHT1 = sf::Keyboard::D;
	sf::Keyboard::Key UP1 = sf::Keyboard::W;
	sf::Keyboard::Key DOWN1 = sf::Keyboard::S;
	sf::Keyboard::Key SPINCW1 = sf::Keyboard::V;
	sf::Keyboard::Key SPINCCW1 = sf::Keyboard::C;
	sf::Keyboard::Key HOLD1 = sf::Keyboard::LShift;

	// Pvp player 2
	sf::Keyboard::Key LEFT2 = sf::Keyboard::Left;
	sf::Keyboard::Key RIGHT2 = sf::Keyboard::Right;
	sf::Keyboard::Key UP2 = sf::Keyboard::Up;
	sf::Keyboard::Key DOWN2 = sf::Keyboard::Down;
	sf::Keyboard::Key SPINCW2 = sf::Keyboard::Period;
	sf::Keyboard::Key SPINCCW2 = sf::Keyboard::Comma;
	sf::Keyboard::Key HOLD2 = sf::Keyboard::RShift;

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
	// KeyRecorder

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