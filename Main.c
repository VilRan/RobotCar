#include "msp430g2553.h"
#include <stdio.h>
#include "Main.h"

main()
{
  Initialize();
  while (1)
  {
    if (SuperState == SUPERSTATE_DRIVE_FROM_MEMORY)
    {
      State = ReadStateFromMemory();
    }
      
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
    case STATE_COMPLETE_LAP_TURN_LEFT:
      CompleteLap_TurnLeft();
      break;
    case STATE_COMPLETE_LAP_TURN_RIGHT:
      CompleteLap_TurnRight();
      break;
    case STATE_STOPPED:
      Stop();
      break;
    }
  }
}

void Initialize()
{ 
  WDTCTL = WDTPW + WDTHOLD;
  
  P1DIR = LEFT_MOTOR_ADDRESS + RIGHT_MOTOR_ADDRESS;
  P1OUT = 0;
  
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
 
  TA1CCTL0 = CCIE;
  TA1CTL = TASSEL_2 + ID_0 + MC_1; // SMCLK, NO DIVIDER, UP_MODE
  TA1CCTL1 = OUTMOD_7; // SET/RESET_MODE
  TA1CCR0 = 0xFF;
  TA1CCR1 = 0xF0;
  
  ADC10CTL1 = INCH_10 + ADC10DIV_3; // Temp Sensor ADC10CLK/4
  ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON + ADC10IE;
  __enable_interrupt();
  
  EraseFlash(FlashWriteAddress, 100);
}

void Standby()
{
  P1SEL = 0;
  TA0CCR1 = 0;
  P1OUT = 0;
  
  ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
  //__bis_SR_register(CPUOFF + GIE);        // LPM0 with interrupts enabled
}

void DriveForward()
{
  P1SEL = 0;
  TA0CCR1 = 0;
  P1OUT = LEFT_MOTOR_ADDRESS + RIGHT_MOTOR_ADDRESS;
  
  if (SuperState == SUPERSTATE_DRIVE_FROM_MEMORY)
  {
    return;
  }
  
  if (MiddleSensorDetectsBlack)
  {
    if (LeftSensorDetectsBlack && RightSensorDetectsBlack)
    {
      ChangeState(STATE_COMPLETE_LAP);
      Laps++;
    }
  }
  else if (LeftSensorDetectsBlack)
  {
    ChangeState(STATE_TURN_LEFT);
  }
  else if (RightSensorDetectsBlack)
  {
    ChangeState(STATE_TURN_RIGHT);
  }
}

void TurnRight()
{
  P1SEL = LEFT_MOTOR_ADDRESS;
  TA0CCR1 = 0xE0;
  P1OUT = 0;
  
  if (SuperState == SUPERSTATE_DRIVE_FROM_MEMORY)
  {
    return;
  }
  
  if (MiddleSensorDetectsBlack)
  {
    ChangeState(STATE_DRIVE_FORWARD);
  }/*
  else if (LeftSensorDetectsBlack)
  {
    State = STATE_TURN_LEFT;
  }*/
}

void TurnLeft()
{
  P1SEL = 0;
  TA0CCR1 = 0;
  P1OUT = RIGHT_MOTOR_ADDRESS;
  
  if (SuperState == SUPERSTATE_DRIVE_FROM_MEMORY)
  {
    return;
  }
  
  if (MiddleSensorDetectsBlack)
  {
    ChangeState(STATE_DRIVE_FORWARD);
  }/*
  else if (RightSensorDetectsBlack)
  {
    State = STATE_TURN_RIGHT;
  }*/
}

void CompleteLap()
{
  P1SEL = 0;
  TA0CCR1 = 0;
  P1OUT = LEFT_MOTOR_ADDRESS + RIGHT_MOTOR_ADDRESS;
  
  if (SuperState == SUPERSTATE_DRIVE_FROM_MEMORY)
  {
    return;
  }
  
  if (Laps == MAX_LAPS)
  {
    State = STATE_STOPPED;
    ChangeSuperState(SUPERSTATE_DRIVE_FROM_MEMORY);
    return;
  }
  
  if (MiddleSensorDetectsBlack)
  {
    if (!LeftSensorDetectsBlack)
    {
      if (!RightSensorDetectsBlack)
      {
        ChangeState(STATE_DRIVE_FORWARD);
      }
      else
      {
        ChangeState(STATE_COMPLETE_LAP_TURN_RIGHT);
      }
    }
    else if (!RightSensorDetectsBlack)
    {
      ChangeState(STATE_COMPLETE_LAP_TURN_LEFT);
    }
  }
  else
  {
    if (LeftSensorDetectsBlack)
    {
      ChangeState(STATE_TURN_LEFT);
    }
    else if (RightSensorDetectsBlack)
    {
      ChangeState(STATE_TURN_RIGHT);
    }
  }
}

void CompleteLap_TurnLeft()
{
  P1SEL = 0;
  TA0CCR1 = 0;
  P1OUT = RIGHT_MOTOR_ADDRESS;
  
  if (SuperState == SUPERSTATE_DRIVE_FROM_MEMORY)
  {
    return;
  }
  
  if (Laps == MAX_LAPS)
  {
    State = STATE_STOPPED;
    ChangeSuperState(SUPERSTATE_DRIVE_FROM_MEMORY);
    return;
  }
  
  if (MiddleSensorDetectsBlack)
  {
    if (LeftSensorDetectsBlack && RightSensorDetectsBlack)
    {
      ChangeState(STATE_COMPLETE_LAP);
    }
    else if (!LeftSensorDetectsBlack && !RightSensorDetectsBlack)
    {
      ChangeState(STATE_DRIVE_FORWARD);
    }
  }
  else
  {
    if (LeftSensorDetectsBlack)
    {
      ChangeState(STATE_TURN_LEFT);
    }
    else if (RightSensorDetectsBlack)
    {
      ChangeState(STATE_TURN_RIGHT);
    }
  }
}

