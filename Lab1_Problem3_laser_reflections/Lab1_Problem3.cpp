/*
Description:
    Solution file for Problem 1 in Lab 3
    Takes an incident ray as input into an equilateral triangle
    and reflects it across the sides till the ray exits
    Prints the number of reflections inside the triangle

    Code logics used:
        Side length = 20
        ray enters at x,y = 0,0
        input argument, x intersection of the point with line parallel to x-axis => y = 20

        values known
        edges of the triangle (using trigonometry) -> (0,0), (-10, 10sqrt3), (10, 10sqrt3)
                 line 3
  (-10, 10sqrt3)-----(10, 10sqrt3)
                |   |
         line1   | |  line 2
                  |
                (0,0)

    line equations:
    line 1 => y = -sqrt3x
    line 2 => y = sqrt3x
    line 3 => y = 20

    to find perpendicular of file
        m1 * m2 = -1
    and use one point on the line to solve for y = mx + c which is where the ray intersects the side

    logic for finding the equation of reflection of line:
    reference: https://math.stackexchange.com/questions/67024/the-equation-of-a-line-reflected-about-another-line
    ax + by = c reflected over dx + ey = f
    
    y - h =   (ae^2 - ad^2 - 2bde)(x-k)/(be^2 - bd^2 + 2ade)
*/


#include <algorithm>
#include <climits>
#include <cstring>
#include <iostream>
#include <fstream>
#include <limits>
#include <string>
#include <cmath>
#include <iomanip>

/*
* Function to check whether the input argument is a number,
* if so, then set the number through reference
* else, return false
*
* @param strToCheck char array to verify
* @param inNumber reference to original variable to set
* Returns: bool if input is a number
*/
bool convertToNumber(const char* charsToCheck, long double& inNumber)
{
    // check to see if the char array is a number (including decimal point and negative sign)
    // Using lambda function to check if each char is a number
    try
    {
        bool check = std::all_of(charsToCheck, charsToCheck + strlen(charsToCheck),
            [](char c) { return ::isdigit(c) || c == '.' || c == '-'; });
        if (check)
        {
            // if the number of valid
            std::string strToCheck(charsToCheck);
            inNumber = std::stold(strToCheck);
            return true;
        }
        return false;
    }
    catch (const std::exception&)
    {
        // return invalid if exception occured
        return false;
    }
}

/*
* Class for cartesian coordinates of point
*/
class Coordinate
{
    // X and Y coordinates
    long double x;
    long double y;
public:
    /*
    * Constructor to setup default values
    */
    Coordinate() : x(0), y(0) {}

    /*
    * Constructor to setup using input values
    * @param xCorr x coordinate
    * @param yCorr y coordinate
    */
    Coordinate(long double const xCorr, long double const yCorr) : x(xCorr), y(yCorr) {}

    /*
    * Getter to get x coordinate
    * Return x coordinate of the point
    */
    long double getX()
    {
        return this->x;
    }

    /*
    * Getter to get y coordinate
    * Return y coordinate of the point
    */
    long double getY()
    {
        return this->y;
    }

    /*
    * Setter for x and y coordinate
    * @param xCorr x coordinate of the point
    * @param yCorr y coorindate of the point
    */
    void setCoordinate(long double xCorr, long double yCorr)
    {
        this->x = xCorr;
        this->y = yCorr;
    }

    /*
    * Function to print the coordinates
    * Used for debugging
    */
    void printCoordinate()
    {
        std::cout << std::setprecision(64) << "Point location is x = " << this->x << " y = " << this->y << std::endl;
    }
    /*
    * Function to check if the current point is valid
    * Validity check: if x is between -10 and 10 or equal to either
    * and y is between 0 and 20 or equal to either
    * Return bool
    */
    bool isValid()
    {
        double xToCheck = double(this->x);
        double yToCheck = double(this->y);
        bool checkX = ((xToCheck > -10 && xToCheck < 10) || xToCheck == -10 || xToCheck == 10);
        bool checkY = ((yToCheck > 0 && yToCheck < 10 * sqrt(3)) || yToCheck == double(10 * sqrt(3)) || yToCheck == 0);
        return checkX && checkY;
    }

