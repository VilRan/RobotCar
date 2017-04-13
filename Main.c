#include "msp430g2553.h"
#include <stdio.h>
#include "Main.h"

main()
{
  StopWatchdog();
  Initialize();
  
  while (1)
  {
    /*
    LeftSensorDetectsBlack = !(P1IN & LEFT_SENSOR_ADDRESS);
    MiddleSensorDetectsBlack = !(P1IN & MIDDLE_SENSOR_ADDRESS);
    RightSensorDetectsBlack = !(P1IN & RIGHT_SENSOR_ADDRESS);
    */
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

void StopWatchdog()
{
  WDTCTL = WDTPW + WDTHOLD;
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
  
  ADC10CTL1 = INCH_10 + ADC10DIV_3; // Temp Sensor ADC10CLK/4
  ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON + ADC10IE;
  __enable_interrupt();
  
  Stop();
}

void Standby()
{
  SetLeftMotorSpeed(0x00);
  SetRightMotorSpeed(0x00);
  ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
  //__bis_SR_register(CPUOFF + GIE);        // LPM0 with interrupts enabled
}

void DriveForward()
{
  SetLeftMotorSpeed(0xFF);
  SetRightMotorSpeed(0xFF);
  
  if (MiddleSensorDetectsBlack)
  {
    if (LeftSensorDetectsBlack && RightSensorDetectsBlack)
    {
      State = STATE_COMPLETE_LAP;
      Laps++;
    }
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

void TurnRight()
{
  SetLeftMotorSpeed(0xCC);
  SetRightMotorSpeed(0x00);
  
  if (MiddleSensorDetectsBlack)
  {
    State = STATE_DRIVE_FORWARD;
  }/*
  else if (LeftSensorDetectsBlack)
  {
    State = STATE_TURN_LEFT;
  }*/
}

void TurnLeft()
{
  SetLeftMotorSpeed(0x00);
  SetRightMotorSpeed(0xCC);
  
  if (MiddleSensorDetectsBlack)
  {
    State = STATE_DRIVE_FORWARD;
  }/*
  else if (RightSensorDetectsBlack)
  {
    State = STATE_TURN_RIGHT;
  }*/
}

void Stop()
{
  SetLeftMotorSpeed(0x88);
  SetRightMotorSpeed(0x88);
}

void CompleteLap()
{  
  if (Laps == 3)
  {
    State = STATE_STOPPED;
    return;
  }
  
  SetLeftMotorSpeed(0xFF);
  SetRightMotorSpeed(0xFF);
  
  if (MiddleSensorDetectsBlack)
  {
    if (!LeftSensorDetectsBlack)
    {
      if (!RightSensorDetectsBlack)
      {
        State = STATE_DRIVE_FORWARD;
      }
      else
      {
        SetRightMotorSpeed(0x00);
      }
    }
    else if (!RightSensorDetectsBlack)
    {
      SetLeftMotorSpeed(0x00);
    }
  }
  else
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
  if(temp > 775)
  {
    State = STATE_DRIVE_FORWARD;
  }
  __bic_SR_register_on_exit(CPUOFF); // Clear CPUOFF bit from 0(SR)
}