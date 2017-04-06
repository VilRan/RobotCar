#include "msp430g2553.h"
#include <stdio.h>

// Left Motor Port: 1.2 => Address: 0000 0100
#define LEFT_MOTOR_ADDRESS 0x04

// Right Motor Port: 2.0 => Address: 0000 0010
#define RIGHT_MOTOR_ADDRESS 0x02

// Right Sensor Port: 1.3 => Address: 0000 1000
#define RIGHT_SENSOR_ADDRESS 0x08

// Middle Sensor Port: 1.4  => Address: 0001 0000
#define MIDDLE_SENSOR_ADDRESS 0x10

// Left Sensor Port: 1.5 => Address: 0010 0000
#define LEFT_SENSOR_ADDRESS 0x20

#define STATE_STANDBY 0
#define STATE_DRIVE_FORWARD 1
#define STATE_TURN_LEFT 2
#define STATE_TURN_RIGHT 3
#define STATE_COMPLETE_LAP 4
#define STATE_STOPPED 5

int State = STATE_DRIVE_FORWARD;
int LastState = STATE_DRIVE_FORWARD;
int Laps = 0;
int MiddleSensorDetectsBlack = 0;
int LeftSensorDetectsBlack = 0;
int RightSensorDetectsBlack = 0;

void StopWatchdog()
{
  WDTCTL = WDTPW + WDTHOLD;
}

void SetLeftMotorSpeed(int speed)
{
  TA0CCR1 = speed;
}

void SetRightMotorSpeed(int speed)
{  
  TA1CCR1 = speed;
}

int ReadLeftSensor()
{
  //return P1IN & LEFT_SENSOR_ADDRESS;
  return P1IFG & LEFT_SENSOR_ADDRESS;
}

int ReadMiddleSensor()
{
  //return P1IN & MIDDLE_SENSOR_ADDRESS;
  return P1IFG & MIDDLE_SENSOR_ADDRESS;
}

int ReadRightSensor()
{
  //return P1IN & RIGHT_SENSOR_ADDRESS;
  return P1IFG & RIGHT_SENSOR_ADDRESS;
}
/*
void ReadSensors()
{
  LastState = State;
  
  if (
      MiddleSensorDetectsBlack 
      && LeftSensorDetectsBlack 
      && RightSensorDetectsBlack
  )
  {
    State = STATE_COMPLETE_LAP;
  }
  else if (MiddleSensorDetectsBlack)
  {
    State = STATE_DRIVE_FORWARD;
  }
  else if (LeftSensorDetectsBlack)
  {
    State = STATE_TURN_LEFT;
  }
  else if (RightSensorDetectsBlack)
  {
    State = STATE_TURN_RIGHT;
  }
}
*/
void Standby()
{
  ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
  //__bis_SR_register(CPUOFF + GIE);        // LPM0 with interrupts enabled
}

void DriveForward()
{
  SetLeftMotorSpeed(0xFF);
  SetRightMotorSpeed(0xFF);
  //ReadSensors();
  
  if (LeftSensorDetectsBlack)
  {
    if (RightSensorDetectsBlack)
    {
      State = STATE_COMPLETE_LAP;
      Laps++;
    }
    else
    {
      State = STATE_TURN_LEFT;
    }
  }
  else if (RightSensorDetectsBlack)
  {
    State = STATE_TURN_RIGHT;
  }
}

void TurnRight()
{
  SetLeftMotorSpeed(0xFF);
  SetRightMotorSpeed(0x00);
  //ReadSensors();
  
  if (MiddleSensorDetectsBlack)
  {
    State = STATE_DRIVE_FORWARD;
  }
}

void TurnLeft()
{
  SetLeftMotorSpeed(0x00);
  SetRightMotorSpeed(0xFF);
  //ReadSensors();
  
  if (MiddleSensorDetectsBlack)
  {
    State = STATE_DRIVE_FORWARD;
  }
}

void Stop()
{
  SetLeftMotorSpeed(0x00);
  SetRightMotorSpeed(0x00);
}

void CompleteLap()
{
  if (Laps == 3)
  {
    State = STATE_STOPPED;
  }
  
  if (MiddleSensorDetectsBlack == 0)
  {
    if (LeftSensorDetectsBlack)
    {
      State = STATE_TURN_LEFT;
    }
    else if (RightSensorDetectsBlack)
    {
      State = STATE_TURN_RIGHT;
    }
  }
}

void Initialize()
{
  P1DIR = LEFT_MOTOR_ADDRESS;
  P2DIR = RIGHT_MOTOR_ADDRESS;
  
  P1DIR |= 0x01;
  P1OUT |= RIGHT_SENSOR_ADDRESS + MIDDLE_SENSOR_ADDRESS + LEFT_SENSOR_ADDRESS;
  P1REN |= RIGHT_SENSOR_ADDRESS + MIDDLE_SENSOR_ADDRESS + LEFT_SENSOR_ADDRESS;
  //P1IFG = 0x01;
  P1IES |= RIGHT_SENSOR_ADDRESS + MIDDLE_SENSOR_ADDRESS + LEFT_SENSOR_ADDRESS;
  P1IE |= RIGHT_SENSOR_ADDRESS + MIDDLE_SENSOR_ADDRESS + LEFT_SENSOR_ADDRESS;
  _BIS_SR(/*LPM4_bits +*/ GIE);
  
  TA0CTL = TASSEL_2 + ID_0 + MC_1; // SMCLK, NO DIVIDER, UP_MODE
  TA0CCTL1 = OUTMOD_7; // SET/RESET_MODE
  TA0CCR0 = 0xFF;
  TA0CCR1 = 0xF0;
  P1SEL = LEFT_MOTOR_ADDRESS;
  
  TA1CTL = TASSEL_2 + ID_0 + MC_1; // SMCLK, NO DIVIDER, UP_MODE
  TA1CCTL1 = OUTMOD_7; // SET/RESET_MODE
  TA1CCR0 = 0xFF;
  TA1CCR1 = 0xF0;
  P2SEL = RIGHT_MOTOR_ADDRESS;
  
  /*
  ADC10CTL1 = INCH_10 + ADC10DIV_3; // Temp Sensor ADC10CLK/4
  ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON + ADC10IE;
  __enable_interrupt();
  
  Stop();
  */
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
  if (ReadMiddleSensor())
  {
    //State = STATE_DRIVE_FORWARD;
    MiddleSensorDetectsBlack = P1IES & MIDDLE_SENSOR_ADDRESS;
    P1IES ^= MIDDLE_SENSOR_ADDRESS;
  }
  else if (ReadLeftSensor())
  {
    //State = STATE_TURN_LEFT;
    LeftSensorDetectsBlack = P1IES & LEFT_SENSOR_ADDRESS;
    P1IES ^= LEFT_SENSOR_ADDRESS;
  }
  else if (ReadRightSensor())
  {
    //State = STATE_TURN_RIGHT;
    RightSensorDetectsBlack = P1IES & RIGHT_SENSOR_ADDRESS;
    P1IES ^= RIGHT_SENSOR_ADDRESS;
  }
  
  P1IFG = 0x00;
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
  
  while (1)
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