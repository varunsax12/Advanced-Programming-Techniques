/*
Description:
    Final Project for ECE 6122: Gatech Buzzy Bowl
    UAVs to display a show where UAVs a placed across
    the football field and launch towards a virtual
    sphere at 0m, 0m, 50m. It then simulates the
    motion of the UAVs around the sphere along with
    collisions.

Top level references:
    http://www.opengl-tutorial.org/
    Code references from tutorial 9: Several objects
*/

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <chrono>
#include <cmath>
#include <iostream>
#include <list>
#include <utility>
#include <thread>
#include <mutex>
#include <atomic>
#include <random>
#include <array>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <common/controls.hpp>

/*
* Enum to define the type of motion the UAV is in
* Motion definitions
* 1. aroundSphere => It uses angular momentum to move
* with constant angular velocity around the sphere in
* a given plane out of XY, YZ and ZX. UAV uses it force
* to simply negate the gravity
* 2. linear => It tries to move linearly from one point
* to another accounting for all types of forces
* 3. stopMotion => To stop the UAV motion
*/
typedef enum motionType
{
    aroundSphere = 0,
    linear = 1,
    stopMotion = 2
};

/*
* Enum to represent the motion plane when
* the UAV is in "aroundSphere" type motion
*/
typedef enum motionPlane
{
    XYPlane = 0,
    YZPlane = 1,
    ZXPlane = 2,
};

/*
* Create mutex locks for each UAV object
*/
std::mutex mtxUAVUpdate[15];

/*
* Global flag to kill threads
*/
std::atomic<bool> atmCloseThread = false;

/*
* Global control to handle the speed based on display refresh rate
*/
float frameSpeedControl = 1.0f;

/*
* Global value of gravity always taken in the negative z direction
*/
float gravityForce = 10.0f;

/*
* Global value of acceleration exerted by the UAV
*
* NOTE: For this applying the physics concept of frame of reference
* In the project PDF, the UAV can exert a 20N force in any arbitrary
* direction. I assume that the UAV is applying force to maintain its
* path either towards the virtual sphere or to ensure it can revolve
* around it => always towards the point 0, 0, 50
* So, if I change my frame of reference to the UAV, it will appear
* as though the virtual sphere at 0, 0, 50 has a local gravity of
* 20N which is pulling the UAV towards it. Thus, in all calculation,
* I have assumed that the frame of reference is the UAV so, performing,
* local calculation for each UAV can be made into a standard algo.
*
* So, setting the global force to denote the local gravity of the
* virtual sphere as seen by each UAV
*/
float uavForce = 20.0f;

/*
* Global variables to hold the location of the central point of
* the virtual sphere, around which the UAV will revolve.
* Position defined as per project requirement PDF
*/
float sphereCenterX = 0.0f;
float sphereCenterY = 0.0f;
float sphereCenterZ = 50.0f;

/*
* Global for max UAV velocity allowed
*/
float maxUAVvelocity = 10.0f;

/*
* Global for max rise velocity allowed for UAV
*/
float maxUAVRiseVelocity = 2.0f;

/*
* Global randomized for plane of motion of UAV
*/
// creating random number generator per thread
std::random_device rd;
std::default_random_engine randGenerator{ rd() };
// number to direction -> 1=>left, 2=>up, 3=>right, 4=>down
std::uniform_int_distribution<int> randDistribution(0, 2);

/*
* Class representing the UAV used in the simulation
*/
class ECE_UAV;

/*
* Function for thread functionality to update the location of the UAV every 10ms
* @param pUAV -> pointer to the UAV
*/
void threadFunction(ECE_UAV* pUAV);

/*
* Function to convert meter to yard
* @param inLength => length in meters
* @return float => length in yards
*/
float convertMeterToYard(float inLength);

/*
* Class representing the UAV used in the simulation
*/
class ECE_UAV
{
    // The 3D coordinates of the UAV
    float x, y, z;
    // The 3D velocity of the UAV
    float vx, vy, vz;
    // The 3D acceleration seen by the UAV
    float ax, ay, az;
    // Mass of the UAV
    float mass;

    // The safe distance from 0,0,50 which still allows the UAV
    // to be visible (set to 10 as per Project PDF)
    float safeRadius;
    // Type of motion UAV is in (aroundSphere, Linear, stopMotion)
    motionType motion;
    // Motion plane direction when moving around the sphere (XY, YZ or ZX plane)
    motionPlane motionDirection;
    // Speed of movement around the sphere
    float angularVelocity;

    // Flag denoting whether the UAV has reached the sphere from its
    // starting position
    std::atomic<bool> reachedSphere;

    // Variable to hold the thread updating the position of the current UAV
    std::thread currentThread;

    // UAV index used to access the correct index in the mutex lock array
    // for the current UAV
    int uavID;

    // Count down timer to track how many time cycles have passed or are left
    // to able to account for next collision. Implemented so as to avoid multiple
    // consecutive collisions with the same object in case velocity is too low
    // such that after collision the uavs do not move far away to move outside
    // the colliding range.
    // NOTE: Only accessed by main thread, so thread safety is not required for this
    int collisionCountDown;
public:

    /*
    * Function to act upon UAV collision
    * @param uav1 -> pointer to the first UAV
    * @param uav2 -> pointer to the second UAV
    */
    friend void uavCollision(ECE_UAV* uav1, ECE_UAV* uav2);

    /*
    * Constructor for the UAV class
    * @param inUavID => uav id for the current uav
    * @param initX => initial X position of uav
    * @param initY => initial Y position of uav
    * @param initZ => initial Z position of uav
    * @param initMotion => initial motion type of the uav
    * @param initDirection => initial motion plane of the uav
    */
    ECE_UAV(int inUavID, float initX, float initY, float initZ, motionType initMotion, motionPlane initDirection) :
        x(initX), y(initY), z(initZ), vx(0), vy(0), vz(0), safeRadius(10), motion(initMotion),
        motionDirection(initDirection), angularVelocity(0), mass(1), reachedSphere(false), uavID(inUavID),
        collisionCountDown(0)
    {
        this->start();
    }

    /*
    * Function to start the motion of the UAV
    */
    void start()
    {
        currentThread = std::thread(threadFunction, this);
    }

    /*
    * Function to check if the uav has reached the virtual sphere
    * after the initial launch
    * @return bool => if the uav has reached the sphere from initial position
    */
    bool isUavOnSphere()
    {
        return this->reachedSphere;
    }

