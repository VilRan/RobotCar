#include "msp430x20x3.h"
#include <stdbool.h>

// Left Motor Port: 1.2 => Address: 0000 0001
#define LEFT_MOTOR_ADDRESS 0x04

// Right Motor Port: 2.0 => Address: 0000 0100
#define RIGHT_MOTOR_ADDRESS 0x01

// Left Sensor Port: 1.3 => Address: 0000 1000
#define RIGHT_SENSOR_ADDRESS 0x08

// Middle Sensor Port: 1.4  => Address: 0001 0000
#define MIDDLE_SENSOR_ADDRESS 0x10

// Right Sensor Port: 1.5 => Address: 0010 0000
#define LEFT_SENSOR_ADDRESS 0x20

void StopWatchdog()
{
  WDTCTL = WDTPW + WDTHOLD;
}

void StartLeftMotor()
{
  P1OUT = LEFT_MOTOR_ADDRESS;
}

void StartRightMotor()
{
  P2OUT = RIGHT_MOTOR_ADDRESS;
}

void StopLeftMotor()
{
  P1OUT = 0;
}

void StopRightMotor()
{
  P2OUT = 0;
}

void Initialize()
{
  StartLeftMotor();
  StartRightMotor();
  P1DIR = LEFT_MOTOR_ADDRESS;
  P2DIR = RIGHT_MOTOR_ADDRESS;
}

void Stop()
{
  StopLeftMotor();
  StopRightMotor();
}

void DriveForward()
{
  StartLeftMotor();
  StartRightMotor();
}

void TurnRight()
{
  StartLeftMotor();
  StopRightMotor();
}

void TurnLeft()
{
  StopLeftMotor();
  StartRightMotor();
}

bool ReadLeftSensor()
{
  return P1IN & LEFT_SENSOR_ADDRESS;
}

bool ReadMiddleSensor()
{
  return P1IN & MIDDLE_SENSOR_ADDRESS;
}

bool ReadRightSensor()
{
  return P1IN & RIGHT_SENSOR_ADDRESS;
}


main()
{
  StopWatchdog();
  Initialize();
  while (true)
  {
    bool middleSensorDetectsBlack = ReadMiddleSensor();
    bool leftSensorDetectsBlack = ReadLeftSensor();
    bool rightSensorDetectsBlack = ReadRightSensor();
    
    if (middleSensorDetectsBlack)
    {
      DriveForward();
    }
    else if (leftSensorDetectsBlack)
    {
      TurnRight();
    }
    else if (rightSensorDetectsBlack)
    {
      TurnLeft();
    }
    else
    {
      Stop();
    }
  }
}