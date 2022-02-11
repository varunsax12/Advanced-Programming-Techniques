
#include <cmath>
#include <random>

#include "GameIcon.h"

// Value of PI used in projectile calculations
#define PI 3.14159265

/*
* Constructor for class
* @param inFile -> name of file to read icon
* @param (Optional) inIconSize -> size of target icon
* @param (Optional) noScale -> if no scaling needs to be done
*/
GameIcon::GameIcon(std::string inFile, int inIconSize, bool noScale) :
    iconSize(inIconSize), iconPosition(-1), xPos(-1), yPos(-1),
    isEvilMascot(false), displayIcon(true), isMadUnicorn(false),
    isWoodlandAnimal(false), isFalling(false), intersectionCheckBuffer(0)
{
    this->textureIcon.loadFromFile(parentLoc + "graphics/" + inFile);
    this->spriteIcon.setTexture(this->textureIcon);

    if (noScale == false)
    {
        float iconWidth = this->textureIcon.getSize().x;
        float iconHeigth = this->textureIcon.getSize().y;

        // Target icon size
        spriteIcon.setScale(this->iconSize / iconWidth, this->iconSize / iconHeigth);
    }
}

/*
* Setter for intersection buffer
* @param inBufferSize -> amount of buffer area to add around icon when checking for intersection
*/
void GameIcon::setIntersectionCheckBuffer(float inBufferSize)
{
    this->intersectionCheckBuffer += inBufferSize;
}

/*
* Function to place icon at custom location
* @param inXPos -> x position of icon
* @param inYPos -> y position of icon
*/
void GameIcon::placeIconCustom(float inXPos, float inYPos)
{
    this->xPos = inXPos;
    this->yPos = inYPos;
    this->spriteIcon.setPosition(this->xPos, this->yPos);
}

/*
* Function to place icon based on icon position
* Grid -> 0 => (row = 0, col = 0)
* Grid -> 1 => (row = 0, col = 1)
* Grid -> 2 => (row = 1, col = 0)
*
* => row = position/2, col = position%2
*
* @param inIconPositionGrid -> position of icon in grid
*/
void GameIcon::placeIconGrid(int inIconPositionGrid)
{
    // Set the grid position
    this->iconPosition = inIconPositionGrid;

    int row = inIconPositionGrid / 2; // row in grid
    int col = inIconPositionGrid % 2; // column in grid

    // Take grid topleft point
    int topLeftGridX = windowWidth - iconSize * 2 - 80;
    int topLeftGridY = windowHeigth - iconSize * 5 - 150;

    this->xPos = topLeftGridX + col * (iconSize + 20);
    this->yPos = topLeftGridY + row * (iconSize + 20);

    this->spriteIcon.setPosition(this->xPos, this->yPos);
}

/*
* Function to draw the icon
* @param window -> window to draw on
*/
void GameIcon::draw(RenderWindow& window)
{
    if (this->displayIcon)
    {
        // display only if icon display enabled
        window.draw(this->spriteIcon);
    }
}

/*
* Function to flip image horizontally
*/
void GameIcon::flip()
{
    this->spriteIcon.scale(-1, 1);
}

/*
* Function to rotate image
* @param angleRot -> angle of rotation
*/
void GameIcon::setRotation(float angleRot)
{
    this->spriteIcon.setRotation(angleRot);
}

/*
* Function to get the current rotation angle of the icon
* @return float -> current rotation angle
*/
float GameIcon::getRotation()
{
    return this->spriteIcon.getRotation();
}

/*
* Getter for xpos
* @return float -> get x postion
*/
float GameIcon::getXPos()
{
    return this->xPos;
}

/*
* Getter for ypos
* @return float -> get y position
*/
float GameIcon::getYPos()
{
    return this->yPos;
}

/*
* Getter for x right in global bounds
* @return float -> get the right most x point in icon
*/
float GameIcon::getXRightMost()
{
    return this->spriteIcon.getGlobalBounds().left + this->spriteIcon.getTextureRect().width / 2;
}

/*
* Getter for mid point along y
* @return float -> get the y mid point for icon
*/
float GameIcon::getYMidPoint()
{
    return this->spriteIcon.getGlobalBounds().top + this->spriteIcon.getGlobalBounds().height / 2;
}

/*
* Function to get row position in grid
* @return int -> row of icon in grid
*/
int GameIcon::getGirdRow()
{
    return this->iconPosition / 2;
}

/*
* Function to get col position in grid
* @return int -> column of icon in grid
*/
int GameIcon::getGridCol()
{
    return this->iconPosition % 2;
}

/*
* Setter for evilMasot
*/
void GameIcon::setEvilMascot()
{
    this->isEvilMascot = true;
}

/*
* Getter for evilMascot
* @return bool -> is evil mascot
*/
bool GameIcon::getIsEvilMascot()
{
    return this->isEvilMascot;
}

/*
* Setter for madUnicorn
*/
void GameIcon::setMadUnicorn()
{
    this->isMadUnicorn = true;
}

/*
* Getter for evilMascot
* @return bool -> is madunicorn
*/
bool GameIcon::getIsMaxUnicorn()
{
    return this->isMadUnicorn;
}

/*
* Setter for woodlandAnimal
*/
void GameIcon::setWoodlandAnimal()
{
    this->isWoodlandAnimal = true;
}

/*
* Getter for evilMascot
* @return bool -> is woodland animal
*/
bool GameIcon::getIsWoodlandAnimal()
{
    return this->isWoodlandAnimal;
}