    /*
    * Function to set the collision counter
    */
    void setCollisionCounter()
    {
        this->collisionCountDown = 20;
    }

    /*
    * Function to check if countdown complete
    * and ready for collision
    * @return bool => if the uav can safely collide again
    */
    bool canCollidedAgain()
    {
        return this->collisionCountDown <= 0;
    }

    /*
    * Function to decrement the collision count down
    */
    void decrementCollisionCounter()
    {
        if (this->collisionCountDown > 0)
        {
            this->collisionCountDown--;
        }
    }

    /*
    * Getter for x coordinate of position
    * @return float -> x position
    */
    float getXPos()
    {
        float xPos = 0;
        mtxUAVUpdate[this->uavID].lock();
        xPos = this->x;
        mtxUAVUpdate[this->uavID].unlock();
        return xPos;
    }

    /*
    * Getter for y coordinate of position
    * @return float -> y position
    */
    float getYPos()
    {
        float yPos = 0;
        mtxUAVUpdate[this->uavID].lock();
        yPos = this->y;
        mtxUAVUpdate[this->uavID].unlock();
        return yPos;
    }

    /*
    * Getter for z coordinate of position
    * @return float -> z position
    */
    float getZPos()
    {
        float zPos = 0;
        mtxUAVUpdate[this->uavID].lock();
        zPos = this->z;
        mtxUAVUpdate[this->uavID].unlock();
        return zPos;
    }

    /*
    * Getter for vx velocity
    * @return float -> vx velocity
    */
    float getXVel()
    {
        float vxVel = 0;
        mtxUAVUpdate[this->uavID].lock();
        vxVel = this->vx;
        mtxUAVUpdate[this->uavID].unlock();
        return vxVel;
    }

    /*
    * Getter for vy velocity
    * @return float -> vy velocity
    */
    float getYVel()
    {
        float vyVel = 0;
        mtxUAVUpdate[this->uavID].lock();
        vyVel = this->vy;
        mtxUAVUpdate[this->uavID].unlock();
        return vyVel;
    }

    /*
    * Getter for vz velocity
    * @return float -> vz velocity
    */
    float getZVel()
    {
        float vzVel = 0;
        mtxUAVUpdate[this->uavID].lock();
        vzVel = this->vz;
        mtxUAVUpdate[this->uavID].unlock();
        return vzVel;
    }

    /*
    * Getter for angular velocity
    * @return float -> angular velocty
    */
    float getAngularVel()
    {
        float currAngularVel = 0;
        mtxUAVUpdate[this->uavID].lock();
        currAngularVel = this->angularVelocity;
        mtxUAVUpdate[this->uavID].unlock();
        return currAngularVel;
    }

    /*
    * Getter for motion direction
    * @return motionPlane => plane of current motion
    */
    motionPlane getMotionDirection()
    {
        motionPlane currPlane = XYPlane;
        mtxUAVUpdate[this->uavID].lock();
        currPlane = this->motionDirection;
        mtxUAVUpdate[this->uavID].unlock();
        return currPlane;
    }

    /*
    * Getter for motion type
    * @return motionType => type of current motion
    */
    motionType getMotionType()
    {
        motionType currMotion = stopMotion;
        mtxUAVUpdate[this->uavID].lock();
        currMotion = this->motion;
        mtxUAVUpdate[this->uavID].unlock();
        return currMotion;
    }

    /*
    * Setter for x coordinate of position
    * @param xPos => x postion
    */
    void setXPos(float xPos)
    {
        mtxUAVUpdate[this->uavID].lock();
        this->x = xPos;
        mtxUAVUpdate[this->uavID].unlock();
    }

    /*
    * Setter for y coordinate of position
    * @param yPos => y postion
    */
    void setYPos(float yPos)
    {
        mtxUAVUpdate[this->uavID].lock();
        this->y = yPos;
        mtxUAVUpdate[this->uavID].unlock();
    }

    /*
    * Setter for z coordinate of position
    * @param zPos => z postion
    */
    void setZPos(float zPos)
    {
        mtxUAVUpdate[this->uavID].lock();
        this->z = zPos;
        mtxUAVUpdate[this->uavID].unlock();
    }

    /*
    * Setter for vx velocity
    * @param vxVel -> vx velocity
    */
    void setXVel(float vxVel)
    {
        mtxUAVUpdate[this->uavID].lock();
        this->vx = vxVel;
        mtxUAVUpdate[this->uavID].unlock();
    }

    /*
    * Setter for vy velocity
    * @param vyVel -> vy velocity
    */
    void setYVel(float vyVel)
    {
        mtxUAVUpdate[this->uavID].lock();
        this->vy = vyVel;
        mtxUAVUpdate[this->uavID].unlock();
    }

    /*
    * Setter for vz velocity
    * @param vzVel -> vz velocity
    */
    void setZVel(float vzVel)
    {
        mtxUAVUpdate[this->uavID].lock();
        this->vz = vzVel;
        mtxUAVUpdate[this->uavID].unlock();
    }

    /*
    * Setter for angular velocity
    * @param inAngularVel -> angular velocty
    */
    void setAngularVel(float inAngularVel)
    {
        mtxUAVUpdate[this->uavID].lock();
        this->angularVelocity = inAngularVel;
        mtxUAVUpdate[this->uavID].unlock();
    }

    /*
    * Setter for motion directio
    * @param inMotionDir => plane of current motion
    */
    void wetMotionDirection(motionPlane inMotionDir)
    {
        mtxUAVUpdate[this->uavID].lock();
        this->motionDirection = inMotionDir;
        mtxUAVUpdate[this->uavID].unlock();
    }

    /*
    * Getter for motion type
    * @param inMotionType => type of current motion
    */
    void setMotionType(motionType inMotionType)
    {
        mtxUAVUpdate[this->uavID].lock();
        this->motion = inMotionType;
        mtxUAVUpdate[this->uavID].unlock();
    }

    /*
    * Function to join the thread in the main
    */
    void joinThread()
    {
        this->currentThread.join();
    }

