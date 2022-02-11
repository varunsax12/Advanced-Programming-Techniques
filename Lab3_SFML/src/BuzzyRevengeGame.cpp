/*
Description:
    Solution file for Lab 3.
    Game: Buzzy's revenge
    Gameplay Overview:
        1. Buzzy is at launch point
        2. Other items in game:
            a. Flying insect (+75 points and insect disappear in level)
            b. Evil mascots (bulldog and tiger) -> (+25 points & column disappear)
            c. Woodland creatures (looses a life)
            d. Mad unicorn (gives extra life)
        3. Buzzy is launched to hit either of items with resulting action mentioned above
        4. Buzzy has 5 lives. If losses all lives, then game over.
        5. When both evil mascots hit, level is reset with all creatures randomly reset
*/

// General c++ includes
#include <list>
#include <array>
#include <iostream>
#include <limits>
#include <cmath>
#include <random>

// SFML includes
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

// Custom includes for game
#include "DisplayText.h"    // Defines the class for text display in game
#include "PowerBar.h"       // Defines the class for handling the power bar in game
#include "GameIcon.h"       // Defines the class for handling all game icons

// Game Icons => all the items created using images like buzzy, insect, woodland
// creatures, evil mascots, mad unicorn, buzzy's lives

// Add name space to reduce code complexity
using namespace sf;

/*
****************************************
Global variables definition
****************************************
*/

// Main window dimensions
float windowWidth;
float windowHeigth;

// String to track the location of all sfml support files => images, sounds, fonts
std::string parentLoc;

// Rate scaling to change speed based on display
int displaySpeedScaling = 5;

/*
****************************************
Helper Function implementations
****************************************
*/

/*
* Function to check if buzz out of bounds and reset
* @param buzzIcon -> game icon of buzz
* @param initX -> reset position for x coordinate
* @param initY -> reset position for y coordinate
* @param angle -> reset angle
* @return bool -> if buzz went out of bounds and was reset
*/
bool checkAndResetBuzz(GameIcon& buzzIcon, float initX, float initY, float angle)
{
    // check if buzz still within window bounds
    buzzIcon.checkOutOfScreenBounds();
    // check if buzz still displayed on screen
    if (buzzIcon.isDisplayed() == false)
    {
        // Reset the location of buzz icon
        buzzIcon.placeIconCustom(initX, initY);
        buzzIcon.setRotation(angle);

        // Return true if buzz went out of bounds
        return true;
    }
    // Return false if buzz still within bounds
    return false;
}

/*
* Function to randomize placement of grid icons (animals on the right side)
* @param listGameIcon -> list of game icons to randomize
* 
* Passed as reference to update original list directly
*/
void randomizeGridIcon(std::list<GameIcon*> &listGameIcon)
{
    // flag set when all right side grid icons successfully randomized
    // such that the evil mascots are not in the same column
    bool success = true;

    // creating random number generator
    std::random_device rd;
    std::default_random_engine randGenerator{ rd() };
    // Range of distribution is 0->9 as grid positions are 2 columns * 5 rows = 10
    std::uniform_int_distribution<int> randDistribution(0, 9);

    // do while loop run till successful placement done
    do
    {
        // grid range is 0 -> 9
        // Logic -> row = num/2, col = num % 2

        // Track location when 1st evil mascot places
        int evilMascotColumn = -1;

        // Track locations already filled in this randomize iteration
        std::array<bool, 10> placeOccupied{ false };
        for (auto gameIcon : listGameIcon)
        {
            // Current grid location, set to invalid grid location (-1)
            int gridLoc = -1;
            // Attempt to find random location which is not occupied
            do
            {
                gridLoc = randDistribution(randGenerator);
            } while (placeOccupied[gridLoc] == true); // retry if place occupied

            // Place icon in the grid location
            gameIcon->placeIconGrid(gridLoc);
            // set rotation as 0
            gameIcon->setRotation(0);

            // Check if the evil mascots in the same column
            if (gameIcon->getIsEvilMascot())
            {
                // check if evilMascot already placed
                if (evilMascotColumn == -1)
                {
                    // evil mascot not placed yet
                    evilMascotColumn = gridLoc % 2;
                }
                else if (gridLoc % 2 == evilMascotColumn)
                {
                    success = false;
                    // if in the same column
                    // check if evil mascot can be placed in other column
                    int col = gridLoc % 2;
                    if (col == 1)
                    {
                        col = 0;
                    }
                    else
                    {
                        col = 1;
                    }
                    // check if can place anywhere in this column
                    bool placed = false;
                    for (int i = 0; i < 5; ++i)
                    {
                        gridLoc = i + col;
                        if (placeOccupied[gridLoc] == false)
                        {
                            // place here
                            gameIcon->placeIconGrid(gridLoc);
                            placed = true;
                        }
                    }
                    if (placed)
                    {
                        // 2nd evil mascot placed successfully
                        success = true;
                    }
                }
            }
            // Mark the grid location as placed in occupied tracker
            placeOccupied[gridLoc] = true;

            // Enable display of game icon
            gameIcon->enableDisplay();
        }
    }
    while (!success);
}

