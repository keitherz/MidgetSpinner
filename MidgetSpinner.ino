#include <SoftwareSerial9.h>

typedef bool bool_t;

#define LED_PIN                22

#define LEFT_WHEEL_RX          26
#define LEFT_WHEEL_TX          27
#define RIGHT_WHEEL_RX         28
#define RIGHT_WHEEL_TX         29
#define SPINNER_WHEEL_RX       30
#define SPINNER_WHEEL_TX       31

#define DEBUG_SERIAL           Serial
#define CTRL_SERIAL            Serial1
#define ctrlSerialEvent()      serialEvent1()

#define INVERT_LEFT_WHEEL
//#define INVERT_RIGHT_WHEEL
//#define INVERT_SPINNER_WHEEL

#define CTRL_TIMEOUT           (1000)
#define WHEEL_DRIVER_INTERVAL  (5)
#define BLINK_INTERVAL         (250)

#define RAMP_INC               (2)
#define RAMP_DEC               (-8)
#define RAMP_PERIOD            (25)

SoftwareSerial9 leftWheelSerial(LEFT_WHEEL_RX, LEFT_WHEEL_TX);
SoftwareSerial9 rightWheelSerial(RIGHT_WHEEL_RX, RIGHT_WHEEL_TX);
SoftwareSerial9 spinnerWheelSerial(SPINNER_WHEEL_RX, SPINNER_WHEEL_TX);

typedef enum
{
  CTRL_SER_SYNC1,
  CTRL_SER_SYNC2,
  CTRL_SER_FETCHDATA
} ctrlSerialState_t;

typedef struct
{
  bool_t up;
  bool_t down;
  bool_t left;
  bool_t right;
  bool_t triangle;
  bool_t circle;
  bool_t cross;
  bool_t square;
  bool_t left1;
  bool_t left2;
  bool_t right1;
  bool_t right2;
  bool_t start;
  uint8_t leftx;
  uint8_t lefty;
  uint8_t rightx;
  uint8_t righty;
} ctrlStatus_t;

typedef struct
{
  int16_t currSpeed;
  int16_t setPointSpeed;
  int16_t rampIncrement;
} speedVars_t;

typedef struct
{
  int16_t left;
  int16_t right;
  int16_t spinner;
} wheelSpeed_t;

static ctrlSerialState_t ctrlSerialState;
static ctrlStatus_t ctrlStatus;
static uint8_t ctrlSerialBuf[15];
static uint8_t ctrlSerialCounter;
static bool_t ctrlSerialAvailable;

static speedVars_t leftWheel;
static speedVars_t rightWheel;
static speedVars_t spinnerWheel;

static wheelSpeed_t wheelSpeed;
static wheelSpeed_t wheelSpeedSetpoint;
static wheelSpeed_t wheelSpeedRampInc;

static uint32_t ctrlTimeoutStamp;
static uint32_t wheelDriverRampStamp;
static uint32_t wheelDriverUpdateStamp;
static uint32_t blinkStamp;

static void controllerProcess(void);
static void wheelDriverProcess(void);
static void blinkProcess(void);

static void parseCtrlWheelSpeed(void);
static void processWheelSpeedRamp(void);
static void updateCtrlSerialStatus(void);
static uint8_t calcCtrlSerialChecksum(void);

void setup()
{
  DEBUG_SERIAL.begin(115200);
  CTRL_SERIAL.begin(9600);
  leftWheelSerial.begin(26315);
  rightWheelSerial.begin(26315);
  spinnerWheelSerial.begin(26315);
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  memset(&wheelSpeed, 0, sizeof(wheelSpeed));
  memset(&wheelSpeedSetpoint, 0, sizeof(wheelSpeedSetpoint));
  memset(&wheelSpeedRampInc, 0, sizeof(wheelSpeedRampInc));
  memset(&ctrlStatus, 0, sizeof(ctrlStatus));
  
  wheelDriverUpdateStamp = millis();
  blinkStamp = millis();
}

void loop()
{
  controllerProcess();
  wheelDriverProcess();
  blinkProcess();
}

void ctrlSerialEvent()
{
  while(CTRL_SERIAL.available())
  {
    uint8_t rxChar = (uint8_t)CTRL_SERIAL.read();
    switch(ctrlSerialState)
    {
      case CTRL_SER_SYNC1:
      {
        ctrlSerialCounter = 0;
        if(0x5A == rxChar)
        {
          ctrlSerialState = CTRL_SER_SYNC2;
        }
        break;
      }
      
      case CTRL_SER_SYNC2:
      {
        if(0xA5 == rxChar)
        {
          ctrlSerialState = CTRL_SER_FETCHDATA;
        }
        else
        {
          ctrlSerialState= CTRL_SER_SYNC1;
        }
        break;
      }
      
      case CTRL_SER_FETCHDATA:
      {
        ctrlSerialBuf[ctrlSerialCounter++] = rxChar;
        if(ctrlSerialCounter >= 10)
        {
          if(calcCtrlSerialChecksum() == ctrlSerialBuf[9])
          {
            updateCtrlSerialStatus();
            ctrlSerialAvailable = true;
          }
          else
          {
            ctrlSerialState = CTRL_SER_SYNC1;
            ctrlSerialAvailable = false;
          }
        }
        break;
      }
      
      default:
      {
        ctrlSerialState = CTRL_SER_SYNC1;
        break;
      }
    }
  }
}

