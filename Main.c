#include "msp430g2553.h"
#include <stdbool.h>
#include <stdio.h>

// Left Motor Port: 1.2 => Address: 0000 0001
#define LEFT_MOTOR_ADDRESS 0x04

// Right Motor Port: 2.0 => Address: 0000 0100
#define RIGHT_MOTOR_ADDRESS 0x01

// Right Sensor Port: 1.3 => Address: 0000 1000
#define RIGHT_SENSOR_ADDRESS 0x08
#define RIGHT_SENSOR 1

// Middle Sensor Port: 1.4  => Address: 0001 0000
#define MIDDLE_SENSOR_ADDRESS 0x10

// Left Sensor Port: 1.5 => Address: 0010 0000
#define LEFT_SENSOR_ADDRESS 0x20
#define LEFT_SENSOR 2

#define NO_SENSOR 0
#define ALL_SENSORS 3

#define STATE_STANDBY 0
#define STATE_DRIVE_FORWARD 1
#define STATE_TURN_LEFT 2
#define STATE_TURN_RIGHT 3
#define STATE_COMPLETE_LAP 4
#define STATE_STOPPED 5

int State = STATE_STANDBY;
int LastState = STATE_DRIVE_FORWARD;
int Laps = 0;

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
  P1SEL = 0;
}

void StopRightMotor()
{
  P2OUT = 0;
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

void ReadSensors()
{
  LastState = State;
  
  bool middleSensorDetectsBlack = !ReadMiddleSensor();
  bool leftSensorDetectsBlack = !ReadLeftSensor();
  bool rightSensorDetectsBlack = !ReadRightSensor();
  
  if (
      middleSensorDetectsBlack 
      && leftSensorDetectsBlack 
      && rightSensorDetectsBlack
  )
  {
    State = STATE_COMPLETE_LAP;
  }
  else if (middleSensorDetectsBlack)
  {
    State = STATE_DRIVE_FORWARD;
  }
  else if (leftSensorDetectsBlack)
  {
    State = STATE_TURN_LEFT;
  }
  else if (rightSensorDetectsBlack)
  {
    State = STATE_TURN_RIGHT;
  }
}

void Standby()
{
  ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
  //__bis_SR_register(CPUOFF + GIE);        // LPM0 with interrupts enabled
}

void DriveForward()
{
  StartLeftMotor();
  StartRightMotor();
  P1SEL = 0;
  ReadSensors();
}

void TurnRight()
{
  StartLeftMotor();
  StopRightMotor();
  P1SEL = LEFT_MOTOR_ADDRESS;
  ReadSensors();
}

void TurnLeft()
{
  StopLeftMotor();
  StartRightMotor();
  ReadSensors();
}

void Stop()
{
  StopLeftMotor();
  StopRightMotor();
}

void CompleteLap()
{
  if (LastState != STATE_COMPLETE_LAP)
  {
    Laps++;
    //LastSensor = ALL_SENSORS;
  }
  
  if (Laps == 3)
  {
    State = STATE_STOPPED;
    Stop();
  }
  else
  {
    DriveForward();
  }
}

void Initialize()
{
  P1DIR = LEFT_MOTOR_ADDRESS;
  P2DIR = RIGHT_MOTOR_ADDRESS;
  
  TA0CTL = TASSEL_2 + ID_0 + MC_1; // SMCLK, NO DIVIDER, UP_MODE
  TA0CCTL1 = OUTMOD_7; // SET/RESET_MODE
  TA0CCR0 = 0xFF;
  TA0CCR1 = 0xCC;
  P1SEL = LEFT_MOTOR_ADDRESS;
  /*
  TA1CTL = TASSEL_2 + ID_0 + MC_1; // SMCLK, NO DIVIDER, UP_MODE
  TA1CCTL1 = OUTMOD_7; // SET/RESET_MODE
  TA1CCR0 = 0xFF;
  TA1CCR1 = 0xCC;
  P2SEL2 = RIGHT_MOTOR_ADDRESS;
  */
  ADC10CTL1 = INCH_10 + ADC10DIV_3; // Temp Sensor ADC10CLK/4
  ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON + ADC10IE;
  __enable_interrupt();
  
  Stop();
}

// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR (void) 
{
  int temp = ADC10MEM;
  if(temp > 770)
  {
    State = STATE_DRIVE_FORWARD;
  }
  __bic_SR_register_on_exit(CPUOFF); // Clear CPUOFF bit from 0(SR)
}

main()
{
  StopWatchdog();
  Initialize();
  
  while (true)
  {
    switch (State)
    {
    case STATE_STANDBY:
      Standby();
      break;
    case STATE_DRIVE_FORWARD:
      DriveForward();
      break;
    case STATE_TURN_LEFT:
      TurnLeft();
      break;
    case STATE_TURN_RIGHT:
      TurnRight();
      break;
    case STATE_COMPLETE_LAP:
      CompleteLap();
      break;
    case STATE_STOPPED:
      Stop();
      break;
    }

  }
}