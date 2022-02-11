
#ifndef  __GAMEICON_HEADER__
#define __GAMEICON_HEADER__

// SFML includes
#include <SFML/Graphics.hpp>

using namespace sf;

/* 
* Globals used
*/
extern float windowWidth;
extern float windowHeigth;
extern std::string parentLoc;

/*
* Class for game icons
* 
* Logic of grid position to row column mapping
* 
* Grid -> 0 => (row = 0, col = 0)
* Grid -> 1 => (row = 0, col = 1)
* Grid -> 2 => (row = 1, col = 0) so on
* 
* => Row = gridPosition / 2
* => Col = gridPosition % 2;
* 
* Grid Position range: 0 - 9
* 
*/
class GameIcon
{
    Texture textureIcon; // icon texture
    Sprite spriteIcon; // icon sprite
    int iconPosition; // iconPosition in row-column combination if grid based positioning
    int iconSize; // size to be kept for each icon
    float xPos; // x position of icon
    float yPos; // y position of incon
    bool isEvilMascot; // whether the icon is of an evil mascot
    bool isWoodlandAnimal; // whether the icon is of a woodland animal
    bool isMadUnicorn; // whether the icon is of the mad unicorn
    bool displayIcon; // flag to decide whether to display icon or not
    bool isFalling; // flag to denote whether the icon is falling
    float intersectionCheckBuffer; // add buffer around global bounds to increase area checked for intersction
public:

    /*
    * Constructor for class
    * @param inFile -> name of file to read icon
    * @param (Optional) inIconSize -> size of target icon
    * @param (Optional) noScale -> if no scaling needs to be done
    */
    GameIcon(std::string inFile, int inIconSize = 130, bool noScale = false);

    /*
    * Setter for intersection buffer
    * @param inBufferSize -> amount of buffer area to add around icon when checking for intersection
    */
    void setIntersectionCheckBuffer(float inBufferSize);

    /*
    * Function to place icon at custom location
    * @param inXPos -> x position of icon
    * @param inYPos -> y position of icon
    */
    void placeIconCustom(float inXPos, float inYPos);

    /*
    * Function to place icon based on icon position
    *
    * => row = position/2, col = position%2
    *
    * @param inIconPositionGrid -> position of icon in grid
    */
    void placeIconGrid(int inIconPositionGrid);

    /*
    * Function to draw the icon
    * @param window -> window to draw on
    */
    void draw(RenderWindow& window);

    /*
    * Function to flip image horizontally
    */
    void flip();

    /*
    * Function to rotate image
    * @param angleRot -> angle of rotation
    */
    void setRotation(float angleRot);

    /*
    * Function to get the current rotation angle of the icon
    * @return float -> current rotation angle
    */
    float getRotation();

    /*
    * Getter for xpos
    * @return float -> get x postion
    */
    float getXPos();

    /*
    * Getter for ypos
    * @return float -> get y position
    */
    float getYPos();

    /*
    * Getter for x right in global bounds
    * @return float -> get the right most x point in icon
    */
    float getXRightMost();

    /*
    * Getter for mid point along y
    * @return float -> get the y mid point for icon
    */
    float getYMidPoint();

    /*
    * Function to get row position in grid
    * @return int -> row of icon in grid
    */
    int getGirdRow();

    /*
    * Function to get col position in grid
    * @return int -> column of icon in grid
    */
    int getGridCol();

    /*
    * Setter for evilMasot
    */
    void setEvilMascot();

    /*
    * Getter for evilMascot
    * @return bool -> is evil mascot
    */
    bool getIsEvilMascot();

    /*
    * Setter for madUnicorn
    */
    void setMadUnicorn();

    /*
    * Getter for evilMascot
    * @return bool -> is madunicorn
    */
    bool getIsMaxUnicorn();

    /*
    * Setter for woodlandAnimal
    */
    void setWoodlandAnimal();

    /*
    * Getter for evilMascot
    * @return bool -> is woodland animal
    */
    bool getIsWoodlandAnimal();

    /*
    * Function to disable display
    */
    void disableDisplay();

    /*
    * Function to enable display
    */
    void enableDisplay();

    /*
    * Function to get if icon displayed in screen
    * @return bool -> is the icon is still displayed
    */
    bool isDisplayed();

    /*
    * Function to check if input coordinates are within the
    * icon bounds
    * @param inXPos -> x coordinate to check
    * @param inYPos -> y coordinate to check
    * @return bool -> if the xy position is within icon bounds
    */
    bool checkWithinBounds(float inXPos, float inYPos);

    /*
    * Function to set the icon as falling
    */
    void setFalling();

    /*
    * Function to reset the icon falling flag
    */
    void resetFalling();

    /*
    * Function to get if the icon is falling
    * @return bool -> is the icon in falling motion
    */
    bool getIsFalling();

    /*
    * Function to set icon into motion
    * Type -> fall and rotate
    * @param rotRate -> rate of rotation
    * @param fallRate -> rate of fall in y direction
    */
    void motionFallAndRotate(float rotRate, float fallRate);

    /*
    * Function to set icon into motion
    * Type -> random up/down within range and right to left
    * @param ymin -> min value of y in random movement
    * @param ymax -> max value of y in random movement
    */
    void motionRandomHorizontal(float ymin, float ymax);

    /*
    * Function to set icon into motion
    * Type -> projectile motion taking velocit of launch, gravity
    * and launch position into consideration
    * @param timeElapsed -> time since launch
    * @param velocity -> velociy of buzz launch
    * @param angle -> angle of buzz launch
    * @param gravity -> in game gravity to use
    * @param xLaunch -> x coordinate of buzz when launched
    * @param yLaunch -> y coordinate of buzz when launched
    */
    void motionProjectile(float timeElapsed, float velocity,
        float angle, float gravity, float xLaunch, float yLaunch);

    /*
    * Function to check if icon is out of screen bounds
    * If so => disable display
    */
    void checkOutOfScreenBounds();

    /*
    * Function to change the origin of the icon
    */
    void setOriginToMidPoint();

};

#endif // ! __GAMEICON_HEADER__