/*
* Function to create a 1 second delay
* @param gameClock -> clock to track game time
*/
void delaySec(Clock gameClock)
{
    float currTime = gameClock.getElapsedTime().asMilliseconds();
    while (gameClock.getElapsedTime().asMilliseconds() - currTime < 1000)
    {
        continue;
    }
}

/*
* Function to reset settings to go to next level
* Resets all the in game variables and flags used to track the level
* @param mainBuzzIcon -> game icon for buzz
* @param insectIcon -> game icon for the flying insect
* @param gameClock -> clock to track game time
* @param powerBarIcon -> power bar object
* @param listGameIcons -> list of in game right side icons
* @param isInGame -> flag to denote whether player is playing game
* @param isGameOver -> flag to denote whether the game is over
* @param hasGameOverSoundPlayed -> flag to denote whether game over sound was already played
* @param didBuzzHitItem -> flag to denote whether buzz has hit something valid
* @param hasLevelCompletd -> flag to denote whether the level has completed
* @param isValidHit -> flag to denote buzz has hit something valid (insect, unicorn, mascot)
* @param isInsectHit -> flag to denote whether the insect has been hit in level
* @param isEvilMascotHit -> tracker for number of evil mascots hit
* @param iconFallingCounter -> tracker to track number of right side icons falling before reset
*/
void resetLevelGameSettings(
    // Game icons and clock
    GameIcon& mainBuzzIcon, GameIcon& insectIcon, Clock& gameClock,
    PowerBar& powerBarIcon,
    // List of game icons
    std::list<GameIcon*>& listGameIcons,
    // All flags
    bool& isInGame, bool& isGameOver, bool& hasGameOverSoundPlayed,
    bool& didBuzzHitItem, bool& hasLevelCompleted, bool& isValidHit,
    bool& isInsectHit,
    // All in game counters
    int& isEvilMascotHit, int& iconFallingCounter)
{
    // Add delay so that the dual functionality of return does not overlap
    delaySec(gameClock);
    randomizeGridIcon(listGameIcons);
    isEvilMascotHit = 0;
    isGameOver = false;
    isInGame = true;
    powerBarIcon.resetPower();
    didBuzzHitItem = false;
    hasLevelCompleted = false;
    isValidHit = false;
    mainBuzzIcon.resetFalling();
    mainBuzzIcon.enableDisplay();
    iconFallingCounter = 0;
    hasGameOverSoundPlayed = false;
    insectIcon.enableDisplay();
    isInsectHit = false;
}

