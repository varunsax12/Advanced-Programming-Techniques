
#ifndef __DISPLAYTEXT_HEADER__
#define __DISPLAYTEXT_HEADER__

// SFML includes
#include <SFML/Graphics.hpp>

using namespace sf;

/*
* Global Used
*/
extern float windowWidth;
extern float windowHeigth;
extern std::string parentLoc;

// Enum of edge to place text near
enum edgeLoc
{
    topLeft,    // Place close to top left corner
    topRight,   // Place close to top right corner
    bottomLeft, // Place close to bottom left corner
    bottomRight // Place close to bottom right corner
};

/*
* Class for text displayed in the game screen
*/
class DisplayText
{
    // Text object
    Text text;
    // Font object
    Font font;

    //Location of text
    float xPos;
    float yPos;

public:

    /*
    * Constructor for class
    * @param inTextColor -> color of text
    * @param inSize -> size of text
    */
    DisplayText(const Color inTextColor, const int inSize);

    /*
    * Function to add text to display
    * @param inStr -> string to display
    */
    void addText(std::string inStr);

    /*
    * Function to add text to object and centre align text
    * @param inStr -> string to display
    * @param yDisplacement -> relative displacement of text in y direction
    */
    void addTextCentreAlign(std::string inStr, int yDisplacement);

    /*
    * Function to add text to object and align near edge
    * @param inStr -> string to display
    * @param edge -> string of edge of place near
    */
    void addTextNearEdge(std::string inStr, edgeLoc edge);

    /*
    * Function to draw text in window
    * @param window -> window to draw on
    */
    void draw(RenderWindow& window);

    /*
    * Function to get the right position
    * @return float -> right edge of displayed text
    */
    float getRightEdge();

    /*
    * Function to get the origin heigth
    * @return float -> y postiion of display text
    */
    float getCentreHeight();

    /*
    * Function to get the top position
    * @return float -> top edge of display text
    */
    float getTopEdge();
};

#endif // !__DISPLAYTEXT_HEADER__

