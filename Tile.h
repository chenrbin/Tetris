#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include "Tetromino.h"

using namespace std;
using namespace TetrisVariables;

class Tile {
	bool hasBlock; // For blocks that have dropped
	bool hasMovingBlock; // For blocks that are still in control
	bool hasPreviewBlock; // For blocks quickdrop preview
	sf::Sprite sprite;
	int xPos, yPos;
public:
	Tile(sf::Texture& texture, int xPos, int yPos) {
		this->xPos = xPos;
		this->yPos = yPos;
		sprite.setPosition(xPos, yPos);
		sprite.setTexture(texture);
		hasBlock = false;
		hasMovingBlock = false;
		hasPreviewBlock = false;
	}
	void moveDown() { // Move tile position down when clearing lines
		yPos += TILESIZE;
		sprite.setPosition(xPos, yPos);
	}
	void setColor(sf::Color& blockColor) {
		sprite.setColor(blockColor);
	}
	void setBlock() {
		hasBlock = !hasBlock;
	}
	void setBlock(sf::Color& blockColor) { // Overload to toggle drawing block and set color
		hasBlock = !hasBlock;
		sprite.setColor(blockColor);
	}
	void setMovingBlock() {
		hasMovingBlock = !hasMovingBlock;
	}
	void setMovingBlock(sf::Color& blockColor) { // Overload to toggle drawing block and set color
		hasMovingBlock = !hasMovingBlock;
		sprite.setColor(blockColor);
	}
	void setPreviewBlock() {
		hasPreviewBlock = !hasPreviewBlock;
	}
	void setPreviewBlock(sf::Color& blockColor) { // Overload to toggle drawing block and set color
		hasPreviewBlock = !hasPreviewBlock;
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