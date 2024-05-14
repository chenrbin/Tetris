#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "TetrisConstants.h"
#include "Screen.h"
#include "Drawing.h"
#include "Mechanisms.h"
using namespace std;
using namespace TetrisVariables;
// Sandbox functionality interface
class SandboxMenu : public sf::Drawable {
    vector<sf::Text> sandboxText; // HUD text
    Checkbox* autoFallBox; // Toggles gravity
    IncrementalBox* gravityBox; // Set gravity speed
    Checkbox* creativeModeBox; // Toggle creative mode
    Checkbox* resetBox; // Reset game
    Checkbox* quitBox; // Quit to menu
    vector<Checkbox*> sandboxes; // Pointer to boxes
    Screen* screen; // Game screen. Only links to player 1 as sandbox mode is single player
protected:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
        for (const sf::Text& text : sandboxText)
            target.draw(text, states);
        for (auto box : sandboxes)
            target.draw(*box, states);
    }
public:
    SandboxMenu(sf::Font& font, Screen* screen) {
        vector<string> menuItems = { "Auto-fall", "Fall speed", "Creative", "Reset", "Quit" };
        for (int i = 0; i < menuItems.size(); i++)
            sandboxText.push_back(SfTextAtHome(font, WHITE, menuItems[i], MENUTEXTSIZE, { SANDBOXMENUPOS.x, SANDBOXMENUPOS.y + MENUSPACING * i }));
        autoFallBox = new Checkbox(TILESIZE, SANDBOXMENUPOS.x + 180, SANDBOXMENUPOS.y, true, font);
        gravityBox = new IncrementalBox(TILESIZE, SANDBOXMENUPOS.x + 180, SANDBOXMENUPOS.y + MENUSPACING, 1, GRAVITYTIERCOUNT, font);
        creativeModeBox = new Checkbox(TILESIZE, SANDBOXMENUPOS.x + 180, SANDBOXMENUPOS.y + MENUSPACING * 2, false, font);
        resetBox = new Checkbox(TILESIZE, SANDBOXMENUPOS.x + 180, SANDBOXMENUPOS.y + MENUSPACING * 3, false, font);
        quitBox = new Checkbox(TILESIZE, SANDBOXMENUPOS.x + 180, SANDBOXMENUPOS.y + MENUSPACING * 4, false, font);
        sandboxes = { autoFallBox, gravityBox, creativeModeBox, resetBox, quitBox };
        this->screen = screen;
    }
    ~SandboxMenu() {
        for (Checkbox* box : sandboxes)
            delete box;
    }
    void toggleGravity() {
        if (autoFallBox->getChecked()) {
            autoFallBox->setChecked(false);
            screen->setAutoFall(false);
        }
        else {
            autoFallBox->setChecked(true);
            screen->setAutoFall(true);
        }
    }
    void decrementGravity() {
        gravityBox->decrement();
        screen->setGravity(GRAVITYSPEEDS[gravityBox->getCurrentNum() - 1]);
    }
    void incrementGravity() {
        gravityBox->increment();
        screen->setGravity(GRAVITYSPEEDS[gravityBox->getCurrentNum() - 1]);
    }
    void toggleCreative() {
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
    // Handle key presses for sandbox-exclusive functions. Pass in variables that may be manipulated

    void onKeyPress(sf::Keyboard::Key& key, PieceBag& bag, sf::Music& bgm, int& currentScreen) {
        // Hot keys for sandbox controls
        if (key == sf::Keyboard::Q || key == sf::Keyboard::Escape)
            toggleGravity();
        else if (key == sf::Keyboard::W)
            decrementGravity();
        else if (key == sf::Keyboard::S)
            incrementGravity();
        else if (key == sf::Keyboard::E)
            toggleCreative();
        else if (key == sf::Keyboard::R) {
            bag.resetQueue();
            screen->resetBoard();
        }
        else if (key == sf::Keyboard::T) {
            bgm.stop();
            currentScreen = MAINMENU;
        }
        // Generate garbage
        else if (key == sf::Keyboard::G)
            screen->receiveGarbage(1);
        else if (key == sf::Keyboard::H)
            screen->receiveGarbage(4);
    }
    // Handle left mouse. Pass in variables that may be manipulated
    void onLeftClick(sf::Vector2f& clickPos, PieceBag& bag, sf::Music& bgm, int& currentScreen){
        if (autoFallBox->getBounds().contains(clickPos))  // Turn off gravity
            toggleGravity();
        else if (gravityBox->getLeftBound().contains(clickPos))  // Speed arrows
            decrementGravity();
        else if (gravityBox->getRightBound().contains(clickPos))
            incrementGravity();
        else if (creativeModeBox->getBounds().contains(clickPos))
            toggleCreative();
        else if (resetBox->getBounds().contains(clickPos)) {
            bag.resetQueue();
            screen->resetBoard();
        }
        else if (quitBox->getBounds().contains(clickPos)) { // Return to menu
            bgm.stop();
            currentScreen = MAINMENU;
        }
    }
    // Shows a transparent X when hovering over unchecked boxes
    void onMouseMove(sf::Vector2f& clickPos) {
        for (Checkbox* box : sandboxes)
            box->setHovering(box->getBounds().contains(clickPos));
    }
    // Reset sandbox settings
    void reset(){
        autoFallBox->setChecked(true);
        gravityBox->setValue(gravityBox->getMin());
        creativeModeBox->setChecked(false);
    }
};