    /*
    * Overload operator !=
    */
    bool operator!= (Coordinate toCheck)
    {
        if ((double)(this->x) != (double)(toCheck.getX()) || (double)(this->y) != (double)(toCheck.getY()))
        {
            return true;
        }
        return false;
    }

    /*
    * Overload operator ==
    */
    bool operator== (Coordinate toCheck)
    {
        if ((double)(this->x) == (double)(toCheck.getX()) && (double)(this->y) == (double)(toCheck.getY()))
        {
            return true;
        }
        return false;
    }
}; // Coordinate


/*
* Class to represent a line
* y = mx+c
* m => slope
* c => constant
*/
class Line
{
    // Variable to represent the line
    long double slope;
    long double constant;
public:
    /*
    * Constructor to init the class
    */
    Line() : slope(0), constant(0) {}

    /*
    * Constructor to init using input values
    * @param inSlope slope of the line
    * @param inConstant constant of the line
    */
    Line(long double inSlope, long double inConstant) : slope(inSlope), constant(inConstant) {}

    /*
    * Setter for the slope and constant of the line
    * @param inSlope slope of the line
    * @param inConstant constant of the line
    */
    void setLine(long double inSlope, long double inConstant)
    {
        this->slope = inSlope;
        this->constant = inConstant;
    }

    /*
    * Getter for the slope
    * Return slope of the line
    */
    long double getSlope()
    {
        return this->slope;
    }

    /*
    * Getter for the constant
    * Return constant of the line
    */
    long double getConstant()
    {
        return this->constant;
    }

    /*
    * Function to create line from 2 points (A and B)
    * @param A point B for the line AB
    * @param B point B for the line AB
    */
    void createLine(Coordinate APoint, Coordinate BPoint)
    {
        // Variable to hold the x and y coordinate of A and B
        long double x1 = APoint.getX();
        long double x2 = BPoint.getX();
        long double y1 = APoint.getY();
        long double y2 = BPoint.getY();

        if (x2 - x1 != 0)
        {
            // If the slope is not inf
            this->slope = (y2 - y1) / (x2 - x1);
        }
        else
        {
            // If the slope is inf then set it the max the variable can hold
            this->slope = std::numeric_limits<double>::max();
        }

        // Now we know slope => put one coordinate in line and solve for constant
        this->constant = y2 - this->slope * x2;
    }

    /*
    * Function to create line from the slope and 1 point
    * @param inSlope slope of the line
    * @param point the point on the line
    */
    void createLine(long double inSlope, Coordinate point)
    {
        this->slope = inSlope;
        // Using y = mx+c then c = y - mx
        this->constant = point.getY() - this->slope * point.getX();
    }

    /*
    * Function to print the line
    * Used of debugging
    */
    void printLine()
    {
        std::cout << "Equation of line is y = " << this->slope << "x + " << this->constant << std::endl;
    }

    /*
    * Function to find the x coordinate if the y is given
    * @param y coordinate of the point on the line
    * Return x coordinate of the point on the line
    */
    long double findXCoordinate(long double y)
    {
        // Using y = mx + c => x = (y-c)/m;
        return (y - this->constant) / this->slope;
    }

    /*
    * Function to find the y coordinate if the x is given
    * @param x coordinate of the point on the line
    * Return y coordinate of the point on the line
    */
    long double findYCoordinate(long double x)
    {
        // Using y = mx + c
        return (this->slope * x + this->constant);
    }

    /*
    * Overload operator !=
    */
    bool operator!= (Line toCheck)
    {
        if (double(this->slope) != double(toCheck.getSlope()) || double(this->constant) != double(toCheck.getConstant()))
        {
            return true;
        }
        return false;
    }
};

/*
* Function to get point of intersection between two lines
* @param line1 of form y = mx+c
* @param line2 of form y = mx+c
* Return point of intersection
*/
Coordinate getIntersectionPoint(Line line1, Line line2)
{
    // Equation of line 1 => y = m1x + c1
    // Equation of line 2 => y = m2x + c2
    long double m1 = line1.getSlope();
    long double m2 = line2.getSlope();
    long double c1 = line1.getConstant();
    long double c2 = line2.getConstant();

    // Intersection point found by solving the line equations
    long double y = (m2 * c1 - m1 * c2) / (m2 - m1);
    long double x = (c2 - c1) / (m1 - m2);

    // Create the coordinate object for intersection
    Coordinate intersection(x, y);
    return intersection;
}

