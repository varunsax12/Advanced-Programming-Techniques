
#include "PowerBar.h"

/*
* Constructor to create the rectange
* @param inXPos -> x position of bar of leftcentre
* @param inYPos -> y position of bar of leftcentre
* @param inVelocityScale -> increase the velocity by factor for debugging
*/
PowerBar::PowerBar(int inXPos, int inYPos, float inVelocityScale) :
    powerMeterVal(1), rectHeight(50.f), maxPower(500), steps(100),
    velocityScale(inVelocityScale)
{
    // centre align based on x and y pos being leftcentre
    rectange.setPosition(Vector2f(inXPos, inYPos - this->rectHeight / 2));
    rectange.setFillColor(Color::Red);
    resizeRect();

    // create the oultine for the rectangle
    outline.setPosition(Vector2f(inXPos, inYPos - this->rectHeight / 2));
    this->outline.setSize(Vector2f(this->maxPower, this->rectHeight));
    outline.setFillColor(Color::Transparent);
    outline.setOutlineColor(Color::Black);
    outline.setOutlineThickness(10);
}

/*
* Function to increase power
* Rate of power increase kept to 1
* @param displaySpeedScaling -> Scale used to increase speed when space is pressed
*/
void PowerBar::increasePower(int displaySpeedScaling = 1)
{
    if (this->powerMeterVal < this->maxPower)
    {
        this->powerMeterVal += 0.1 * displaySpeedScaling;
        resizeRect();
    }
}

/*
* Function to reset the power
*/
void PowerBar::resetPower()
{
    this->powerMeterVal = 1;
    resizeRect();
}

/*
* Function to resize power rectange
*/
void PowerBar::resizeRect()
{
    this->rectange.setSize(Vector2f(this->powerMeterVal, this->rectHeight));
}

/*
* Function to draw rectangle in window
* @param window -> window to display on
*/
void PowerBar::draw(RenderWindow& window)
{
    window.draw(this->rectange);
    window.draw(this->outline);
}

/*
* Function to get buzz velocity
* Added speed scaling of 100
* Scale by factor used for debugging
* Added velocity increment of 1 for better visual display to not have 0 velocity
* @return float -> velocity as per power bar
*/
float PowerBar::getVelocity()
{
    return 1 + (this->powerMeterVal / this->maxPower) * this->velocityScale;
}