void controllerProcess(void)
{
  if(true == ctrlSerialAvailable)
  {
    ctrlSerialAvailable = false;
    ctrlSerialState = CTRL_SER_SYNC1;
    ctrlTimeoutStamp = millis();

    parseCtrlWheelSpeed();
  }
  else
  {
    if((millis() - ctrlTimeoutStamp) >= CTRL_TIMEOUT)
    {
      /* commence shutdown here */
    }
  }
}

void wheelDriverProcess(void)
{
  int16_t speedVal;
  
  if((millis() - wheelDriverRampStamp) >= RAMP_PERIOD)
  {
    wheelDriverRampStamp = millis();

    processWheelSpeedRamp();
#if 1
    DEBUG_SERIAL.print(wheelSpeedSetpoint.left, DEC);
    DEBUG_SERIAL.print(", ");
    DEBUG_SERIAL.print(wheelSpeedSetpoint.right, DEC);
    DEBUG_SERIAL.print(", ");
    DEBUG_SERIAL.print(wheelSpeed.left, DEC);
    DEBUG_SERIAL.print(", ");
    DEBUG_SERIAL.print(wheelSpeed.right, DEC);
    DEBUG_SERIAL.println();
#endif
  }
  
  if((millis() - wheelDriverUpdateStamp) >= WHEEL_DRIVER_INTERVAL)
  {
    wheelDriverUpdateStamp = millis();
    
#ifdef INVERT_LEFT_WHEEL
    speedVal = -wheelSpeed.left;
#else
    speedVal = wheelSpeed.left;
#endif
    leftWheelSerial.write9(0x100);
    leftWheelSerial.write9(speedVal & 0x00FF);
    leftWheelSerial.write9((speedVal >> 8) & 0x00FF);
    leftWheelSerial.write9(speedVal & 0x00FF);
    leftWheelSerial.write9((speedVal >> 8) & 0x00FF);
    leftWheelSerial.write9(0x055);
    
#ifdef INVERT_RIGHT_WHEEL
    speedVal = -wheelSpeed.right;
#else
    speedVal = wheelSpeed.right;
#endif
    rightWheelSerial.write9(0x100);
    rightWheelSerial.write9(speedVal & 0x00FF);
    rightWheelSerial.write9((speedVal >> 8) & 0x00FF);
    rightWheelSerial.write9(speedVal & 0x00FF);
    rightWheelSerial.write9((speedVal >> 8) & 0x00FF);
    rightWheelSerial.write9(0x055);
    
#ifdef INVERT_SPINNER_WHEEL
    speedVal = -wheelSpeed.spinner;
#else
    speedVal = wheelSpeed.spinner;
#endif
    spinnerWheelSerial.write9(0x100);
    spinnerWheelSerial.write9(speedVal & 0x00FF);
    spinnerWheelSerial.write9((speedVal >> 8) & 0x00FF);
    spinnerWheelSerial.write9(speedVal & 0x00FF);
    spinnerWheelSerial.write9((speedVal >> 8) & 0x00FF);
    spinnerWheelSerial.write9(0x055);
  }
}

void blinkProcess(void)
{
  if((millis() - blinkStamp) >= BLINK_INTERVAL)
  {
    blinkStamp = millis();
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }
}

void parseCtrlWheelSpeed(void)
{
  int16_t xVal;
  int16_t yVal;

  if((ctrlStatus.lefty <= 120) || (ctrlStatus.lefty >= 136))
  {
    yVal = map(ctrlStatus.lefty, 255, 0, -2048, 2048);
    //DEBUG_SERIAL.println(yVal, DEC);
  }
  else
  {
    yVal = 0;
  }

  wheelSpeedSetpoint.left = yVal;
  wheelSpeedSetpoint.right = yVal;

  DEBUG_SERIAL.print(ctrlStatus.leftx, DEC);
  DEBUG_SERIAL.print(", ");
  DEBUG_SERIAL.print(ctrlStatus.lefty, DEC);
  DEBUG_SERIAL.println();
}