/*
* Function to create a perpendicular line
*
* @param line1 line to find perpendicular of
* @param point point of intersection between perpendicular and line
* Return perpendicular line
*/
Line getPerpendicularLine(Line line1, Coordinate point)
{
    // Init the object of the perpendicular line
    Line perpendicular;

    // Slope of the perpendicular
    long double m2;

    // Using m1 x m2 = -1
    if (line1.getSlope() != 0)
    {
        // If slope not 0, then m2 will have a non inf value
        m2 = (-1) * (1 / line1.getSlope());
    }
    else
    {
        // if m1 = 0 => m2 = infinite meaning it is parallel to y-axis
        // Set to max value long double can hold
        m2 = std::numeric_limits<double>::max();
    }

    // Intersection coordinates
    long double x = point.getX();
    long double y = point.getY();

    // Solving equation y = m2x + c using x and y above
    long double c = y - m2 * x;

    // Set the value in the perpendicular line
    perpendicular.setLine(m2, c);
    return perpendicular;
}

/*
* Function to get the equation of reflect line
*
* Reference: https://math.stackexchange.com/questions/67024/the-equation-of-a-line-reflected-about-another-line
*

* Derived using trignometric simplification across tan
* 
* @param mirror line equation of the mirror
* @param incident line equation of the incident ray
* Return line equation of the reflected line
*/
Line getReflectedLine(Line mirror, Line incident)
{
    // Setting the values as per the required equation (reference line in the function desc)
    /*
    * Solving equations
    * * Incident Ax + By = C equating to y = mx+c; A = -m, B = 1; C = c;
    * Mirror Dx + Ey = F equating to y = mx+c; D = -m, E = 1; F = c;
    * 
    * Equation used:
    * (y - h) = (AE^2 - AD^2 - 2BDE)(x - k) / (BE^2 - BD^2 + 2ADE)
    * where h = (CD - AF) / (BD - AE)
    *       k = (CE - BF) / (AE - BD)
    * Variables names set to match these equations 
    */
    long double AVar = (-1) * incident.getSlope();
    long double BVar = 1;
    long double CVar = incident.getConstant();
    long double DVar = (-1) * mirror.getSlope();
    long double EVar = 1;
    long double FVar = mirror.getConstant();

    long double kVar = (CVar * EVar - BVar * FVar) / (AVar * EVar - BVar * DVar);
    long double hVar = (CVar * DVar - AVar * FVar) / (BVar * DVar - AVar * EVar);

    long double slope = (AVar * EVar * EVar - AVar * DVar * DVar - 2 * BVar * DVar * EVar) / 
        (BVar * EVar * EVar - BVar * DVar * DVar + 2 * AVar * DVar * EVar);
    long double constant = hVar + slope * (-kVar);

    // Create the reflected line
    Line reflected(slope, constant);
    return reflected;
}

/*
* Function to check if the intersection escapes the triangle
* 
* @param intersection: Intersection point to check
* @param xEscapeEdge1: Upper limit of x for escape
* @param xEscapeEdge2: Lower limit of x for escape
* @param yLimit: Upper limit of y for escape
* Return bool whether the point escapes
*/
bool isEscape(Coordinate intersection, long double xEscapeEdge1, long double xEscapeEdge2, long double yLimit)
{
    long double x = intersection.getX();
    long double y = intersection.getY();

    if (x < xEscapeEdge1 && x > xEscapeEdge2 && y < yLimit)
    {
        return true;
    }
    return false;
}