/*
* Function to disable display
*/
void GameIcon::disableDisplay()
{
    this->displayIcon = false;
}

/*
* Function to enable display
*/
void GameIcon::enableDisplay()
{
    this->displayIcon = true;
}

/*
* Functino to get if icon displayed in screen
* @return bool -> is the icon is still displayed
*/
bool GameIcon::isDisplayed()
{
    return this->displayIcon;
}

/*
* Function to check if input coordinates are within the
* icon bounds
* @param inXPos -> x coordinate to check
* @param inYPos -> y coordinate to check
* @return bool -> if the xy position is within icon bounds
*/
bool GameIcon::checkWithinBounds(float inXPos, float inYPos)
{
    float left = this->spriteIcon.getGlobalBounds().left;
    float top = this->spriteIcon.getGlobalBounds().top;
    float right = left + this->spriteIcon.getGlobalBounds().width;
    float bottom = top + this->spriteIcon.getGlobalBounds().height;

    // Add the buffer around bounds before checking intersection
    left -= this->intersectionCheckBuffer;
    right += this->intersectionCheckBuffer;
    top -= this->intersectionCheckBuffer;
    bottom += this->intersectionCheckBuffer;

    if (
        // check the x position
        inXPos >= left && inXPos <= right &&
        // check the y position
        inYPos >= top && inYPos <= bottom
        )
    {
        return true;
    }
    return false;
}

/*
* Function to set the icon as falling
* @return bool -> is the icon in falling motion
*/
void GameIcon::setFalling()
{
    this->isFalling = true;
}

/*
* Function to reset the icon falling flag
*/
void GameIcon::resetFalling()
{
    this->isFalling = false;
}

/*
* Function to get if the icon is falling
*/
bool GameIcon::getIsFalling()
{
    return this->isFalling;
}

/*
* Function to set icon into motion
* Type -> fall and rotate
* @param rotRate -> rate of rotation
* @param fallRate -> rate of fall in y direction
*/
void GameIcon::motionFallAndRotate(float rotRate, float fallRate)
{
    float currAngle = this->getRotation();
    float currYPos = this->getYPos();
    float currXPos = this->getXPos();

    currAngle += rotRate;
    currYPos += fallRate;

    this->setRotation(currAngle);
    this->placeIconCustom(currXPos, currYPos);

    // check if within bounds of screen
    this->checkOutOfScreenBounds();
}

/*
* Function to set icon into motion
* Type -> random up/down within range and right to left
* @param ymin -> min value of y in random movement
* @param ymax -> max value of y in random movement
*/
void GameIcon::motionRandomHorizontal(float ymin, float ymax)
{
    float xPos = this->getXPos();
    float yPos = this->getYPos();

    // Init random number engine
    std::random_device rd;
    std::default_random_engine randGenerator{ rd() };

    // Get the new x position
    std::uniform_int_distribution<int> randDistributionX(0, 10);
    xPos -= ((float)randDistributionX(randGenerator)) / 10;

    // Get the new y position
    std::uniform_int_distribution<int> randDistributionY(-50, 50);
    yPos += ((float)randDistributionY(randGenerator)) / 10;
    
    // Keep the bee within bounds
    // x -> 0 to windowWidth
    if (xPos < 0)
    {
        xPos = windowWidth;
    }
    // y -> 50 to 150
    if (yPos < 50)
    {
        yPos = 50;
    }
    else if (yPos > 150)
    {
        yPos = 150;
    }

    this->placeIconCustom(xPos, yPos);
}

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
void GameIcon::motionProjectile(float timeElapsed, float velocity,
    float angle, float gravity, float xLaunch, float yLaunch)
{
    // Scale time for better ingame visualization
    timeElapsed = timeElapsed;

    // Switch angle sign as coordinates opposite of cartesian in sfml
    angle = (-1) * angle;

    // Velocity components
    float vy = velocity * sin(angle * PI / 180);
    float vx = velocity * cos(angle * PI / 180);

    // New x position
    float xNew = xLaunch + vx * timeElapsed;

    // New y position
    float yNew = yLaunch - (((float)vy * timeElapsed) + 0.5 * gravity * (pow(timeElapsed, 2)));

    // New velocity components
    float vyNew = vy + (gravity * timeElapsed);
    float vxNew = vx;

    // Create the new velocity and angles
    velocity = sqrt(pow(vyNew, 2) + pow(vxNew, 2));
    angle = atan(vyNew / vxNew) * 180.0 / PI;

    // Reverse the angle sign to go back into sfml coordinates from cartersian
    angle = (-1) * angle;

    // Update the position of buzz
    this->placeIconCustom(xNew, yNew);
    this->setRotation(angle);
}

/*
* Function to check if icon is out of screen bounds
* If so => disable display
*/
void GameIcon::checkOutOfScreenBounds()
{
    // using better float comparison logic
    if (
        (isless(this->xPos, 0.0) || isgreater(this->xPos, windowWidth)) ||
        (isless(this->yPos, 0.0) || isgreater(this->yPos, windowHeigth)))
    {
        // icon is out of screen bounds => disable display
        this->disableDisplay();
    }
}

/*
* Function to change the origin of the icon
*/
void GameIcon::setOriginToMidPoint()
{
    float midPointX = this->spriteIcon.getGlobalBounds().width / 2;
    float midPointY = this->spriteIcon.getGlobalBounds().height / 2;
    this->spriteIcon.setOrigin(midPointX, midPointY);
}