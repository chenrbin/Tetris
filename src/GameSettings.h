#pragma once
#include <vector>
#include <SFML/Graphics.hpp>
#include "TetrisConstants.h"
#include "Screen.h"

using namespace std;
using namespace TetrisVariables;

// Map between keys and their corresponding text to display
// This global variable will be used by several classes
map<sf::Keyboard::Key, string> keyStrings; 

// Contains the setting data structures
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
    // Get config values of current tab
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
    void addKeybind(string text, sf::Vector2f textPosition, sf::Vector2f selectorPosition, sf::Font& font) {
        keybinds.push_back(new KeyRecorder(&keyStrings, font));
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
    // Returns only the keybind configurations
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
    void onMouseMove(int mouseX, int mouseY) {
        for (OptionSelector* selector : settingSelectors)
            if (selector->onMouseMove(mouseX, mouseY))
                break; // Break after a successful action. Skips the need to check everything
    }
    void onMouseClick(int mouseX, int mouseY) {
        for (OptionSelector* selector : settingSelectors)
            if (selector->onMouseClick(mouseX, mouseY)) {
                soundFX->play(LIGHTTAP);
                break; // Since no sprites overlap, this saves the need to check all bounds;
            }
    }
    void onMouseRelease() {
        for (OptionSelector* selector : settingSelectors)
            selector->onMouseRelease();
    }
};

// Menu containing multiple tabs
class SettingsMenu : public sf::Drawable {
    // Menu interface
    vector<SettingsTab> tabs;
    int tabCount;
    int currentTabIndex;
    ClickableButton resetButton; // Reset to default
    ClickableButton discardButton; // Discard and continue
    ClickableButton saveButton; // Save and continue
    SoundManager* soundFX;
    sf::Font* font;
    int* currentScreen; // Pointer to current screen to return to the menu screen 

    // Setting data
    vector<Screen*> screens; // Game screens the setting will apply to
    vector<KeyDAS*> dasSets; // Control profiles to modify
    vector<int> configValues; // A saved copy of config values. Only updates when reading or writing the config file
    string fileName;

    // Draw all tabs
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
        for (int i = 0; i < tabCount; i++)
            target.draw(tabs[i], states);
        target.draw(resetButton, states);
        target.draw(discardButton, states);
        target.draw(saveButton, states);
    }