/*
* Function to reset all in game variables
* Reset all the in game vairables and flags
* @param mainBuzzIcon -> game icon for buzz
* @param insectIcon -> game icon for the flying insect
* @param gameClock -> clock to track game time
* @param powerBarIcon -> power bar object
* @param listGameIcons -> list of in game right side icons
* @param isInGame -> flag to denote whether player is playing game
* @param isGameOver -> flag to denote whether the game is over
* @param hasGameOverSoundPlayed -> flag to denote whether game over sound was already played
* @param didBuzzHitItem -> flag to denote whether buzz has hit something valid
* @param hasLevelCompletd -> flag to denote whether the level has completed
* @param isValidHit -> flag to denote buzz has hit something valid (insect, unicorn, mascot)
* @param isInsectHit -> flag to denote whether the insect has been hit in level
* @param isEvilMascotHit -> tracker for number of evil mascots hit
* @param iconFallingCounter -> tracker to track number of right side icons falling before reset
* @param pointScoredCounter -> tracker for number of points scored
* @param currLifeCount -> tracker for number of lives in game play
* @param lifeCount -> max life count set for game
*/
void resetAllGameSetting(
    // Game icons and clock
    GameIcon& mainBuzzIcon, GameIcon& insectIcon, Clock& gameClock,
    PowerBar& powerBarIcon,
    // List of game icons
    std::list<GameIcon*>& listGameIcons,
    // All flags
    bool& isInGame, bool& isGameOver, bool& hasGameOverSoundPlayed,
    bool& didBuzzHitItem, bool& hasLevelCompleted, bool& isValidHit,
    bool& isInsectHit,
    // All in game counters
    int& isEvilMascotHit, int& iconFallingCounter, int& pointScoredCounter, int& currLifeCount,
    const int& lifeCount)
{
    // Reset all in game settings
    currLifeCount = lifeCount;
    pointScoredCounter = 0;
    // Reset other level based settings
    resetLevelGameSettings(mainBuzzIcon, insectIcon, gameClock, powerBarIcon,
        listGameIcons, isInGame, isGameOver, hasGameOverSoundPlayed, didBuzzHitItem,
        hasLevelCompleted, isValidHit, isInsectHit, isEvilMascotHit, iconFallingCounter);
}