    /*
    * Function to update the position of the UAV based on motion type,
    * current position and velocities
    * @param timeElapsed => time elapsed since last update to position
    */
    void updatePosition(float timeElapsed)
    {
        motionPlane currMotionDir = this->getMotionDirection();
        motionType currMotion = this->getMotionType();
        // If the UAV is currently stalled and not moving
        if (currMotion == stopMotion)
        {
            // => no change in the position
            return;
        }
        // if the UAV if moving around the virtual sphere at 0, 0, 50
        else if (currMotion == aroundSphere)
        {
            // If motion is in the XY plane && not on the XY plane centre
            if (currMotionDir == XYPlane)
            {
                // Call function to update the position
                this->moveAroundSphereInXYPlane();
            }
            // If motion is in the XZ plane && not on the XZ plane centre
            else if (currMotionDir == ZXPlane)
            {
                // Call function to update the position
                this->moveAroundSphereInZXPlane();
            }
            // If motion is in the YZ plane && not on the ZY plane centre
            else
            {
                // Call function to update the position
                this->moveAroundSphereInZYPlane();
            }
        }
        // If motion is translational
        else if (currMotion == linear)
        {
            // If the UAV has reached the sphere for its starting position
            if (this->reachedSphere == false)
            {
                moveLinear(timeElapsed, maxUAVRiseVelocity);

            }
            else
            {
                moveLinear(timeElapsed, maxUAVvelocity);
            }
            // Check if the UAV within the radius of 0, 0, 50 to initiate revolution
            // around the virtual sphere
            if (this->checkDistanceFromSphere(this->safeRadius))
            {
                // Flag set to denote it has reached sphere after first rise
                this->reachedSphere = true;
                // Change motion type to indicate UAV is now revoling around sphere
                this->setMotionType(aroundSphere);
                // Compute the angular velocity to use constant angular momemtum
                // for the revolution around the sphere
                this->computerVelocityAngularFromTranslation();
            }
        }
    }

    /*
    * Function to check the distance from the center of
    * virtual sphere
    * @param acceptableDist -> allowable distance from sphere center
    * @return bool -> if the UAV is within range
    */
    bool checkDistanceFromSphere(float acceptableDist)
    {
        // Calculate disatance from the target (X,Y,Z)
        float dist = std::sqrt(
            std::pow(this->getXPos() - sphereCenterX, 2) +
            std::pow(this->getYPos() - sphereCenterY, 2) +
            std::pow(this->getZPos() - sphereCenterZ, 2));
        // If it is within the threshold => target hit
        // Additional 0.05 space kept to keep the UAV models visible
        if (dist <= acceptableDist + 0.05)
        {
            return true;
        }
        return false;
    }

    /*
    * Function to move the UAV in linear motion
    * @param timeElapsed -> time elapsed from initial to destination points
    * @param inMaxUAVVel -> maximum UAV for this segment of motion
    */
    void moveLinear(float timeElapsed, float inMaxUAVVel)
    {

        // Init variables to hold values to avoid accessing thread safe variables too many times
        float xPos = this->getXPos();
        float yPos = this->getYPos();
        float zPos = this->getZPos();
        float xVel = this->getXVel();
        float yVel = this->getYVel();
        float zVel = this->getZVel();

        // Creating the displacement vector between where the UAV is
        // and the centre of the virtual sphere
        // UAV -------> Sphere
        float dispX = sphereCenterX - xPos;
        float dispY = sphereCenterY - yPos;
        float dispZ = sphereCenterZ - zPos;

        // Compute the acceleration seen by the UAV by resolving the force
        // between UAV and virtual sphere into x y z

        // Remove the amount of force the UAV uses to counter gravity
        float currUavForce = uavForce - gravityForce;

        float forceX = currUavForce * dispX / std::sqrt(std::pow(dispX, 2) + std::pow(dispY, 2) + std::pow(dispZ, 2));
        float forceY = currUavForce * dispY / std::sqrt(std::pow(dispX, 2) + std::pow(dispY, 2) + std::pow(dispZ, 2));
        float forceZ = currUavForce * dispZ / std::sqrt(std::pow(dispX, 2) + std::pow(dispY, 2) + std::pow(dispZ, 2));

        // Converting force into acceleration
        this->ax = forceX / this->mass;
        this->ay = forceY / this->mass;
        this->az = forceZ / this->mass;

        // Calculating current absolute UAV velocity
        float currVelocity = std::sqrt(std::pow(xVel, 2) + std::pow(yVel, 2) + std::pow(zVel, 2));
        // Flag to denote whether the UAV is at the highest velocity value
        bool hasReachedMaxVel = (inMaxUAVVel - currVelocity) < 0.1f;

        // If it is near max velocity then the velocities cannot increase further
        // Now, UAV can use all its force to negate the force which will increase
        // the velocity further and simply use its own force to keep the velocity
        // in check
        // => check if sign of velocty and acceleration opposite, only then use it

        // If max velocity not reached or if reached, then deacceleration in progress in X
        if (hasReachedMaxVel == false || (xVel * this->ax) < 0)
        {
            xPos += xVel * timeElapsed + 0.5 * this->ax * timeElapsed * timeElapsed;
            xVel += this->ax * timeElapsed;
        }
        // If max velocity hit and still accelerating in X
        else
        {
            xPos += xVel * timeElapsed;
        }
        // If max velocity not reached or if reached, then deacceleration in progress in Y
        if (hasReachedMaxVel == false || (yVel * this->ay) < 0)
        {
            yPos += yVel * timeElapsed + 0.5 * this->ay * timeElapsed * timeElapsed;
            yVel += this->ay * timeElapsed;
        }
        // If max velocity hit and still accelerating in Y
        else
        {
            yPos += yVel * timeElapsed;
        }
        // If max velocity not reached or if reached, then deacceleration in progress in Z
        if (hasReachedMaxVel == false || (zVel * this->az) < 0)
        {
            zPos += zVel * timeElapsed + 0.5 * this->az * timeElapsed * timeElapsed;
            zVel += this->az * timeElapsed;
        }
        // If max velocity hit and still accelerating in Z
        else
        {
            zPos += zVel * timeElapsed;
        }

        // Update all thread safe variables now
        this->setXPos(xPos);
        this->setYPos(yPos);
        this->setZPos(zPos);
        this->setXVel(xVel);
        this->setYVel(yVel);
        this->setZVel(zVel);
    }

