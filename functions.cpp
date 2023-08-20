#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
#include <chrono>
#include <thread>
#include "Memory.cpp"
#include "Offsets.cpp"
#include <cmath>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

bool keyDown(int keyCode)
{
    Display *display = XOpenDisplay(NULL);
    char keys_return[32];
    XQueryKeymap(display, keys_return);
    KeyCode kc2 = XKeysymToKeycode(display, keyCode);
    bool buttonDown = !!(keys_return[kc2 >> 3] & (1 << (kc2 & 7)));
    XCloseDisplay(display);
    return buttonDown;
}


double calculateDistance2D(float x1, float y1, float x2, float y2)
{
    float dx = (x1 - x2);
    float dy = (y1 - y2);
    float distance = sqrt(pow(dx, 2) + pow(dy, 2));
    return distance;
}

double calculateDesiredYaw(
    double localPlayerLocationX,
    double localPlayerLocationY,
    double enemyPlayerLocationX,
    double enemyPlayerLocationY)
{
    const double locationDeltaX = enemyPlayerLocationX - localPlayerLocationX;
    const double locationDeltaY = enemyPlayerLocationY - localPlayerLocationY;
    const double yawInRadians = atan2(locationDeltaY, locationDeltaX);
    const double yawInDegrees = yawInRadians * (180 / M_PI);
    return yawInDegrees;
}

double calculateDesiredPitch(
    double localPlayerLocationX,
    double localPlayerLocationY,
    double localPlayerLocationZ,
    double enemyPlayerLocationX,
    double enemyPlayerLocationY,
    double enemyPlayerLocationZ)
{
    const double locationDeltaZ = enemyPlayerLocationZ - localPlayerLocationZ;
    const double distanceBetweenPlayers = calculateDistance2D(enemyPlayerLocationX, enemyPlayerLocationY, localPlayerLocationX, localPlayerLocationY);
    const double pitchInRadians = atan2(-locationDeltaZ, distanceBetweenPlayers);
    const double pitchInDegrees = pitchInRadians * (180 / M_PI);
    return pitchInDegrees;
}



double calculatePitchAngleDelta(double oldAngle, double newAngle)
{
    double wayA = newAngle - oldAngle;
    return wayA;
}

double calculateAngleDelta(double oldAngle, double newAngle)
{
    double wayA = newAngle - oldAngle;
    double wayB = 360 - abs(wayA);
    if (wayA > 0 && wayB > 0)
        wayB *= -1;
    if (abs(wayA) < abs(wayB))
        return wayA;
    return wayB;
}

double flipYawIfNeeded(double angle)
    {
        double myAngle = angle;
        if (myAngle > 180)
            myAngle = (360 - myAngle) * -1;
        else if (myAngle < -180)
            myAngle = (360 + myAngle);
        return myAngle;
    }

double flipPitchIfNeeded(double angle)
{
    double myAngle = angle;
    if (myAngle > 90)
        myAngle = (90 - (myAngle - 90));
    else if (myAngle < -90)
        myAngle = (-90 + (-90 - myAngle));
    return myAngle;
}

struct Data {
    float pos;
    int index;
};

void selectionSort(float* array, int size) {
  // Initialize the minimum index
  int minIndex = 0;

  // Iterate through the array
  for (int i = 0; i < size - 1; i++) {
    // Find the smallest element in the unsorted subarray
    minIndex = i;
    for (int j = i + 1; j < size; j++) {
      if (array[j] < array[minIndex]) {
        minIndex = j;
      }
    }

    // Swap the smallest element with the element at index i
    float temp = array[i];
    array[i] = array[minIndex];
    array[minIndex] = temp;
  }
}
