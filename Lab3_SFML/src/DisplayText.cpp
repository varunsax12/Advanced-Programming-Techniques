
#include "DisplayText.h"

/*
* Constructor for class
* @param inTextColor -> color of text
* @param inSize -> size of text
*/
DisplayText::DisplayText(const Color inTextColor, const int inSize) : xPos(0), yPos(0)
{
    this->font.loadFromFile(parentLoc + "fonts/KOMIKAP_.ttf");
    this->text.setFont(this->font);
    this->text.setCharacterSize(inSize);
    this->text.setFillColor(inTextColor);
}

/*
* Function to add text to display
* @param inStr -> string to display
*/
void DisplayText::addText(std::string inStr)
{
    // Add text to object
    this->text.setString(inStr);
}

/*
* Function to add text to object and centre align text
* @param inStr -> string to display
* @param yDisplacement -> relative displacement of text in y direction
*/
void DisplayText::addTextCentreAlign(std::string inStr, int yDisplacement)
{
    // Add text
    this->addText(inStr);

    // Set the origin of text
    FloatRect textRect = this->text.getLocalBounds();
    this->text.setOrigin(textRect.left + textRect.width / 2.0f,
        textRect.top + textRect.height / 2.0f);

    // Set the position of text
    this->xPos = windowWidth / 2.0f;
    this->yPos = windowHeigth / 2.0f + yDisplacement;
    this->text.setPosition(this->xPos, this->yPos);
}

/*
* Function to add text to object and align near edge
* @param inStr -> string to display
* @param edge -> string of edge of place near
*/
void DisplayText::addTextNearEdge(std::string inStr, edgeLoc edge)
{
    // Add text
    this->addText(inStr);
    int xRelativeDisp = 40;
    int yRelativeDisp = 40;
    // Set the origin of text
    FloatRect textRect = this->text.getLocalBounds();
    this->text.setOrigin(textRect.left + textRect.width / 2.0f,
        textRect.top + textRect.height / 2.0f);

    // Set the position of text
    int xPos = 0, yPos = 0;
    switch (edge)
    {
    case topLeft:
        this->xPos = (float)xRelativeDisp;
        this->yPos = (float)yRelativeDisp;
        break;
    case topRight:
        this->xPos = windowWidth - textRect.width - xRelativeDisp;
        this->yPos = (float)yRelativeDisp;
        break;
    case bottomLeft:
        this->xPos = (float)xRelativeDisp;
        this->yPos = (float)(windowHeigth - yRelativeDisp);
        break;
    case bottomRight:
        this->xPos = windowWidth - textRect.width - xRelativeDisp;
        this->yPos = windowHeigth - yRelativeDisp;
        break;
    default:
        break;
    }
    this->text.setPosition(this->xPos, this->yPos);
}

/*
* Function to draw text in window
* @param window -> window to draw on
*/
void DisplayText::draw(RenderWindow& window)
{
    window.draw(this->text);
}

/*
* Function to get the right position
* @return float -> right edge of displayed text
*/
float DisplayText::getRightEdge()
{
    return this->xPos + float(this->text.getGlobalBounds().width);
}

/*
* Function to get the origin heigth
* @return float -> y postiion of display text
*/
float DisplayText::getCentreHeight()
{
    return (float)this->yPos;
}

/*
* Function to get the top position
* @return float -> top edge of display text
*/
float DisplayText::getTopEdge()
{
    return (float)this->text.getGlobalBounds().top;
}