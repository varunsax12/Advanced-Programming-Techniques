
#ifndef  __POWERBAR_HEADER__
#define __POWERBAR_HEADER__


// SFML includes
#include <SFML/Graphics.hpp>

using namespace sf;

/*
* Class to represent the power bar for buzz launch
*/
class PowerBar
{
    float powerMeterVal; // current value of power
    RectangleShape rectange; // rect object
    RectangleShape outline; // outline of power bar
    float rectHeight; // rectange heigth is fixed
    int maxPower; // max power for the power bar
    int steps;
    float velocityScale; // used to scale velocity to directly project buzz
public:
    /*
    * Constructor to create the rectange
    * @param inXPos -> x position of bar of leftcentre
    * @param inYPos -> y position of bar of leftcentre
    * @param inVelocityScale -> increase the velocity by factor for debugging
    */
    PowerBar(int inXPos, int inYPos, float inVelocityScale);

    /*
    * Function to increase power
    * Rate of power increase kept to 1
    * @param displaySpeedScaling -> Scale used to increase speed when space is pressed
    */
    void increasePower(int displaySpeedScaling);

    /*
    * Function to reset the power
    */
    void resetPower();

    /*
    * Function to resize power rectange
    */
    void resizeRect();

    /*
    * Function to draw rectangle in window
    * @param window -> window to display on
    */
    void draw(RenderWindow& window);

    /*
    * Function to get buzz velocity
    * Added speed scaling of 100
    * Scale by factor used for debugging
    * Added velocity increment of 1 for better visual display to not have 0 velocity
    * @return float -> velocity as per power bar
    */
    float getVelocity();
};

#endif // ! __POWERBAR_HEADER__