    /*
    * Function to get angle between vectors in the same plane
    * @param vec1 => first vector
    * @param vec2 => second vector
    * @return float => angle between the two vectors
    *
    * Reference: https://www.wikihow.com/Find-the-Angle-Between-Two-Vectors
    */
    float getCircularAngle(std::pair<float, float> vec1, std::pair<float, float> vec2)
    {
        // Calculate the angle between vec1 and vec2
        float num = (vec1.first * vec2.first + vec1.second * vec2.second);
        float denom = std::sqrt(std::pow(vec1.first, 2) + std::pow(vec1.second, 2));
        denom *= std::sqrt(std::pow(vec2.first, 2) + std::pow(vec2.second, 2));
        // Using vector content to increase the range of motion across 360 degree
        if (vec1.second < 0)
        {
            return std::acos(num / denom) * (-180.0f) / 3.14 + this->getAngularVel() * frameSpeedControl;
        }
        else
        {
            return std::acos(num / denom) * 180.0f / 3.14 + this->getAngularVel() * frameSpeedControl;
        }
    }

    /*
    * Function to move the UAV around the virtual sphere
    * in the XY plane
    *
    * Logic: Create 2 vector, one reference with one of the standard axis and the
    * displacement vector. Find angle between the two, update the angle using the
    * angular velocity
    *
    * Reference: https://math.stackexchange.com/questions/1267644/move-a-point-with-known-angle-on-a-circle
    */
    void moveAroundSphereInXYPlane()
    {
        // Centre of XY plane circle (x,y)
        // Calculate new radius which will be smaller than 10m
        float new_radius = std::sqrt(std::pow(this->safeRadius, 2) - std::pow(sphereCenterZ - this->getZPos(), 2));
        // Compute first vector between XY plane sphere center and XY point
        std::pair<float, float> vec1 = std::make_pair(this->getXPos() - sphereCenterX, this->getYPos() - sphereCenterY);
        // Using the x intercept of current circle are reference angle, computing second vector
        std::pair<float, float> vec2 = std::make_pair(new_radius - sphereCenterX, sphereCenterY);
        // Update location based on angle and radius
        this->setXPos(new_radius * cos(this->getCircularAngle(vec1, vec2) * 3.14 / 180) + sphereCenterX);
        this->setYPos(new_radius * sin(this->getCircularAngle(vec1, vec2) * 3.14 / 180) + sphereCenterY);
    }

    /*
    * Function to move the UAV around the virtual sphere
    * in the ZY plane
    *
    * Logic: Create 2 vector, one reference with one of the standard axis and the
    * displacement vector. Find angle between the two, update the angle using the
    * angular velocity
    *
    * Reference: https://math.stackexchange.com/questions/1267644/move-a-point-with-known-angle-on-a-circle
    */
    void moveAroundSphereInZYPlane()
    {
        // Centre of circle (z,y)
        // Calculate new radius which will be smaller than 10m
        float new_radius = std::sqrt(std::pow(this->safeRadius, 2) - std::pow(sphereCenterX - this->getXPos(), 2));
        // Compute first vector between ZY plane sphere center and ZY point
        std::pair<float, float> vec1 = std::make_pair(this->getZPos() - sphereCenterZ, this->getYPos() - sphereCenterY);
        // Using the z intercept of current circle are reference angle, computing second vector
        std::pair<float, float> vec2 = std::make_pair(sphereCenterZ - new_radius, sphereCenterY);
        // Update location based on angle and radius
        this->setZPos(new_radius * cos(this->getCircularAngle(vec1, vec2) * 3.14 / 180) + sphereCenterZ);
        this->setYPos(new_radius * sin(this->getCircularAngle(vec1, vec2) * 3.14 / 180) + sphereCenterY);
    }

    /*
    * Function to move the UAV around the virtual sphere
    * in the ZX plane
    *
    * Logic: Create 2 vector, one reference with one of the standard axis and the
    * displacement vector. Find angle between the two, update the angle using the
    * angular velocity
    *
    * Reference: https://math.stackexchange.com/questions/1267644/move-a-point-with-known-angle-on-a-circle
    */
    void moveAroundSphereInZXPlane()
    {
        // Centre of circle (z,x)
        // Calculate radius
        float new_radius = std::sqrt(std::pow(this->safeRadius, 2) - std::pow(sphereCenterY - this->getYPos(), 2));
        // Compute first vector between ZX plane sphere center and ZX point
        std::pair<float, float> vec1 = std::make_pair(this->getZPos() - sphereCenterZ, this->getXPos() - sphereCenterX);
        // using the y intercept of current circle are reference angle, copmuting second vector
        std::pair<float, float> vec2 = std::make_pair(sphereCenterZ - new_radius, sphereCenterX);
        // Update location based on angle and radius
        this->setZPos(new_radius * cos(this->getCircularAngle(vec1, vec2) * 3.14 / 180) + sphereCenterZ);
        this->setXPos(new_radius * sin(this->getCircularAngle(vec1, vec2) * 3.14 / 180) + sphereCenterX);
    }

    /*
    * Function to check for collision between 2 UAVs
    * @param inX -> centre of the other UAV
    * @param inY -> centre of the other UAV
    * @param inZ -> centre of the other UAV
    * @return bool -> if collision has occured
    */
    bool checkCollision(float inx, float iny, float inz)
    {
        // using a circular bounding box. Check distance between centre of two objects
        // Bbox radius is 10cm
        float dist = std::sqrt(
            std::pow(this->getXPos() - inx, 2) +
            std::pow(this->getYPos() - iny, 2) +
            std::pow(this->getZPos() - inz, 2));
        // Check if the current UAV is moving around sphere
        // and the distance between the two is such that
        // the bounding box is within 1cm
        if (dist < 1 && this->checkDistanceFromSphere(this->safeRadius + 1))
        {
            // Collision has occured
            return true;
        }
        // No collision occured
        return false;
    }

    /*
    * Function to get the moment of inertia
    * References: http://hyperphysics.phy-astr.gsu.edu/hbase/mi.html
    *
    * @return float => the moment of inertia
    */
    float getMomentInertia()
    {
        return (2.0f / 5.0f) * this->mass * std::pow(this->safeRadius, 2);
    }