int main()
{
    /*
    ****************************************
    Game settings
    ****************************************
    */

    //parentLoc = "C:/Users/varun/OneDrive - Georgia Institute of Technology/ECE_6122_APT/Labs/Lab3/sfml_support/";
    parentLoc = "";
    windowHeigth = 1080;
    windowWidth = 1920;

    // Total number of lives in game
    int lifeCount = 5;
    // Current number of lives left in game
    int currLifeCount = lifeCount;

    // Initial Position of main player
    int xStartPos = 90;
    int yStartPos = windowHeigth / 2;

    // Initial Position of flying insect on screen
    int xFlyingBeePos = 1000;
    int yFlyingBeePos = 100;

    // Main player launch angles
    float defaultLaunchAngle = -20;
    float currentLaunchAngle = defaultLaunchAngle;

    // Buzz velocity scaling => used for debugging
    float buzzVelocityScaling = 1 * displaySpeedScaling/2;

    // Rate of rotation of launch angle (kept low for better GUI display)
    float rateAngleRot = 0.05 * displaySpeedScaling/2;

    // Limits of bee motion along y axis
    float beeLocYMin = 25;
    float beeLocYMax = 200;

    // Set in-game gravity value such that if shot straight up, buzz can land back within 5 seconds
    float gravity = -0.0005 * displaySpeedScaling;

    // Set rate of fall when items start falling
    float rotRate = 0.05 * displaySpeedScaling; // Rate of rotation
    float fallRate = 0.05 * displaySpeedScaling; // Rate of fall in y axis

    // Amount of buffer around local bounds to check for hit
    float boundsBuffer = 0;

    /*
    ****************************************
    Set up graphic settings
    ****************************************
    */

    // Video mode object
    VideoMode vm(windowWidth, windowHeigth);

    // Render the window
    RenderWindow window(vm, "Buzzy's Revenge Game", Style::Default);

    // Disabled repeated release events
    window.setKeyRepeatEnabled(false);

    /*
    ****************************************
    Set up background
    ****************************************
    */

    // Create texture background and load image
    Texture textureBackground;
    textureBackground.loadFromFile(parentLoc + "./graphics/background.png");

    // Create background sprite and attach to background
    Sprite spriteBackground;
    spriteBackground.setTexture(textureBackground);
    spriteBackground.setPosition(0, 0);

    /*
    ****************************************
    Set up the home screen text
    ****************************************
    */
    // List to track all the text added to display
    // the text on refresh in minimal code
    std::list<DisplayText*> listDisplayText;

    // Relative displacement of centre text from vertical centre
    int relativeDisplacement = -3;
    // Add the game title
    int textSize = 60;
    DisplayText* textTitle = new DisplayText(Color::Red, textSize);
    textTitle->addTextCentreAlign("Buzzy's Revenge", -3 * textSize);
    listDisplayText.push_back(textTitle);
    ++relativeDisplacement;

    // Add the in game instructions
    std::string homeScreenText[] = {
    "Press Enter to Restart Game",
    "Press Esc to exit",
    "Press Space to Powerup",
    "Press up/down arrow to aim" };
    textSize = 40;
    for (std::string currLine : homeScreenText)
    {
        DisplayText* textLine = new DisplayText(Color::White, textSize);
        textLine->addTextCentreAlign(currLine, relativeDisplacement * (textSize + 10));
        listDisplayText.push_back(textLine);
        // Increment the relative displacement to move the next centre
        // text down to avoid overlap
        ++relativeDisplacement;
    }

    // Add the creator name on screen
    textSize = 20;
    DisplayText* textCreator = new DisplayText(Color::White, textSize);
    textCreator->addTextCentreAlign("   created by\n  ROBOT", (relativeDisplacement + 8) * textSize);
    listDisplayText.push_back(textCreator);
    ++relativeDisplacement;

    std::list<DisplayText*> listInGameText;

    // Add Lives text
    textSize = 20;
    DisplayText* textLive = new DisplayText(Color::White, textSize);
    textLive->addTextNearEdge("Lives", topLeft);
    listInGameText.push_back(textLive);

    // Add Scores text
    DisplayText* textScore = new DisplayText(Color::White, textSize);
    textScore->addTextNearEdge("Score: --", topRight);
    listInGameText.push_back(textScore);

    // Add Power text
    DisplayText* textPower = new DisplayText(Color::White, textSize);
    textPower->addTextNearEdge("Power", bottomLeft);
    listInGameText.push_back(textPower);

    // Add Game Over text
    DisplayText textGameOver(Color::Red, 60);
    textGameOver.addTextCentreAlign("Game Over!", (relativeDisplacement+3)*textSize);

    /*
    ****************************************
    Set up the home screen icons
    ****************************************
    */

    // Add the power bar
    PowerBar powerBarIcon(
        textPower->getRightEdge(), textPower->getCentreHeight(),
        buzzVelocityScaling);

    // List of icons added to made display easier
    std::list<GameIcon*> listGameIcons;

    // Keep the evil mascots near the end to make randomize function faster
    std::string madUnicorn = "angry_unicorn.png";
    std::string evilMascot1 = "tiger.png";
    std::string evilMascot2 = "Georgia_Bulldogs_logo_dog_g.png";
    std::array<std::string, 10> iconToAdd =
    { evilMascot1, evilMascot2, "bunny.png",
        "chicken.png", madUnicorn, "dog.png",
        "frog.png", "mouse.png",
        "pig.png", "sheep.png" };

    // Add the game play characters
    for (int i = 0; i < iconToAdd.size(); ++i)
    {
        GameIcon* icon = new GameIcon(iconToAdd[i]);
        icon->placeIconGrid(i);
        // Check for evil mascots and set
        if (iconToAdd[i].compare(evilMascot1) == 0 ||
            iconToAdd[i].compare(evilMascot2) == 0)
        {
            icon->setEvilMascot();
        }
        // Check if it is mad unicorn
        else if (iconToAdd[i].compare(madUnicorn) == 0)
        {
            icon->setMadUnicorn();
        }
        // else set as woodland animal
        else
        {
            icon->setWoodlandAnimal();
        }
        listGameIcons.push_back(icon);
    }

    // Randomize game icons
    randomizeGridIcon(listGameIcons);

    // List of lives icons to display
    std::list<GameIcon*> listLifeIcons;

    // Add the lives bar
    for (int i = 0; i < lifeCount; ++i)
    {
        GameIcon* icon = new GameIcon("buzzy\ life.png", 40);
        icon->placeIconCustom(textLive->getRightEdge() + (i)*40, textLive->getTopEdge());
        listLifeIcons.push_back(icon);
    }

    // Add the main player and no scaling to be done on it
    GameIcon mainBuzzIcon("smallbuzzy.png", -1, true);
    mainBuzzIcon.placeIconCustom(xStartPos, yStartPos);
    // Rotate the main player icon into launch position
    mainBuzzIcon.setRotation(defaultLaunchAngle);
    // Set the mainplayer sprite origin to its midpoints for better visual rotation
    mainBuzzIcon.setOriginToMidPoint();

    // add the flying bee
    GameIcon insectIcon("insect.png", -1, true);
    insectIcon.placeIconCustom(xFlyingBeePos, yFlyingBeePos);
    // Flip bee to match homework PDF homescreen
    insectIcon.flip();
    // Add the buffer when checking for intersection
    insectIcon.setIntersectionCheckBuffer(boundsBuffer);

    /*
    ****************************************
    Set up game sounds
    ****************************************
    */

    // Sound played when wrong icon hit
    SoundBuffer deathBuffer;
    deathBuffer.loadFromFile(parentLoc + "sound/jump.wav");
    Sound deathSound;
    deathSound.setBuffer(deathBuffer);

    // Sound played when buzz launched
    SoundBuffer launchBuffer;
    launchBuffer.loadFromFile(parentLoc + "sound/pickup.wav");
    Sound launchSound;
    launchSound.setBuffer(launchBuffer);

    // Sound played when unicorn hit
    SoundBuffer powerupBuffer;
    powerupBuffer.loadFromFile(parentLoc + "sound/powerup.wav");
    Sound powerupSound;
    powerupSound.setBuffer(powerupBuffer);

    // Sound played when mascot hit
    SoundBuffer goodHitBuffer;
    goodHitBuffer.loadFromFile(parentLoc + "sound/out_of_time.wav");
    Sound goodHitSound;
    goodHitSound.setBuffer(goodHitBuffer);

    // Sound played when grid icons layout refreshed
    SoundBuffer layoutResetBuffer;
    layoutResetBuffer.loadFromFile(parentLoc + "sound/reachgoal.wav");
    Sound layoutResetSound;
    layoutResetSound.setBuffer(layoutResetBuffer);

    // Sound played when game over
    SoundBuffer gameOverBuffer;
    gameOverBuffer.loadFromFile(parentLoc + "sound/death.wav");
    Sound gameOverSound;
    gameOverSound.setBuffer(gameOverBuffer);

    /*
    ****************************************
    Start the game clock
    ****************************************
    */
    Clock gameClock;

    /*
    ****************************************
    Functionality based vairablea
    ****************************************
    */

    // flag to denote buzz is in flight
    bool isBuzzInFlight = false;
    // flag to track buzz velocity at launch
    float buzzVelocity = -1;
    // Tracker for time when buzz projectile begins
    float launchTime = 0;
    // Tracker for the x and y location of the projectile launch
    float xLaunch = 0;
    float yLaunch = 0;
    // Flag to denote game gover
    bool isGameOver = false;
    // Flag to denote in game => remove intro text
    bool isInGame = false;
    // Points tracker
    int pointScoredCounter = 0;
    // Tracker for mascots hit
    int isEvilMascotHit = 0;
    // Tracker to check whether buzz hit an item in the current flight
    bool didBuzzHitItem = false;
    // Tracker to check if level completed
    bool hasLevelCompleted = false;
    // Tracker to check if the hit was valid => do not loose life
    bool isValidHit = false;
    // Flag to make buzz fall vertically on hitting item
    mainBuzzIcon.resetFalling();
    // Tracker to keep to track of icons falling
    int iconFallingCounter = 0;
    // Flag to track if game over sound player
    bool hasGameOverSoundPlayed = false;
    // Flag to track whether the flying insect is hit
    bool isInsectHit = false;
    // Flag to track spacebar key pressed then release
    bool wasSpaceBarHit = false;

    resetAllGameSetting(mainBuzzIcon, insectIcon, gameClock, powerBarIcon,
        listGameIcons, isInGame, isGameOver, hasGameOverSoundPlayed, didBuzzHitItem,
        hasLevelCompleted, isValidHit, isInsectHit, isEvilMascotHit, iconFallingCounter,
        pointScoredCounter, currLifeCount, lifeCount);

    // Set in game to false to display home screen
    isInGame = false;

    while (window.isOpen())
    {
        // Tracking event
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        /*
        ****************************************
        check Player inputs
        ****************************************
        */
        // Exit on esc press
        if (Keyboard::isKeyPressed(Keyboard::Escape))
        {
            window.close();
        }
        // Launch buzz on enter press
        if (Keyboard::isKeyPressed(Keyboard::Return))
        {
            // when not ingame => set in game flag and remove intro text
            // when game over flag true then also same action to restart game
            if (!isInGame)
            {
                // reset game settings
                resetAllGameSetting(mainBuzzIcon, insectIcon, gameClock, powerBarIcon,
                    listGameIcons, isInGame, isGameOver, hasGameOverSoundPlayed, didBuzzHitItem,
                    hasLevelCompleted, isValidHit, isInsectHit, isEvilMascotHit, iconFallingCounter,
                    pointScoredCounter, currLifeCount, lifeCount);

                // Play layout reset sound
                layoutResetSound.play();

            }
        }

        // Disable play functionality when not in game and when buzz in flight
        if (isInGame && !isBuzzInFlight)
        {
            // Power up on space press
            if (Keyboard::isKeyPressed(Keyboard::Space))
            {
                powerBarIcon.increasePower(displaySpeedScaling);
                wasSpaceBarHit = true;
            }
            if (wasSpaceBarHit && event.type == Event::KeyReleased && event.key.code == Keyboard::Space)
            {
                wasSpaceBarHit = false;
                // Launch allowed only when buzz not already in flight
                if (!isBuzzInFlight)
                {
                    // Launch buzz
                    isBuzzInFlight = true;
                    buzzVelocity = powerBarIcon.getVelocity();

                    // Record the current time
                    launchTime = gameClock.getElapsedTime().asMilliseconds();

                    // Store the launch coordinates
                    xLaunch = mainBuzzIcon.getXPos();
                    yLaunch = mainBuzzIcon.getYPos();

                    // Reset the buzz hit item flag
                    didBuzzHitItem = false;

                    // Play launch sound
                    launchSound.play();
                }
            }
            // Up arrow to increase launch angle (=> reduce rot angle as inverted coordinates)
            if (Keyboard::isKeyPressed(Keyboard::Up))
            {
                if (currentLaunchAngle > -90)
                {
                    currentLaunchAngle -= rateAngleRot;
                    mainBuzzIcon.setRotation(currentLaunchAngle);
                }
            }
            // Down arrow to decrease launch angle (=> Increase rot angle as inverted coordinates)
            if (Keyboard::isKeyPressed(Keyboard::Down))
            {
                if (currentLaunchAngle < 90)
                {
                    currentLaunchAngle += rateAngleRot;
                    mainBuzzIcon.setRotation(currentLaunchAngle);
                }
            }
        }

        /*
        ****************************************
        Game functionality
        ****************************************
        */

        // When buzz in flight
        if (isBuzzInFlight == true)
        {
            // check the type of motion buzz is in
            if (mainBuzzIcon.getIsFalling() == false)
            {
                // => Buzz is in projectile
                // find new position and angle for icon
                mainBuzzIcon.motionProjectile(
                    gameClock.getElapsedTime().asMilliseconds() - launchTime,
                    buzzVelocity, currentLaunchAngle, gravity, xLaunch,
                    yLaunch);
            }
            else
            {
                // => Buzz has hit icon and is falling
                mainBuzzIcon.motionFallAndRotate(rotRate, fallRate);
            }

            // check for hit only if buzz not already hit another item in projectile
            if (didBuzzHitItem == false)
            {
                // Add logic to hold data to changed the drawing based on
                // what was hit outside the for checking loop below
                // so that it does not dynamically update the grid position
                // it is working on
                bool hitUnicorn = false;
                bool hitEvilMascot = false;
                int rowHit = -1, colHit = -1; // to track the row/col of item hit in grid
                // Check if buzz new location hits game icons
                for (auto gameIcon : listGameIcons)
                {
                    if (gameIcon->isDisplayed())
                    {
                        if (gameIcon->checkWithinBounds(mainBuzzIcon.getXRightMost(),
                            mainBuzzIcon.getYMidPoint()))
                        {
                            // check if evil mascot hit
                            if (gameIcon->getIsEvilMascot())
                            {
                                hitEvilMascot = true;
                                colHit = gameIcon->getGridCol();
                                rowHit = gameIcon->getGirdRow();
                                // increase score
                                pointScoredCounter += 25;

                                // Remove all game icons from same column
                                for (auto innerGameIcon : listGameIcons)
                                {
                                    if (innerGameIcon->getGridCol() ==
                                        gameIcon->getGridCol())
                                    {
                                        innerGameIcon->setFalling();
                                        ++iconFallingCounter;
                                    }
                                }
                                // Increment the evil mascot hit tracker
                                ++isEvilMascotHit;

                                // Set hit as valid
                                isValidHit = true;

                                // Play good hit sound
                                goodHitSound.play();
                            }
                            // check if mad unicorn hit
                            else if (gameIcon->getIsMaxUnicorn())
                            {
                                hitUnicorn = true;
                                colHit = gameIcon->getGridCol();
                                rowHit = gameIcon->getGirdRow();
                                // increase the life if possible
                                if (currLifeCount < lifeCount)
                                {
                                    ++currLifeCount;
                                }

                                gameIcon->setFalling();
                                ++iconFallingCounter;

                                // Set hit as valid
                                isValidHit = true;

                                // Play powerup sound
                                powerupSound.play();
                            }
                            else
                            {
                                // hit normal icon
                                // Set the hit icon to falling
                                 
                                // NOTE: Removing below 2 lines which were used to
                                // remove the woodland creatures from the game when
                                // hit while buzzy looses 1 life
                                //gameIcon->setFalling();
                                //++iconFallingCounter;
                                
                                // hit was invalid
                                isValidHit = false;

                                // Run the death sounds
                                deathSound.play();
                            }

                            // set buzz hit item to true
                            didBuzzHitItem = true;
                            // set to buzz falling to change motion type
                            mainBuzzIcon.setFalling();
                        }
                    }
                }

                // Check if buzz hit the insect and it was not hit previously in level
                if (isInsectHit == false &&
                    insectIcon.checkWithinBounds(mainBuzzIcon.getXPos(), mainBuzzIcon.getYPos()))
                {
                    insectIcon.disableDisplay();
                    isInsectHit = true;
                    pointScoredCounter += 75;
                    isValidHit = true;
                }

                // Change the drawing if needed based on what was hit
                if (hitUnicorn)
                {
                    // Drop the icons above it in the same column by 1 row
                    for (auto innerGameIcon : listGameIcons)
                    {
                        if (
                            // check if in the same column
                            colHit == innerGameIcon->getGridCol() &&
                            // check if in a row higher than unicorn
                            rowHit > innerGameIcon->getGirdRow())
                        {
                            // increase row by 1
                            int newGridPosition = (innerGameIcon->getGirdRow() + 1)*2 + colHit;
                            innerGameIcon->placeIconGrid(newGridPosition);
                        }
                    }
                }
            }

            // Make the icons fall if set to fallings
            for (auto gameIcon : listGameIcons)
            {
                if (gameIcon->getIsFalling())
                {
                    gameIcon->motionFallAndRotate(rotRate, fallRate);
                    if (not gameIcon->isDisplayed())
                    {
                        // if not still displayed
                        --iconFallingCounter;
                        // reset falling to stop executing the block again
                        gameIcon->resetFalling();
                    }
                }
            }

            // if both icons hit
            if (isEvilMascotHit == 2)
            {
                hasLevelCompleted = true;
            }

            if (// don't reset buzz till all falling icons out of window
                iconFallingCounter == 0 &&
                checkAndResetBuzz(mainBuzzIcon, xStartPos, yStartPos, defaultLaunchAngle))
            {
                isBuzzInFlight = false;
                currentLaunchAngle = defaultLaunchAngle;
                // Reset buzz motion type and display
                mainBuzzIcon.resetFalling();
                mainBuzzIcon.enableDisplay();
                // Reset the power
                powerBarIcon.resetPower();
                // Check if anything valid was hit
                if (isValidHit == false)
                {
                    --currLifeCount;
                }
                // Reset the valid hit tracker
                isValidHit = false;
                // Check if level completed
                if (hasLevelCompleted)
                {
                    // Reset the level data
                    resetLevelGameSettings(mainBuzzIcon, insectIcon, gameClock, powerBarIcon,
                        listGameIcons, isInGame, isGameOver, hasGameOverSoundPlayed, didBuzzHitItem,
                        hasLevelCompleted, isValidHit, isInsectHit, isEvilMascotHit, iconFallingCounter);

                    // Play level up sound
                    layoutResetSound.play();
                }
            }
        }
        if (currLifeCount == 0)
        {
            isGameOver = true;
            isInGame = false;
        }

        // Fly the insect around
        //flyInsect(insectIcon, gameClock, insectSpeed);
        insectIcon.motionRandomHorizontal(beeLocYMin, beeLocYMax);

        // Update the score before drawing
        textScore->addText("Score: " + std::to_string(pointScoredCounter));

        // Draw the scene
        window.clear();

        // Add the background
        window.draw(spriteBackground);

        if (!isInGame)
        {
            // Add message text
            for (auto textObj : listDisplayText)
            {
                textObj->draw(window);
            }

            if (isGameOver)
            {
                // Add game over text
                textGameOver.draw(window);
                
                if (hasGameOverSoundPlayed == false)
                {
                    // Play game over sound
                    gameOverSound.play();
                    // Set flag to true to not play sound again till next game over
                    hasGameOverSoundPlayed = true;
                }
            }
        }

        // Add in game text
        for (auto textObj : listInGameText)
        {
            textObj->draw(window);
        }

        // Add power bar
        powerBarIcon.draw(window);

        // Add the game icons
        for (auto iconObj : listGameIcons)
        {
            iconObj->draw(window);
        }

        // Add lives icons based on lives left
        std::list<GameIcon*>::iterator itr = listLifeIcons.begin();
        for (int i = 0; i < currLifeCount; ++i)
        {
            (*itr)->draw(window);
            ++itr;
        }

        // Add main player buzz
        mainBuzzIcon.draw(window);

        // Add flying bee icon
        insectIcon.draw(window);

        // Display the drawing
        window.display();
    }

    // Delete the objects
    for (auto textObj : listDisplayText)
    {
        delete textObj;
    }
    for (auto iconObj : listGameIcons)
    {
        delete iconObj;
    }
    for (auto lifeObj : listLifeIcons)
    {
        delete lifeObj;
    }

    return 0;
}
