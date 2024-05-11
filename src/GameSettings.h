#pragma once
#include "Drawing.h"
#include "TetrisConstants.h"
#include <vector>
#include <SFML/Graphics.hpp>
using namespace TetrisVariables;
// Contains the setting data structure

// A single tab containing configurable settings
class SettingsTab : public sf::Drawable {
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
    ~SettingsTab() {
        for (OptionSelector* sel : settingSelectors)
            delete sel;
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
            if (keyRec->getSelected() && keyRec->readKey(key)) {
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

// Menu containing multiple tabs
class SettingsMenu : public sf::Drawable {
    vector<SettingsTab> tabs;
    int tabCount;
    int currentTabIndex;
    SoundManager* soundFX;
    sf::Font* font;
    // Draw all tabs
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
        for (int i = 0; i < tabCount; i++)
            target.draw(tabs[i], states);
    }
public:
    SettingsMenu(SoundManager* soundFX, sf::Font& font, map<sf::Keyboard::Key, string>& keyStrings) {
        tabCount = 0;
        currentTabIndex = 0;
        this->soundFX = soundFX;
        this->font = &font;
        addTab(font, "Gameplay");
        addTab(font, "Controls");
        addTab(font, "Graphics (WIP)");
        selectTab(0);

        vector<sf::Vector2f> settingPositions;
        vector<OptionSelector*> selectors;
        vector<sf::Vector2f> selectorPositions;
        vector<string> settingText;

        // Set up text, selectors, and positions for tab 1
        for (float i = 0; i < 11; i++) { // Positions 
            settingPositions.push_back({ SETTINGXPOS, SETTINGYPOS + SETTINGSPACING * i });
            selectorPositions.push_back({ SETTINGXPOS + SELECTORRIGHTSPACING, SETTINGYPOS + SETTINGSPACING * i });
        }
        settingText = { "Starting Speed","Next Piece Count", "Piece Holding", "Ghost Piece", "Auto Shift Delay", "Auto Shift Speed",
            "Piece RNG", "Rotation Style", "Garbage Timer", "Garbage Multiplier", "Garbage RNG" };

        selectors.push_back(new SlidingBar(270, { "Easy", "Normal", "Hard" }, font));
        selectors.push_back(new SlidingBar(270, { "0", "1", "2", "3", "4", "5", "6" }, font));
        selectors.push_back(new OnOffSwitch(font));
        selectors.push_back(new OnOffSwitch(font));
        selectors.push_back(new SlidingBar(270, { "Long", "Normal", "Short", "Instant" }, font));
        selectors.push_back(new SlidingBar(270, { "Slow", "Normal", "Fast", "Instant" }, font));
        selectors.push_back(new SlidingBar(150, { "Random", "7-Bag" }, font));
        selectors.push_back(new SlidingBar(150, { "Classic", "Modern" }, font));
        selectors.push_back(new SlidingBar(270, { "5s", "3s", "1s", "Instant" }, font));
        selectors.push_back(new SlidingBar(270, { "0.5x", "1x", "1.5x" }, font));
        selectors.push_back(new SlidingBar(270, { "Easy", "Normal", "Hard" }, font));

        // Add settings to tab 1
        for (int i = 0; i < selectors.size(); i++)
            tabs[0].addSetting(settingText[i], settingPositions[i], selectors[i], selectorPositions[i], font);

        // Set contents for tab 2
        // Generate positions for 21 keybinds
        vector<sf::Vector2f> keybindPositions;
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 7; j++)
                keybindPositions.push_back(sf::Vector2f(SETTINGXPOS + SELECTORRIGHTSPACING / 1.5f * (i + 1), SETTINGYPOS + SETTINGSPACING * (j + 1)));
        // Add key recorders to settings tab
        for (sf::Vector2f& vec : keybindPositions)
            tabs[1].addKeybind("", ORIGIN, &keyStrings, vec, font);
        vector<string> controlsText = { "Hard Drop", "Left", "Down", "Right", "SpinCW", "SpinCCW", "Hold", "Solo", "PVP P1", "PVP P2" };
        for (int i = 0; i < 7; i++)
            tabs[1].addExtraText(SfTextAtHome(font, WHITE, controlsText[i], MENUTEXTSIZE, sf::Vector2f(SETTINGXPOS, SETTINGYPOS + SETTINGSPACING * (i + 1))));
        for (int i = 7; i < 10; i++)
            tabs[1].addExtraText(SfTextAtHome(font, WHITE, controlsText[i], MENUTEXTSIZE, sf::Vector2f(SETTINGXPOS + SELECTORRIGHTSPACING / 1.5f * (i - 6), SETTINGYPOS), true, false, true));
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
            int tab2Size = tabs[1].getSelectors().size();
            for (int i = 0; i < tab1Size; i++) // First tab
                tabs[0][i].setIndex(config[i]);
            for (int i = 0; i < tab2Size; i++) { // Second tab
                tabs[1].setKey(i, sf::Keyboard::Key(config.at(i + tab1Size)));
            }
        }
        catch (out_of_range err) {
            cout << "applyConfig out of range. Restoring to default\n";
            cout << err.what();
            applyConfig(DEFAULTSETTINGS);
        }
        catch (ConfigError err) {
            applyConfig(DEFAULTSETTINGS);
        }
    }
};