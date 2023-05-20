#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include "Tetromino.h"

using namespace std;
using namespace TetrisVariables;

class Tile {
	bool hasBlock; // For blocks that have dropped
	bool hasMovingBlock; // For blocks that are still in control
	bool hasPreviewBlock; // For block quickdrop preview
	sf::Sprite sprite;
	int xPos, yPos;
public:
	Tile(sf::Texture& texture, int xPos, int yPos) {
		this->xPos = xPos;
		this->yPos = yPos;
		sprite.setPosition(xPos + 1, yPos + 1); // Very minor adjustment to help sprites stay within outlines
		sprite.setTexture(texture);
		hasBlock = false;
		hasMovingBlock = false;
		hasPreviewBlock = false;
	}
	// Move tile position down when clearing lines
	void moveDown() { 
		yPos += TILESIZE;
		sprite.setPosition(xPos, yPos);
	}
	void setColor(sf::Color& blockColor) {
		sprite.setColor(blockColor);
	}
	void setBlock(bool value) {
		hasBlock = value;
	}
	void setBlock(bool value, sf::Color& blockColor) { // Overload to toggle drawing block and set color
		hasBlock = value;
		sprite.setColor(blockColor);
	}
	void setMovingBlock(bool value) {
		hasMovingBlock = value;
	}
	void setMovingBlock(bool value, sf::Color& blockColor) { // Overload to toggle drawing block and set color
		hasMovingBlock = value;
		sprite.setColor(blockColor);
	}
	void setPreviewBlock(bool value) {
		hasPreviewBlock = value;
	}
	void setPreviewBlock(bool value, sf::Color& blockColor) { // Overload to toggle drawing block and set color
		hasPreviewBlock = value;
		sprite.setColor(blockColor);
	}
	void draw(sf::RenderWindow* window) {
		if (hasMovingBlock || hasBlock || hasPreviewBlock) 
			window->draw(sprite);
	}
	sf::Sprite& getSprite() {
		return sprite;
	}
	bool getHasBlock() {
		return hasBlock;
	}
	bool getHasMovingBlock() {
		return hasMovingBlock;
	}
};