    /*
    * Function to compute the angular velocity from the translation velocty
    * This happens when there is a change in motion type from linear to rotation.
    * So, we use the logic of angular momentum to keep the UAV revolving around
    * the sphere. This considers that UAV is using all its force of 20N to negate
    * the effect of gravity and maintain constant angular momentum
    *
    * References: https://en.wikipedia.org/wiki/Rotational_energy
    */
    void computerVelocityAngularFromTranslation()
    {
        // Now by conserving kinetic energy, I convert the translations kinetic
        // energy into rotational kinetic energy
        // KE translation = 1/2 mv^2, where m = mass, v = velocity
        // KE rotation = 1/2 Iw^2, where I = moment of inertic, w = angular velocity
        // Thus, equating the two, we get, w = v * sqrt(m/I)

        float velocityTrans = std::sqrt(std::pow(this->getXVel(), 2) + std::pow(this->getYVel(), 2) + std::pow(this->getZVel(), 2));
        // If the velocity of trans is around 2, then scale to between 2 and 10 as per lab PDF
        // to show better motion representation
        if (std::abs(velocityTrans - 2) < 0.5)
        {
            velocityTrans = 5.0f;
        }
        float newAngularVel = velocityTrans * std::sqrt(this->mass / this->getMomentInertia());

        // Now to randomize the motion, if the UAV is coming in from -x direction
        // then set the angular velocity to counter clockwise
        if (this->getXPos() <= 0)
        {
            newAngularVel *= -1;
        }
        this->setAngularVel(newAngularVel);
    }

    /*
    * Function to get directional velocity pair using the
    * displacement vector and total translation velocity
    * @param dispVec => pair of floats for the displacement vector
    * @param velocityTans => translational velocity
    * @return pair of float => representing components of velocity in given plane
    *
    * NOTE: No need to call thread safe functions here as parent caller uses global
    * thread safety
    */
    std::pair<float, float> computerVelocityComponents(std::pair<float, float> dispVec,
        float velocityTrans)
    {
        // Get the perpendicular vector to represent the directional velocity in plane
        // This is based on direction of angular velocity
        std::pair<float, float> perpendicularVec;
        if (this->angularVelocity > 0)
        {
            // Take perpendicular in clockwise direction
            perpendicularVec = std::make_pair(dispVec.second, (-1.0f) * dispVec.first);
        }
        else
        {
            // Take perpendicular in counterclockwise direction
            perpendicularVec = std::make_pair((-1.0f) * dispVec.second, dispVec.first);
        }

        // Now use rations logic to split the velocityTrans into x and y components as
        // the perpendicular vector and velocity vector must represent same thing
        float velComp1 = velocityTrans * perpendicularVec.first /
            std::sqrt(std::pow(perpendicularVec.first, 2) + std::pow(perpendicularVec.second, 2));
        float velComp2 = velocityTrans * perpendicularVec.second /
            std::sqrt(std::pow(perpendicularVec.first, 2) + std::pow(perpendicularVec.second, 2));
        return std::make_pair(velComp1, velComp2);
    }

    /*
    * Function to compute the translation velocity
    * from the angular velocity
    *
    * Using the reverse logic of function: computerVelocityAngularFromTranslation
    * References: https://en.wikipedia.org/wiki/Rotational_energy
    *
    * NOTE: No need to call thread safe functions here as caller uses global
    * thread safety already
    */
    void computerVelocityTranslationFromAngular()
    {
        // Computing the translational velocity
        float velocityTrans = this->angularVelocity * std::sqrt(this->getMomentInertia() / this->mass);
        // Scale to keep within range mentioned in project PDF
        if (velocityTrans < 2.0f)
        {
            velocityTrans = 5.0f;
        }
        // Computing the displacement vector from the sphere center to x, y, z point of UAV
        float dispVecX = this->x - sphereCenterX;
        float dispVecY = this->y - sphereCenterY;
        float dispVecZ = this->z - sphereCenterZ;

        float xVel = 0, yVel = 0, zVel = 0;

        // Computation if motion in XY plane
        if (this->motionDirection == XYPlane)
        {
            // Get vector of position displacement from 0, 0, 50
            std::pair<float, float> dispVec = std::make_pair(dispVecX, dispVecY);
            std::pair<float, float> velComps = this->computerVelocityComponents(dispVec, velocityTrans);
            xVel = velComps.first;
            yVel = velComps.second;
            zVel = 0.0f;
        }
        // Computation if motion in YZ plane
        else if (this->motionDirection == YZPlane)
        {
            // Get vector of position displacement from 0, 0, 50
            std::pair<float, float> dispVec = std::make_pair(dispVecY, dispVecZ);
            std::pair<float, float> velComps = this->computerVelocityComponents(dispVec, velocityTrans);
            yVel = velComps.first;
            zVel = velComps.second;
            xVel = 0.0f;
        }
        // Computation if motion in ZX plane
        else if (this->motionDirection == ZXPlane)
        {
            // Get vector of position displacement from 0, 0, 50
            std::pair<float, float> dispVec = std::make_pair(dispVecZ, dispVecX);
            std::pair<float, float> velComps = this->computerVelocityComponents(dispVec, velocityTrans);
            zVel = velComps.first;
            xVel = velComps.second;
            yVel = 0.0f;
        }
        // Check the sign of the velocity to ensure it is outward from the centre
        if (dispVecX != 0 && xVel * dispVecX < 0)
        {
            xVel *= -1;
        }
        if (dispVecY != 0 && yVel * dispVecY < 0)
        {
            yVel *= -1;
        }
        if (dispVecZ != 0 && zVel * dispVecZ < 0)
        {
            zVel *= -1;
        }
        // Check if the new velocity is within the max velocity limit
        while (std::sqrt(std::pow(xVel, 2) + std::pow(yVel, 2) + std::pow(zVel, 2)) > maxUAVvelocity)
        {
            // If it exceed the max velocity limit, then update the vx, vy and vz values
            // to bring the total velocity within the limit
            if (xVel > 0)
            {
                xVel -= 0.1;
            }
            else
            {
                xVel += 0.1;
            }
            if (yVel > 0)
            {
                yVel -= 0.1;
            }
            else
            {
                yVel += 0.1;
            }
            if (zVel > 0)
            {
                zVel -= 0.1;
            }
            else
            {
                zVel += 0.1;
            }
        }
        this->vx = xVel;
        this->vy = yVel;
        this->vz = zVel;
    }
};

