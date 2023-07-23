#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include "TetrisConstants.h"
using namespace TetrisVariables;

// Modified sf::Clock for ease of use and pausing
class sfClockAtHome {
	sf::Clock clock;
	sf::Time storage; // For pausing and resuming the clock;
	bool paused;
public:
	sfClockAtHome() {
		paused = false;
	}

	// Restarts both clock and storage
	void restart() {
		clock.restart();
		storage = sf::Time();
		paused = false;
	}
	// Saves current time in storage
	void pause() {
		if (paused)
			return;
		storage += clock.getElapsedTime();
		clock.restart();
		paused = true;
	}
	// Resets current clock only
	void resume() {
		if (!paused)
			return;
		clock.restart();
		paused = false;
	}
	// Returns paused status
	bool getPaused() {
		return paused;
	}
	// Returns time values that include stored time from pausing
	sf::Time getElapsedTime() {
		return clock.getElapsedTime() + storage;
	}
	float getTimeSeconds() {
		return clock.getElapsedTime().asSeconds() + storage.asSeconds();
	}
	sf::Int32 getTimeMilliseconds() {
		return clock.getElapsedTime().asMilliseconds() + storage.asMilliseconds();
	}

};

// Class to handle auto repeat / DAS
class KeyTimer {
	sf::Clock startTimer, holdTimer;
	int startDelay, holdDelay; // In milliseconds
	bool startOn, holdOn;
public:
	KeyTimer(int startDelay = 0, int holdDelay = 0) {
		this->startDelay = startDelay;
		this->holdDelay = holdDelay;
		startOn = false;
		holdOn = false;
	}
	// Handles action while key is pressed. Returns true if an action should be performed
	bool press() {
		if (holdOn) { // If startOn and holdOn are true, check for auto repeat
			if (holdTimer.getElapsedTime().asMilliseconds() >= holdDelay) {
				holdTimer.restart();
				return true;
			}
			return false;
		}
		// If holdOn isn't true, check startOn for startTimer
		if (!startOn) {
			startOn = true;
			startTimer.restart();
			return true;
		}
		else {
			if (startTimer.getElapsedTime().asMilliseconds() >= startDelay) {
				holdOn = true;
				holdTimer.restart();
			}
			return false;
		}
	}
	// Key is released. Reset boolean values
	void release() {
		holdOn = false;
		startOn = false;
	}
};

// Controls for each player
class KeySet {
	sf::Keyboard::Key left;
	sf::Keyboard::Key right;
	sf::Keyboard::Key up;
	sf::Keyboard::Key down;
	sf::Keyboard::Key spinCW;
	sf::Keyboard::Key spinCCW;
	sf::Keyboard::Key hold;
public:
	KeySet(sf::Keyboard::Key left, sf::Keyboard::Key right, sf::Keyboard::Key up,
		sf::Keyboard::Key down, sf::Keyboard::Key spinCW, sf::Keyboard::Key spinCCW, sf::Keyboard::Key hold) {
		this->left = left;
		this->right = right;
		this->up = up;
		this->down = down;
		this->spinCW = spinCW;
		this->spinCCW = spinCCW;
		this->hold = hold;
	}
	void setLeft(sf::Keyboard::Key key) {
		left = key;
	}
	void setRight(sf::Keyboard::Key key) {
		right = key;
	}
	void setUp(sf::Keyboard::Key key) {
		up = key;
	}
	void setDown(sf::Keyboard::Key key) {
		down = key;
	}
	void setSpinCW(sf::Keyboard::Key key) {
		spinCW = key;
	}
	void setSpinCCW(sf::Keyboard::Key key) {
		spinCCW = key;
	}
	void setHold(sf::Keyboard::Key key) {
		hold = key;
	}
	sf::Keyboard::Key getLeft() {
		return left;
	}
	sf::Keyboard::Key getRight() {
		return right;
	}
	sf::Keyboard::Key getUp() {
		return up;
	}
	sf::Keyboard::Key getDown() {
		return down;
	}
	sf::Keyboard::Key getSpinCW() {
		return spinCW;
	}
	sf::Keyboard::Key getSpinCCW() {
		return spinCCW;
	}
	sf::Keyboard::Key getHold() {
		return hold;
	}
};

// Organize DAS for each player
class KeyDAS {
	KeyTimer leftKey;
	KeyTimer rightKey;
	KeyTimer downKey;
	KeySet* keySet;
public:
	KeyDAS(int startDelay = 0, int holdDelay = 0, KeySet* keySet = nullptr) {
		leftKey = KeyTimer(startDelay, holdDelay);
		rightKey = KeyTimer(startDelay, holdDelay);
		downKey = KeyTimer(startDelay, holdDelay);
		this->keySet = keySet;
	}
	template<typename T>
	// Handles movement with auto-repeat (DAS). 
	void checkKeyPress(T& screen) { // Doesn't work if parameter type is Screen& for some reason. Needs template
		if (sf::Keyboard::isKeyPressed(keySet->getLeft())) {
			if (leftKey.press())
				screen.movePiece(0);
		}
		else if (sf::Keyboard::isKeyPressed(keySet->getRight()) && (rightKey.press()))
			screen.movePiece(2);
		// The code above prioritizes the left key if both left and right are held.
		if (sf::Keyboard::isKeyPressed(keySet->getDown()) && (downKey.press()))
			screen.movePiece(1);
	}
	void releaseKey(sf::Keyboard::Key& event) {
		if (event == keySet->getLeft())
			leftKey.release();
		else if (event == keySet->getDown())
			downKey.release();
		else if (event == keySet->getRight())
			rightKey.release();
	}
};