void processWheelSpeedRamp(void)
{
  int16_t tempBuf;

  if(wheelSpeedSetpoint.left > wheelSpeed.left)
  {
    if(wheelSpeedRampInc.left < 0)
    {
      wheelSpeedRampInc.left = RAMP_INC;
    }
    else
    {
      wheelSpeedRampInc.left += RAMP_INC;
    }

    tempBuf = wheelSpeed.left + wheelSpeedRampInc.left;
    if(tempBuf > wheelSpeedSetpoint.left)
    {
      wheelSpeed.left = wheelSpeedSetpoint.left;
    }
    else
    {
      wheelSpeed.left = tempBuf;
    }
  }
  else if(wheelSpeedSetpoint.left < wheelSpeed.left)
  {
    if(wheelSpeedRampInc.left > 0)
    {
      wheelSpeedRampInc.left = RAMP_DEC;
    }
    else
    {
      wheelSpeedRampInc.left += RAMP_DEC;
    }

    tempBuf = wheelSpeed.left + wheelSpeedRampInc.left;
    if(tempBuf < wheelSpeedSetpoint.left)
    {
      wheelSpeed.left = wheelSpeedSetpoint.left;
    }
    else
    {
      wheelSpeed.left = tempBuf;
    }
  }
  else
  {
    wheelSpeedRampInc.left = 0;
  }

  if(wheelSpeedSetpoint.right > wheelSpeed.right)
  {
    if(wheelSpeedRampInc.right < 0)
    {
      wheelSpeedRampInc.right = RAMP_INC;
    }
    else
    {
      wheelSpeedRampInc.right += RAMP_INC;
    }

    tempBuf = wheelSpeed.right + wheelSpeedRampInc.right;
    if(tempBuf > wheelSpeedSetpoint.right)
    {
      wheelSpeed.right = wheelSpeedSetpoint.right;
    }
    else
    {
      wheelSpeed.right = tempBuf;
    }
  }
  else if(wheelSpeedSetpoint.right < wheelSpeed.right)
  {
    if(wheelSpeedRampInc.right > 0)
    {
      wheelSpeedRampInc.right = RAMP_DEC;
    }
    else
    {
      wheelSpeedRampInc.right += RAMP_DEC;
    }

    tempBuf = wheelSpeed.right + wheelSpeedRampInc.right;
    if(tempBuf < wheelSpeedSetpoint.right)
    {
      wheelSpeed.right = wheelSpeedSetpoint.right;
    }
    else
    {
      wheelSpeed.right = tempBuf;
    }
  }
  else
  {
    wheelSpeedRampInc.right = 0;
  }
}

void updateCtrlSerialStatus(void)
{
  memset(&ctrlStatus, 0, sizeof(ctrlStatus));
  if(0 == (ctrlSerialBuf[3] & 0x08))
  {
    ctrlStatus.up = true;
  }
  if(0 == (ctrlSerialBuf[3] & 0x02))
  {
    ctrlStatus.down = true;
  }
  if(0 == (ctrlSerialBuf[3] & 0x01))
  {
    ctrlStatus.left = true;
  }
  if(0 == (ctrlSerialBuf[3] & 0x04))
  {
    ctrlStatus.right = true;
  }
  
  if(0 == (ctrlSerialBuf[4] & 0x08))
  {
    ctrlStatus.triangle = true;
  }
  if(0 == (ctrlSerialBuf[4] & 0x04))
  {
    ctrlStatus.circle = true;
  }
  if(0 == (ctrlSerialBuf[4] & 0x02))
  {
    ctrlStatus.cross = true;
  }
  if(0 == (ctrlSerialBuf[4] & 0x01))
  {
    ctrlStatus.square = true;
  }
  
  if(0 == (ctrlSerialBuf[4] & 0x20))
  {
    ctrlStatus.left1 = true;
  }
  if(0 == (ctrlSerialBuf[4] & 0x80))
  {
    ctrlStatus.left2 = true;
  }
  if(0 == (ctrlSerialBuf[4] & 0x10))
  {
    ctrlStatus.right1 = true;
  }
  if(0 == (ctrlSerialBuf[4] & 0x40))
  {
    ctrlStatus.right2 = true;
  }
  if(0 == (ctrlSerialBuf[3] & 0x10))
  {
    ctrlStatus.start = true;
  }
  
  ctrlStatus.rightx = ctrlSerialBuf[5];
  ctrlStatus.rightx = ctrlSerialBuf[6];
  ctrlStatus.leftx = ctrlSerialBuf[7];
  ctrlStatus.lefty = ctrlSerialBuf[8];
}

uint8_t calcCtrlSerialChecksum(void)
{
  uint8_t idx;
  uint8_t sum;
  
  sum = (0x5A + 0xA5);
  for(idx = 0; idx < 9; idx++)
  {
    sum += ctrlSerialBuf[idx];
  }
  
  return sum;
}