/*
* Function to act upon UAV collision
* @param uav1 -> pointer to the first UAV
* @param uav2 -> pointer to the second UAV
*/
void uavCollision(ECE_UAV* uav1, ECE_UAV* uav2)
{
    mtxUAVUpdate[uav1->uavID].lock();
    mtxUAVUpdate[uav2->uavID].lock();

    // Exchange angular velocities
    std::swap(uav1->angularVelocity, uav2->angularVelocity);
    // Exchange the motion directions
    std::swap(uav1->motionDirection, uav2->motionDirection);

    // Compute the translation velocity from the angular velocity
    // of the UAVs
    uav1->computerVelocityTranslationFromAngular();
    uav2->computerVelocityTranslationFromAngular();
    // Change the motion types to linear
    uav1->motion = linear;
    uav2->motion = linear;
    // Move there centres outside the collision range to avoid double counting
    if (uav1->x < uav2->x)
    {
        uav1->x -= 0.2;
        uav2->x += 0.2;
    }
    else
    {
        uav1->x += 0.2;
        uav2->x -= 0.2;
    }
    if (uav1->y < uav2->y)
    {
        uav1->y -= 0.2;
        uav2->y += 0.2;
    }
    else
    {
        uav1->y += 0.2;
        uav2->y -= 0.2;
    }
    if (uav1->y < uav2->y)
    {
        uav1->y -= 0.2;
        uav2->y += 0.2;
    }
    else
    {
        uav1->y += 0.2;
        uav2->y -= 0.2;
    }

    mtxUAVUpdate[uav1->uavID].unlock();
    mtxUAVUpdate[uav2->uavID].unlock();
}

/*
* Function for thread functionality to update the location of the UAV every 10ms
* @param pUAV -> pointer to the UAV
*/
void threadFunction(ECE_UAV* pUAV)
{
    // UAV stay in the starting positions for 5 seconds
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // Post that, they start in linear motion towards the virutal sphereaccounting
    // for all forces

    // Loop run till the global atomic of closing threads not set
    while (atmCloseThread != true)
    {
        // Update the UAV location every 10 ms
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        // Update the position of the UAV
        pUAV->updatePosition(0.01f);
    }
}

/*
* Function to convert meter to yard
* @param inLength => length in meters
* @return float => length in yards
*/
float convertMeterToYard(float inLength)
{
    return (1.09361 * inLength);
}