/*
* Main function of the program
*/
int main(int argc, char* argv[])
{
    // Write data into file
    std::ofstream outfile;
    outfile.open("output3.txt", std::ios::trunc);

    if (not outfile.is_open())
    {
        // Print error if unable to open outfile
        std::cerr << "Unable to open output file: output3.txt" << std::endl;
        return 1;
    }

    if (argc != 2)
    {
        // Check 1: Expected input args = 1 (+ executable)
        // Print error if check 1 fails
        outfile << "Invalid inputs";
        outfile.close();
        return 1;
    }

    // Variable to hold the input number
    long double inNumber{ 0 };

    if (not convertToNumber(argv[1], inNumber))
    {
        // Check 2: if the number if not valid
        outfile << "Invalid inputs";
        outfile.close();
        return 1;
    }

    // check for range
    if (inNumber > 10 || inNumber < -10)
    {
        // Check 3: if number provided is outside the length of AB
        outfile << "Invalid inputs";
        outfile.close();
        return 1;
    }

    // Create the edges of the triangle
    // Variables A,B,C used to match the lab PDF vertices
    Coordinate A(-10, 10 * sqrt(3));
    Coordinate B(10, 10 * sqrt(3));
    Coordinate C(0, 0);

    // Create equations of the sides of triangle
    Line side1;
    side1.createLine(A, B);
    Line side2;
    side2.createLine(B, C);
    Line side3;
    side3.createLine(C, A);

    // Create equations of lines for corners case
    // When the line hits the vertex, imagine a mirror there such that it reflects
    // the ray back along its the angle bisector

    // For line through A
    Line cornerASide;
    cornerASide.createLine(sqrt(3), A);

    // For line through B
    Line cornerBSide;
    cornerBSide.createLine((-1) * sqrt(3), B);

    // For line through C
    Line cornerCSide;
    cornerCSide.createLine(0, C);

    // Find the x coordinates on the line AC and BC for y = 0.01 for escape
    long double xEscapeEdge1 = side2.findXCoordinate(0.01);
    long double xEscapeEdge2 = side3.findXCoordinate(0.01);

    // inNumber is the x-coordinate of where the first incident ray meets y=20
    // Create line
    Line incidentLine;
    // Point A = (0,0), Point B = (inNumber, 10sqrt3)
    Coordinate inA(0, 0);
    Coordinate inB(inNumber, 10 * sqrt(3));
    incidentLine.createLine(inA, inB);

    // Variable to hold the number of bounces
    long double relectionCount = 0;

    // Final intersection after checks
    Coordinate intersection;
    // Common variable to exclude the same line from being considered in the next iteration
    Line side;
    // Reflected equation for each iteration
    Line reflected;

    do
    {
        relectionCount++;
        // Get point of intersection with all 3 sides
        Coordinate intersection1 = getIntersectionPoint(incidentLine, side1);
        Coordinate intersection2 = getIntersectionPoint(incidentLine, side2);
        Coordinate intersection3 = getIntersectionPoint(incidentLine, side3);

        // check which instersection if valid and inside the equilateral triangle
        // it should also not be the intersection point considered in the last iteration
        // => this new mirror side should be different
        if (intersection1.isValid() && side1 != side)
        {
            intersection = intersection1;
            side = side1;
        }
        else if (intersection2.isValid() && side2 != side)
        {
            intersection = intersection2;
            side = side2;
        }
        else if (intersection3.isValid() && side3 != side)
        {
            intersection = intersection3;
            side = side3;
        }
        else
        {
            // If not intersection found print error
            outfile.close();
            std::cerr << "No intersection found. Bug in code" << std::endl;
            return 0;
        }

        // Get line equation of the reflected line using mirror (side) and incidentLine
        reflected = getReflectedLine(side, incidentLine);

        // check for the corner case when the intersection is at the vertex of the triangle
        if (intersection == A)
        {
            reflected = getReflectedLine(cornerASide, incidentLine);
        }
        else if (intersection == B)
        {
            reflected = getReflectedLine(cornerBSide, incidentLine);
        }
        // No need to check for corner C as that is where the ray would exit

        incidentLine = reflected;
        // Used for debugging the intersection points
        //intersection.printCoordinate();
    } while (isEscape(intersection, xEscapeEdge1, xEscapeEdge2, 0.01) == false);

    relectionCount--; // Remove the last intersection as a reflection as that was the exit point


    outfile << relectionCount;
    outfile.close();

    return 0;
}
