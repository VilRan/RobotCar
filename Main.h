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

int State = STATE_STANDBY;
int LastState = STATE_DRIVE_FORWARD;
int Laps = 0;
int MiddleSensorDetectsBlack = 0;
int LeftSensorDetectsBlack = 0;
int RightSensorDetectsBlack = 0;

void StopWatchdog();
void Initialize();
void Standby();
void DriveForward();
void TurnLeft();
void TurnRight();
void CompleteLap();
void Stop();
void SetLeftMotorSpeed();
void SetRightMotorSpeed();
int ReadLeftSensor();
int ReadMiddleSensor();
int ReadRightSensor();