// Class to store random piece order to it is consistent across all players
class pieceBag {
	vector<char> pieceQueue; // Complete piece order
	vector<unsigned int> positions; // Position in the queue for each player
public:
	pieceBag() {
		addBatch();
	}
	// Adds a bag of 7 numbers to the queue when needed.
	void addBatch() {
		vector<char> queueBatch{ 0, 1, 2, 3, 4, 5, 6 };
		random_shuffle(queueBatch.begin(), queueBatch.end());
		for (char num : queueBatch) {
			pieceQueue.push_back(num);
		}
	}
	// Adds a player who is accessing the queue. Returns the player index
	int addPlayer() {
		positions.push_back(0);
		return positions.size() - 1;
	}
	// Resets the queue
	// NOTE: resetQueue must be called before resetPosition 
	// during each reset for a proper starting bag for all players
	void resetQueue() {
		pieceQueue.clear();
		addBatch();
	}
	void resetPosition(int playerIndex) {
		positions[playerIndex] = 0;
	}
	// Get piece and increment position
	int getPiece(int playerIndex) {
		positions[playerIndex]++;
		if (pieceQueue.size() < positions[playerIndex] + 7)
			addBatch(); // Replenish queue
		return pieceQueue[positions[playerIndex] - 1];
	}
	// Returns the queue of next pieces to display
	vector<int> getNextPieces(int playerIndex, int pieceCount) {
		vector<int> queue;
		for (int i = 0; i < pieceCount; i++)
			queue.push_back(pieceQueue[positions[playerIndex] + i]);
		return queue;
	}
};
// A batch of garbage with a timer before it is dumped onto a board
class Garbage {
	int size;
	float duration; // In seconds. Time before garbage is dumped.
	sfClockAtHome timer;
public:
	Garbage(int size, float duration) {
		this->size = size;
		this->duration = duration;
		timer.restart();
	}
	void setSize(int size) {
		this->size = size;
	}
	int getSize() {
		return size;
	}
	float getDuration() {
		return duration;
	}
	float getTime() {
		return timer.getElapsedTime().asSeconds();
	}
	// Return true if timer has exceeded duration
	bool checkTimer() {
		return (timer.getElapsedTime().asSeconds() >= duration);
	}
	void pause() {
		timer.pause();
	}
	void resume() {
		timer.resume();
	}
};
// Queue for incoming garbage
class GarbageBin {
	vector<Garbage> bin;
	float currentDuration;
public:
	GarbageBin() {
		currentDuration = BASEGARBAGETIMER;
	}
	// Add a batch of garbage
	void addGarbage(int count) {
		bin.push_back(Garbage(count, currentDuration));
	}
	void setDuration(float duration) {
		currentDuration = duration;
	}
	// Remove garbage at front of queue. 
	// Returns number of lines to send back to opponent
	int clearGarbage(int count) {
		if (isEmpty())
			return count;
		int difference = count - bin[0].getSize();
		// If count is smaller than the first garbage batch, remove lines from the garbage
		if (difference < 0) {
			bin[0].setSize(-difference);
			return 0;
		}
		else {
			// Delete batch
			bin.erase(bin.begin());
			// If count is bigger than first batch, overflow to the next batch
			if (difference > 0)
				return clearGarbage(difference);
			return 0;
		}
	}
	// Return true if garbage bin is empty
	bool isEmpty() {
		return bin.size() == 0;
	}
	// Return the number of lines of garbage to be dumped
	int checkGarbage() {
		if (isEmpty() || !bin[0].checkTimer())
			return 0;
		int count;
		if (bin[0].checkTimer()) {
			// Garbage that is ready to be dumped leaves the bin and is stored in screen.inGarbage
			count = bin[0].getSize();
			bin.erase(bin.begin());
		}
		else
			count = 0;
		return count;
	}
	void clear() {
		bin.clear();
	}
	// Output a visual representation of the garbage bin for debug purposes
	void printBin() {
		for (Garbage& garb : bin) {
			for (int i = 0; i < garb.getSize(); i++) {
				cout << "[] " << garb.getTime() << "/" << garb.getDuration() << endl;
			}
		}
	}
	void pause() {
		for (Garbage& garb : bin)
			garb.pause();
	}
	void resume() {
		for (Garbage& garb : bin)
			garb.resume();
	}
	// Get a textual representation of the garbage that is remaining in the bin.
	vector<float> getBin() {
		vector<float> vec;
		for (Garbage& garb : bin) {
			for (int i = 0; i < garb.getSize(); i++) {
				vec.push_back(garb.getTime() / garb.getDuration());
			}
		}
		return vec;
	}
};