public:
    SettingsMenu(vector<Screen*> screens, vector<KeyDAS*> dasSets, SoundManager* soundFX, sf::Font& font, int* currentScreen) {
        tabCount = 0;
        currentTabIndex = 0;
        this->screens = screens;
        this->dasSets = dasSets;
        this->soundFX = soundFX;
        this->font = &font;
        this->currentScreen = currentScreen;
        fileName = CONFIGFILEPATH;
        // Initialize keyStrings if empty
        if (keyStrings.empty())
            initializeKeyStrings(keyStrings);

        // Add exit buttons
        resetButton = ClickableButton({ 230, 40 }, { 150, 725 }, font, "Reset Defaults", BUTTONTEXTSIZE, RED);
        discardButton = ClickableButton({ 230, 40 }, { 400, 725 }, font, "Discard & Quit", BUTTONTEXTSIZE, RED);
        saveButton = ClickableButton({ 230, 40 }, { 650, 725 }, font, "Save & Quit", BUTTONTEXTSIZE, RED);

        // Add tabs
        addTab(font, "Gameplay");
        addTab(font, "Controls");
        addTab(font, "Graphics");
        selectTab(0);

        vector<string> tab1Text;
        vector<sf::Vector2f> tab1TextPositions;
        vector<OptionSelector*> tab1Selectors;
        vector<sf::Vector2f> tab1SelectorPositions;

        // Set up text, selectors, and positions for tab 1
        for (float i = 0; i < 11; i++) { // Positions 
            tab1TextPositions.push_back({ SETTINGXPOS, SETTINGYPOS + SETTINGSPACING * i });
            tab1SelectorPositions.push_back({ SETTINGXPOS + SELECTORRIGHTSPACING, SETTINGYPOS + SETTINGSPACING * i });
        }
        tab1Text = { "Starting Speed","Next Piece Count", "Piece Holding", "Ghost Piece", "Auto Shift Delay", "Auto Shift Speed",
            "Piece RNG", "Rotation Style", "Garbage Timer", "Garbage Multiplier", "Garbage RNG" };

        tab1Selectors.push_back(new SlidingBar(270, { "Easy", "Normal", "Hard" }, font));
        tab1Selectors.push_back(new SlidingBar(270, { "0", "1", "2", "3", "4", "5", "6" }, font));
        tab1Selectors.push_back(new OnOffSwitch(font));
        tab1Selectors.push_back(new OnOffSwitch(font));
        tab1Selectors.push_back(new SlidingBar(270, { "Long", "Normal", "Short", "Instant" }, font));
        tab1Selectors.push_back(new SlidingBar(270, { "Slow", "Normal", "Fast", "Instant" }, font));
        tab1Selectors.push_back(new SlidingBar(150, { "Random", "7-Bag" }, font));
        tab1Selectors.push_back(new SlidingBar(150, { "Classic", "Modern" }, font));
        tab1Selectors.push_back(new SlidingBar(270, { "5s", "3s", "1s", "Instant" }, font));
        tab1Selectors.push_back(new SlidingBar(270, { "0.5x", "1x", "1.5x" }, font));
        tab1Selectors.push_back(new SlidingBar(270, { "Easy", "Normal", "Hard" }, font));

        // Add settings to tab 1
        for (int i = 0; i < tab1Selectors.size(); i++)
            tabs[0].addSetting(tab1Text[i], tab1TextPositions[i], tab1Selectors[i], tab1SelectorPositions[i], font);

        // Set contents for tab 2
        // Generate positions for 21 keybinds
        vector<sf::Vector2f> keybindPositions;
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 7; j++)
                keybindPositions.push_back(sf::Vector2f(SETTINGXPOS + SELECTORRIGHTSPACING / 1.5f * (i + 1), SETTINGYPOS + SETTINGSPACING * (j + 1)));
        // Add key recorders to settings tab
        for (sf::Vector2f& vec : keybindPositions)
            tabs[1].addKeybind("", ORIGIN, vec, font);
        vector<string> controlsText = { "Hard Drop", "Left", "Down", "Right", "SpinCW", "SpinCCW", "Hold"};
        vector<string> playersText = { "Solo", "PVP P1", "PVP P2" };
        for (int i = 0; i < controlsText.size(); i++)
            tabs[1].addExtraText(SfTextAtHome(font, WHITE, controlsText[i], MENUTEXTSIZE, sf::Vector2f(SETTINGXPOS, SETTINGYPOS + SETTINGSPACING * (i + 1))));
        for (int i = 0; i < playersText.size(); i++)
            tabs[1].addExtraText(SfTextAtHome(font, WHITE, playersText[i], MENUTEXTSIZE, sf::Vector2f(SETTINGXPOS + SELECTORRIGHTSPACING / 1.5f * (i + 1), SETTINGYPOS), true, false, true));

        // Set contents for tab 3
        vector<string> tab3Text {"Block Colors"};
        vector<sf::Vector2f> tab3TextPositions {{100, 100}};
        vector<OptionSelector*> tab3Selectors { new BulletListSelector(50, {"1", "2", "3"}, font)};
        vector<sf::Vector2f> tab3SelectorPositions = {{100, 150}};

        // Add settings to tab 1
        for (int i = 0; i < tab3Selectors.size(); i++)
            tabs[2].addSetting(tab3Text[i], tab3TextPositions[i], tab3Selectors[i], tab3SelectorPositions[i], font);

        // Read settings file
        readConfigFile();
        resetConfig();
        updateAllSettings();
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
    void onMouseMove(int mouseX, int mouseY) {
        tabs[currentTabIndex].onMouseMove(mouseX, mouseY);
    }
    void onMouseClick(int mouseX, int mouseY) {
        clickTab(mouseX, mouseY);
        tabs[currentTabIndex].onMouseClick(mouseX, mouseY);
        // Restore to default settings
        if (resetButton.checkClick(mouseX, mouseY)) {
            resetConfig(true);
            soundFX->play(MEDIUMBEEP);
        }
        // Discard & quit
        else if (discardButton.checkClick(mouseX, mouseY)) {
            resetConfig();
            *currentScreen = MAINMENU;
            soundFX->play(HIGHBEEP);
        }
        // Save & quit
        else if (saveButton.checkClick(mouseX, mouseY)) {
            // Update gameplay settings for two screens and three DAS sets
            updateAllSettings();
            writeConfigFile();
            *currentScreen = MAINMENU;
            soundFX->play(HIGHBEEP);
        }
    }
    void onMouseRelease() {
        tabs[currentTabIndex].onMouseRelease();
    }
    SettingsTab& operator[](int index) {
        return tabs[index];
    }
    // Obtain the config vector of current settings
    vector<int> getValues() {
        vector<int> vec;
        for (SettingsTab& tab : tabs)
            for (int val : tab.getValues())
                vec.push_back(val);
        return vec;
    }
    // Reset settings to saved configValues (false) or defaultValues (true)
    // This function does not change gameplay variables
    void resetConfig(bool defaultValues = false) {
        const vector<int>& config = (defaultValues) ? DEFAULTSETTINGS : configValues;
        try {
            int index = 0;
            int tab1Size = tabs[0].getSelectors().size(); // Index to separate tabs
            int tab2Size = tabs[1].getSelectors().size();
            int tab3Size = tabs[2].getSelectors().size();
            for (int i = 0; i < tab1Size; i++) // First tab
                tabs[0][i].setIndex(config.at(index++));
            for (int i = 0; i < tab2Size; i++) { // Second tab
                tabs[1].setKey(i, sf::Keyboard::Key(config.at(index++)));
            }
            for (int i = 0; i < tab3Size; i++) // Third tab
                tabs[2][i].setIndex(config.at(index++));
        }
        catch (out_of_range err) {
            cout << "config out of range. Restoring to default\n";
            cout << err.what();
            resetConfig(true);
        }
        writeConfigFile();
    }
    // Read config file and save the vector of integers
    void readConfigFile() {
        ifstream inFile(fileName);
        try {
            if (!inFile.is_open())
                throw ConfigError();
            configValues.clear();
            string line;
            while (getline(inFile, line))
                configValues.push_back(stoi(line));
        }
        catch (ConfigError err) {
            cout << "Config file does not exist. Creating default file.\n";
            resetConfig(true);
            writeConfigFile();
        }
        catch (exception err) {
            cout << "Config file reading error. Restoring to defaults.\n";
            resetConfig(true);
            writeConfigFile();
        }
        inFile.close();
    }

    // Write setting menu contents to a file
    void writeConfigFile() {
        configValues = getValues();
        ofstream outFile(fileName);
        if (!outFile.is_open()) {
            cout << "Failed to write file";
            throw exception();
        }
        for (int val : configValues)
            outFile << to_string(val) << endl;
        outFile.close();
    }

    // Convert settings menu contents to gameplay variables
    void updateAllSettings() {
        vector<int> settings = tabs[0].getValues();
        // Update game screens
        for (Screen* screen : screens) {
            // Starting speed. Will not take effect in sandbox mode
            screen->setStartingGravity(GRAVITYSPEEDS[settings[0]]);
            screen->setNextPieceCount(settings[1]); // Next piece count
            screen->setHoldEnabled(settings[2]);	// Piece holding
            screen->setGhostPieceEnabled(settings[3]); // Ghost piece
            screen->setBagEnabled(settings[6]); // 7-bag
            screen->setSRS(settings[7]); // Rotation style
            screen->setGarbageTimer(GARBAGETIMERS[settings[8]]); // Garbage timer
            screen->setGarbageMultiplier(GARBAGEMULTIPLIERS[settings[9]]); // Garbage multiplier
            screen->setGarbRepeatProbability(GARBAGEREPEATPROBABILITIES[settings[10]]); // Garbage repeat probability

            // Update hold and queue display
            screen->getScreenRects()[1].setSize(sf::Vector2f{ TILESIZE * 4.0f * settings[2], TILESIZE * 4.0f * settings[2] });
            screen->getScreenRects()[2].setSize(sf::Vector2f{ TILESIZE * 4.0f * settings[1] / settings[1], GAMEHEIGHT / 9.0f * settings[1] });

        }

        // Update das responsiveness
        vector<KeySet*> keySets;
        for (KeyDAS* keyDas : dasSets) {
            keyDas->setStartDelay(DASDELAYVALUES[settings[4]]); // DAS delay
            keyDas->setHoldDelay(DASSPEEDVALUES[settings[5]]); // DAS speed
            keySets.push_back(keyDas->getKeySet());
        }

        // Update keybind configurations from setting tab entries
        std::vector<KeyRecorder*> keybinds = tabs[1].getKeybinds();
        for (int i = 0; i < keybinds.size(); i++) // Update keybind controls
            *keySets[i / 7]->getSet()[i % 7] = *keybinds[i]->getKey();

        // Update color pallete
        settings = tabs[2].getValues();
        for (Screen* screen : screens) 
            screen->setColorPallete(settings[0]);
    }
};