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

#define SUPERSTATE_RECORDING 0
#define SUPERSTATE_DRIVE_FROM_MEMORY 1

#define STATE_STANDBY 0
#define STATE_DRIVE_FORWARD 1
#define STATE_TURN_LEFT 2
#define STATE_TURN_RIGHT 3
#define STATE_COMPLETE_LAP 4
#define STATE_COMPLETE_LAP_TURN_LEFT 5
#define STATE_COMPLETE_LAP_TURN_RIGHT 6
#define STATE_STOPPED 7

#define MAX_LAPS 1

#define FLASH_START_ADDRESS 0xD000
#define FLASH_END_ADDRESS 0xE000

int SuperState = SUPERSTATE_RECORDING;
int State = STATE_DRIVE_FORWARD;
int Laps = 0;
int MiddleSensorDetectsBlack = 0;
int LeftSensorDetectsBlack = 0;
int RightSensorDetectsBlack = 0;
char* FlashWriteAddress = (char*)FLASH_START_ADDRESS;
char* FlashReadAddress = (char*)FLASH_START_ADDRESS;
long Time = 0;

void Initialize();
void Run();
void Standby();
void DriveForward();
void TurnLeft();
void TurnRight();
void CompleteLap();
void CompleteLap_TurnLeft();
void CompleteLap_TurnRight();
void Stop();
void ChangeState(int state);
void ChangeSuperState(int superState);
void EraseFlash(char* address, int bytes);
void WriteCharToFlash(char* address, char value);
void WriteIntToFlash(char* address, int value);
void WriteLongToFlash(char* address, long value);
long ReadLongFromFlash(char* address);
void BeginWriteToFlash();
void EndWriteToFlash();
void SetLeftMotorSpeed();
void SetRightMotorSpeed();
int ReadStateFromMemory();