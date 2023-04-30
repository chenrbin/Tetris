#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
using namespace std;
namespace TetrisVariables {
	// Set constant variables
	const int WIDTH = 800, HEIGHT = 800, LEFTMARGIN = 150;
	const int TILESIZE = 32, NUMROWS = 20, NUMCOLS = 10;
	const int GAMEWIDTH = TILESIZE * NUMCOLS, GAMEHEIGHT = TILESIZE * NUMROWS;
	const int LINEWIDTH = 2, TOPROWPIXELS = 10;
	const float SCALE = 0.8;
	const int FPS = 60; // Frame limit of the game
	const float LOCKDELAY = 0.5f; // Delay before a piece sets in seconds
	const float DEFAULTGRAVITY = 1; // Time between gravity movements in seconds
	const int NEXTPIECECOUNT = 5; // Number of next pieces visible. Max is 7. Will crash if above 7.

	// Game screen state codes
	const int MENUSCREEN = 1, GAMESCREEN = 2, SANDBOXSCREEN = 3;
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
	const int PREVIEWTRANSPARENCY = 120;
	
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