/*
* Main function of program
*/
int main(void)
{
    // Init glfw
    if (!glfwInit())
    {
        // If unable to init, print failure and exit
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(2048, 1536, "ECE 6122: Gatech Buzzy Bowl", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    glDisable(GL_CULL_FACE);

    // Enable 2D textures in openGL
    glEnable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);

    // Create vertex array
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    // Shaders used for displaying the football field
    GLuint programID1 = LoadShaders("TransformVertexShader.vertexshader", "TextureFragmentShader.fragmentshader");
    // Shaders used for displaying the virtual sphere
    GLuint programID2 = LoadShaders("TransformVertexShader.vertexshader", "StandardTransparentShading.fragmentshader");
    // Shaders used for displaying the UAVs
    GLuint programID3 = LoadShaders("TransformVertexShader.vertexshader", "TextureFragmentShaderOscillate.fragmentshader");

    // Get a handle for our "MVP" uniform for all programs
    GLuint MatrixID = glGetUniformLocation(programID1, "MVP");
    GLuint ViewMatrixID = glGetUniformLocation(programID1, "V");
    GLuint ModelMatrixID = glGetUniformLocation(programID1, "M");
    GLuint MatrixID2 = glGetUniformLocation(programID2, "MVP");
    GLuint ViewMatrixID2 = glGetUniformLocation(programID2, "V");
    GLuint ModelMatrixID2 = glGetUniformLocation(programID2, "M");
    GLuint MatrixID3 = glGetUniformLocation(programID2, "MVP");
    GLuint ViewMatrixID3 = glGetUniformLocation(programID2, "V");
    GLuint ModelMatrixID3 = glGetUniformLocation(programID2, "M");

    // Projection matrix : 45° Field of View
    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 200.0f);
    // Camera matrix
    // Order of arguments: Camera location in world space, Camera look at, Head up location
    glm::mat4 View = glm::lookAt(glm::vec3(0, 150, 30), glm::vec3(0, 0, 25), glm::vec3(0, 0, 1));

    // Handle for the MVP
    glm::mat4 Model = glm::mat4(1.0f);
    glm::mat4 MVP = Projection * View * Model;

    // Traingles to represent the location of the field on screen
    // The locations also account for the 7 pixel displacement in the image
    // Points are decided such that 10 units on screen represent 10m
    static const GLfloat gVertexBufferDataField[] = {
        convertMeterToYard(48.0f + 17.0f), convertMeterToYard(25.0f + 10.0f), convertMeterToYard(0.0f),
        convertMeterToYard(48.0f + 17.0f), convertMeterToYard(-25.0f - 10.0f), convertMeterToYard(0.0f),
        convertMeterToYard(-52.0f - 17.0f), convertMeterToYard(25.0f + 10.0f), convertMeterToYard(0.0f),
        convertMeterToYard(48.0f + 17.0f), convertMeterToYard(-25.0f - 10.0f), convertMeterToYard(0.0f),
        convertMeterToYard(-52.0f - 17.0f), convertMeterToYard(25.0f + 10.0f), convertMeterToYard(0.0f),
        convertMeterToYard(-52.0f - 17.0f), convertMeterToYard(-25.0f - 10.0f), convertMeterToYard(0.0f),
    };

    // Traingle texture mapping of the image onto the screen
    static const GLfloat gUVBufferDataField[] = {
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
    };

    // Vertex buffer for the football field
    GLuint vertexbufferField;
    glGenBuffers(1, &vertexbufferField);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbufferField);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gVertexBufferDataField), gVertexBufferDataField, GL_STATIC_DRAW);

    // UV buffer for the football field
    GLuint uvbufferField;
    glGenBuffers(1, &uvbufferField);
    glBindBuffer(GL_ARRAY_BUFFER, uvbufferField);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gUVBufferDataField), gUVBufferDataField, GL_STATIC_DRAW);

    // Reference sphere obj file downloaded from:
    // http://web.mit.edu/djwendel/www/weblogo/shapes/basic-shapes/sphere/sphere.obj

    // Vertices for the virtual sphere from the obj
    std::vector<glm::vec3> verticesSphere;
    // UV points for the virtual sphere from the obj
    std::vector<glm::vec2> uvsSphere;
    // Normals for the virtual sphere from the obj
    std::vector<glm::vec3> normalsSphere;
    // Load the sphere obj
    bool res = loadOBJ("sphere.obj", verticesSphere, uvsSphere, normalsSphere);

    // Loading all the variables from the VBO
    std::vector<unsigned short> indicesSphere;
    std::vector<glm::vec3> indexedVerticesSphere;
    std::vector<glm::vec2> indexedUVsSphere;
    std::vector<glm::vec3> indexedNormalsSphere;
    indexVBO(verticesSphere, uvsSphere, normalsSphere, indicesSphere, indexedVerticesSphere, indexedUVsSphere, indexedNormalsSphere);

    GLuint vertexbufferSphere;
    glGenBuffers(1, &vertexbufferSphere);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbufferSphere);
    glBufferData(GL_ARRAY_BUFFER, indexedVerticesSphere.size() * sizeof(glm::vec3), &indexedVerticesSphere[0], GL_STATIC_DRAW);

    GLuint uvbufferSphere;
    glGenBuffers(1, &uvbufferSphere);
    glBindBuffer(GL_ARRAY_BUFFER, uvbufferSphere);
    glBufferData(GL_ARRAY_BUFFER, indexedUVsSphere.size() * sizeof(glm::vec2), &indexedUVsSphere[0], GL_STATIC_DRAW);

    GLuint normalbufferSphere;
    glGenBuffers(1, &normalbufferSphere);
    glBindBuffer(GL_ARRAY_BUFFER, normalbufferSphere);
    glBufferData(GL_ARRAY_BUFFER, indexedNormalsSphere.size() * sizeof(glm::vec3), &indexedNormalsSphere[0], GL_STATIC_DRAW);

    GLuint elementbufferSphere;
    glGenBuffers(1, &elementbufferSphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbufferSphere);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSphere.size() * sizeof(unsigned short), &indicesSphere[0], GL_STATIC_DRAW);

    // Vertics for the UAV
    std::vector<glm::vec3> verticesUAV;
    // UV points for the UAV
    std::vector<glm::vec2> uvsUAV;
    // Normals for the UAV
    std::vector<glm::vec3> normalsUAV;
    // Loading the suzzane object as the UAV
    res = loadOBJ("suzanne.obj", verticesUAV, uvsUAV, normalsUAV);

    std::vector<unsigned short> indicesUAV;
    std::vector<glm::vec3> indexedVerticesUAV;
    std::vector<glm::vec2> indexedUVsUAV;
    std::vector<glm::vec3> indexedNormalsUAV;
    indexVBO(verticesUAV, uvsUAV, normalsUAV, indicesUAV, indexedVerticesUAV, indexedUVsUAV, indexedNormalsUAV);

    GLuint vertexbufferUAV;
    glGenBuffers(1, &vertexbufferUAV);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbufferUAV);
    glBufferData(GL_ARRAY_BUFFER, verticesUAV.size() * sizeof(glm::vec3), &verticesUAV[0], GL_STATIC_DRAW);

    GLuint uvbufferUAV;
    glGenBuffers(1, &uvbufferUAV);
    glBindBuffer(GL_ARRAY_BUFFER, uvbufferUAV);
    glBufferData(GL_ARRAY_BUFFER, uvsUAV.size() * sizeof(glm::vec2), &uvsUAV[0], GL_STATIC_DRAW);

    GLuint normalbufferUAV;
    glGenBuffers(1, &normalbufferUAV);
    glBindBuffer(GL_ARRAY_BUFFER, normalbufferUAV);
    glBufferData(GL_ARRAY_BUFFER, normalsUAV.size() * sizeof(glm::vec3), &normalsUAV[0], GL_STATIC_DRAW);

    // Texture for the football field from the bmp file
    GLuint Texture = loadBMP_custom("ff.bmp");
    GLuint TextureID = glGetUniformLocation(programID1, "myTextureSampler");
    // Texture for the 3D suzzane object from the DDS file (contained in the tutorial)
    GLuint Texture1 = loadDDS("suzanne.DDS");
    GLuint TextureID1 = glGetUniformLocation(programID3, "myTextureSampler");

    // List to hold pointers to the UAVs
    std::array<ECE_UAV*, 15> uavList;
    for (int i = 0; i < 3; ++i)
    {
        // The X location of the leftmost UAV
        float initX = 50.0;
        for (int j = 0; j < 5; ++j)
        {
            int currUAVIndex = i * 5 + j;
            // Set the initial location of the uavs
            float initX = convertMeterToYard(50 - j * 25);
            float initY = convertMeterToYard(30 - i * 30);
            float initZ = convertMeterToYard(1);
            // Set the motion direction of the UAV in a given plane
            // This ensures that even though randomized, they UAVs
            // don't all fall in the same initial plane
            //motionPlane initDirection = static_cast<motionPlane>(randDistribution(randGenerator));
            motionPlane initDirection = static_cast<motionPlane>(currUAVIndex % 3);
            // Create the object for the uav
            ECE_UAV* uav = new ECE_UAV(currUAVIndex, initX, initY, initZ, linear, initDirection);
            // Add to the list
            uavList[currUAVIndex] = uav;
        }
    }

    // Color Intensity scaling value of the UAV
    float colorIntensityValue = 0.6;
    // Color Intensity delta change 
    float colorIntensityDelta = 0.01;

    // Variable to hold the time when all UAVs reach sphere
    std::chrono::steady_clock::time_point timeUAVReachSphere;
    // Variable to check whether the time has been recorded when all UAV
    // reach the sphere
    bool timeRecorded = false;

    // Main loop for the openGL rendering
    do {
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use shader for football field
        glUseProgram(programID1);

        // Send transformation to the currently bound shader, in the "MVP" uniform
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

        // Bind our texture in Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
        // Set our "myTextureSampler" sampler to use Texture Unit 0
        glUniform1i(TextureID, 0);

        // Load the vertex buffer
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbufferField);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // Load the uv buffer
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uvbufferField);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // Draw the triangles for the football field
        glDrawArrays(GL_TRIANGLES, 0, 2 * 3);

        // Loop through all uavs
        for (int i = 0; i < 15; ++i)
        {
            ECE_UAV* uav = uavList[i];
            // Load the specific shaders
            glUseProgram(programID3);

            // Get the color intensity of the uav
            GLint uavColorIntensityVal = glGetUniformLocation(programID3, "colorIntensity");
            // Use the sin function to oscillate the color intensity of the uav
            // Set the value in the fragment shader
            glUniform1f(uavColorIntensityVal, colorIntensityValue);

            // Use the texture for UAV
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, Texture1);
            glUniform1i(TextureID1, 1);

            // Setup the MVP for the UAV
            glm::mat4 ModelMatrixUAV = glm::mat4(1.0);
            // Translate the location based on updates done in the object
            ModelMatrixUAV = glm::translate(ModelMatrixUAV, glm::vec3(uav->getXPos(), uav->getYPos(), uav->getZPos()));
            // Rotate the UAV to display the front side towards the camera
            ModelMatrixUAV = glm::rotate(ModelMatrixUAV, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 1.0f));
            // Scale the UAV object size as per project PDF requirement
            ModelMatrixUAV = glm::scale(ModelMatrixUAV, glm::vec3(0.75f));
            // Get the updated MVP
            glm::mat4 MVP3 = Projection * View * ModelMatrixUAV;

            glUniformMatrix4fv(MatrixID3, 1, GL_FALSE, &MVP3[0][0]);
            glUniformMatrix4fv(ModelMatrixID3, 1, GL_FALSE, &ModelMatrixUAV[0][0]);
            glUniformMatrix4fv(ViewMatrixID3, 1, GL_FALSE, &View[0][0]);

            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, vertexbufferUAV);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, uvbufferUAV);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

            glEnableVertexAttribArray(2);
            glBindBuffer(GL_ARRAY_BUFFER, normalbufferUAV);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

            glDrawArrays(GL_TRIANGLES, 0, verticesUAV.size());

            glDisableVertexAttribArray(0);
            glDisableVertexAttribArray(1);
            glDisableVertexAttribArray(2);
        }

        // Use shader for the virtual sphere
        glUseProgram(programID2);

        // Setu pthe MVP for the virtual sphere
        glm::mat4 ModelMatrixSphere = glm::mat4(1.0);
        // Move the sphere to the required position on the screen
        ModelMatrixSphere = glm::translate(ModelMatrixSphere, glm::vec3(sphereCenterX, sphereCenterY, sphereCenterZ));
        // Scale the sphere so that it acts as a reference for UAV motion
        ModelMatrixSphere = glm::scale(ModelMatrixSphere, glm::vec3(3.5f));
        // Generate the MVP for the sphere
        glm::mat4 MVP2 = Projection * View * ModelMatrixSphere;

        glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP2[0][0]);
        glUniformMatrix4fv(ModelMatrixID2, 1, GL_FALSE, &ModelMatrixSphere[0][0]);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbufferSphere);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uvbufferSphere);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, normalbufferSphere);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbufferSphere);

        glDrawElements(GL_TRIANGLES, indicesSphere.size(), GL_UNSIGNED_SHORT, (void*)0);

        // Check for collisions

        // Tracker to check if the uav has already collided to avoid
        // counting multi object collisions in same rendering flow
        bool hasUavCollided[15] = { false };

        // Flag to set if all uavs reached virtual sphere
        bool hasUavReachedSphere = true;

        for (int i = 0; i < 15; ++i)
        {
            ECE_UAV* uav1 = uavList[i];
            if (hasUavCollided[i] == true || uav1->canCollidedAgain() == false)
            {
                continue;
            }
            // Get the uav coordinates to avoid locking the mutex again and again
            float uav1XPos = uav1->getXPos();
            float uav1YPos = uav1->getYPos();
            float uav1ZPos = uav1->getZPos();
            for (int j = i + 1; j < 15; ++j)
            {
                ECE_UAV* uav2 = uavList[j];
                if (hasUavCollided[j] == true || uav2->canCollidedAgain() == false)
                {
                    continue;
                }
                if (uav2->checkCollision(uav1XPos, uav1YPos, uav1ZPos))
                {
                    // If collided, call the collision handling function
                    uavCollision(uav1, uav2);
                    hasUavCollided[i] = true;
                    hasUavCollided[j] = true;
                    uav1->setCollisionCounter();
                    uav2->setCollisionCounter();;
                }
            }
            if (uav1->isUavOnSphere() == false)
            {
                hasUavReachedSphere = false;
            }
        }

        // Check if all UAV have reached sphere
        if (timeRecorded == false && hasUavReachedSphere == true)
        {
            timeUAVReachSphere = std::chrono::steady_clock::now();
            timeRecorded = true;
        }

        std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
        // Check if 60 seconds elapsed
        if (timeRecorded == true &&
            static_cast<float>(std::chrono::duration_cast<std::chrono::seconds>(currentTime - timeUAVReachSphere).count()) >= 60.0f)
        {
            break;
        }

        if (colorIntensityValue > 0.99f || colorIntensityValue < 0.51f)
        {
            colorIntensityDelta *= -1;
        }
        colorIntensityValue += colorIntensityDelta;

        std::this_thread::sleep_for(std::chrono::milliseconds(15));

        for (int i = 0; i < 15; ++i)
        {
            uavList[i]->decrementCollisionCounter();
        }

        // Swap buffers to the main window
        glfwSwapBuffers(window);
        // Poll for events
        glfwPollEvents();

    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

    // Clear all the initialized buffers
    glDeleteBuffers(1, &vertexbufferField);
    glDeleteBuffers(1, &uvbufferField);
    glDeleteBuffers(1, &vertexbufferSphere);
    glDeleteBuffers(1, &uvbufferSphere);
    glDeleteBuffers(1, &normalbufferSphere);
    glDeleteBuffers(1, &elementbufferSphere);
    glDeleteBuffers(1, &vertexbufferUAV);
    glDeleteBuffers(1, &uvbufferUAV);
    glDeleteBuffers(1, &normalbufferUAV);
    glDeleteProgram(programID1);
    glDeleteProgram(programID2);
    glDeleteProgram(programID3);
    glDeleteTextures(1, &Texture);
    glDeleteTextures(1, &Texture1);
    glDeleteVertexArrays(1, &VertexArrayID);

    // Terminate
    glfwTerminate();

    // Set the close thread flag
    // Important to ensure the threadFunction finishes operation
    // cleanly before join is called, else join would become blocking
    atmCloseThread = true;
    for (auto uav : uavList)
    {
        uav->joinThread();
    }

    // Delete the UAV objects
    for (auto uav : uavList)
    {
        delete uav;
    }

    return 0;
}

