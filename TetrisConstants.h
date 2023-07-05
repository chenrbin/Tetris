#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
using namespace std;
namespace TetrisVariables {
	// Todo: instructions, review comments, scoring, pause screen, pvp rng
	// Known bugs:

	// Set constant variables

	// Item sizes and spacings
	const int TILESIZE = 32, LEFTMARGIN = 150, GAMETEXTSIZE = 20, MENUTEXTSIZE = 24, CLEARTEXTSIZE = 30;
	const int LINEWIDTH = 2, TOPROWPIXELS = 10, MENUSPACING = 40;
	const float SCALE = 0.8;

	// Board dimensions
	const int WIDTH = 800, HEIGHT = 800, NUMROWS = 20, NUMCOLS = 10;
	const int GAMEWIDTH = TILESIZE * NUMCOLS, GAMEHEIGHT = TILESIZE * NUMROWS;

	// Game mechanic related variables
	const int FPS = 60; // Frame limit of the game
	const float LOCKDELAY = 0.5f; // Delay before a piece sets in seconds 68
	const float SUPERLOCKSECONDS = 3;
	const int SUPERLOCKFRAMECOUNT = FPS * SUPERLOCKSECONDS; // Second lock delay to prevent infinites
	const float DEFAULTGRAVITY = 1; // Time between gravity movements in seconds
	const int NEXTPIECECOUNT = 6; // Number of next pieces visible. Max is 6. Will crash if above 7.
	const int GRAVITYTIERLINES[] = { 0, 20, 40, 60, 80, 100, 120 };
	const float GRAVITYSPEEDS[] = { 1, 0.75, 0.5, 0.25, 0.1, 0.05, 0.01 };
	const int GRAVITYTIERCOUNT = 7;

	// Rectangle positions
	const float MENUXPOS = WIDTH / 1.7f, MENUYPOS = HEIGHT / 2 - 40;
	const int GAMEXPOS = LEFTMARGIN, GAMEYPOS = ((HEIGHT - GAMEHEIGHT) / 2);
	const sf::Vector2f TITLETEXTPOS(WIDTH / 2, 100);
	const sf::Vector2f GAMEPOS(GAMEXPOS, GAMEYPOS);
	const sf::Vector2f SANDBOXMENUPOS(GAMEXPOS + GAMEWIDTH + LINEWIDTH * 2, GAMEYPOS + GAMEHEIGHT / 1.5);

	// Game screen state codes
	const int MAINMENU = 1, CLASSIC = 2, SANDBOX = 3, MULTIPLAYER = 4, CLASSICLOSS = 5;
	
	// Set color constants for easy use and passing to functions
	const sf::Color WHITE(255, 255, 255);
	const sf::Color BLACK(0, 0, 0);
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

	const sf::Color TPIECECOLOR = VIOLET; // Defining T piece color for detecting T-spins
	
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