void CompleteLap_TurnRight()
{  
  P1SEL = LEFT_MOTOR_ADDRESS;
  TA0CCR1 = 0xE0;
  P1OUT = 0;
  
  if (SuperState == SUPERSTATE_DRIVE_FROM_MEMORY)
  {
    return;
  }
  
  if (Laps == MAX_LAPS)
  {
    State = STATE_STOPPED;
    ChangeSuperState(SUPERSTATE_DRIVE_FROM_MEMORY);
    return;
  }
  
  if (MiddleSensorDetectsBlack)
  {
    if (LeftSensorDetectsBlack && RightSensorDetectsBlack)
    {
      ChangeState(STATE_COMPLETE_LAP);
    }
    else if (!LeftSensorDetectsBlack && !RightSensorDetectsBlack)
    {
      ChangeState(STATE_DRIVE_FORWARD);
    }
  }
  else
  {
    if (LeftSensorDetectsBlack)
    {
      ChangeState(STATE_TURN_LEFT);
    }
    else if (RightSensorDetectsBlack)
    {
      ChangeState(STATE_TURN_RIGHT);
    }
  }
}

void Stop()
{
  P1SEL = 0;
  TA0CCR1 = 0;
  P1OUT = 0;
}

void ChangeSuperState(int superState)
{
  SuperState = superState;
  Time = 0;
}

void ChangeState(int state)
{
  State = state;
  
  if (FlashWriteAddress < (char*)FLASH_END_ADDRESS)
  {
    WriteLongToFlash(FlashWriteAddress, Time);
    FlashWriteAddress += sizeof(Time);
    WriteCharToFlash(FlashWriteAddress, (char)state);
    FlashWriteAddress++;
  }
}

int ReadStateFromMemory()
{
  if (FlashReadAddress > FlashWriteAddress)
  {
    return STATE_STOPPED;
  }
  
  int state = State;
  long time = ReadLongFromFlash(FlashReadAddress);
  
  if (Time > time)
  {
    FlashReadAddress += sizeof(Time);
    state = *FlashReadAddress;
    FlashReadAddress++;
  }
  
  return state;
}

void EraseFlash(char* addr, int bytes)
{
  __disable_interrupt();               // Disable interrupts. This is important, otherwise,
                                       // a flash operation in progress while interrupt may
                                       // crash the system.
  while(BUSY & FCTL3);                 // Check if Flash being used
  FCTL2 = FWKEY + FSSEL_1 + FN3;       // Clk = SMCLK/4
  FCTL1 = FWKEY + ERASE;               // Set Erase bit
  FCTL3 = FWKEY;                       // Clear Lock bit
  
  *addr = 0;
  
  while(BUSY & FCTL3);                 // Check if Flash being used
  FCTL1 = FWKEY;                       // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                // Set LOCK bit
  __enable_interrupt();
}

void WriteCharToFlash(char* addr, char value)
{
  BeginWriteToFlash();

  *addr = value;

  EndWriteToFlash();
}

void WriteIntToFlash(char* addr, int value)
{
  BeginWriteToFlash();

  *addr = value >> 8;
  addr++;
  *addr = value;

  EndWriteToFlash();
}

void WriteLongToFlash(char* addr, long value)
{
  BeginWriteToFlash();

  *addr = value >> 24;
  addr++;
  *addr = value >> 16;
  addr++;
  *addr = value >> 8;
  addr++;
  *addr = value;

  EndWriteToFlash();
}

long ReadLongFromFlash(char* address)
{
  long long1 = (long)*address << 24;
  *address++;
  long long2 = (long)*address << 16;
  *address++;
  long long3 = (long)*address << 8;
  *address++;
  long long4 = (long)*address;
  
  return long1 + long2 + long3 + long4;
}

void BeginWriteToFlash()
{
  __disable_interrupt();
  FCTL2 = FWKEY + FSSEL_1 + FN0;       // Clk = SMCLK/4
  FCTL3 = FWKEY;                       // Clear Lock bit
  FCTL1 = FWKEY + WRT;                 // Set WRT bit for write operation
}

void EndWriteToFlash()
{
  FCTL1 = FWKEY;                        // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                 // Set LOCK bit
  while(BUSY & FCTL3);
  __enable_interrupt();
}

void SetLeftMotorSpeed(int speed)
{
  TA0CCR1 = speed;
}

void SetRightMotorSpeed(int speed)
{  
  TA1CCR1 = speed;
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
  if (P1IFG & MIDDLE_SENSOR_ADDRESS)
  {
    MiddleSensorDetectsBlack = P1IES & MIDDLE_SENSOR_ADDRESS;
    P1IES ^= MIDDLE_SENSOR_ADDRESS;
  }
  else if (P1IFG & LEFT_SENSOR_ADDRESS)
  {
    LeftSensorDetectsBlack = P1IES & LEFT_SENSOR_ADDRESS;
    P1IES ^= LEFT_SENSOR_ADDRESS;
  }
  else if (P1IFG & RIGHT_SENSOR_ADDRESS)
  {
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
  if(temp > 0)//755)
  {
    State = STATE_DRIVE_FORWARD;
  }
  __bic_SR_register_on_exit(CPUOFF); // Clear CPUOFF bit from 0(SR)
}

// Timer A0 interrupt service routine
#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer_A0 (void)
{
  Time++;
}