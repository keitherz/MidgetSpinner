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

#define CTRL_TIMEOUT           (1000)
#define WHEEL_DRIVER_INTERVAL  (5)
#define BLINK_INTERVAL         (250)

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
  int16_t left;
  int16_t right;
  int16_t spinner;
} wheelSpeeds_t;

static ctrlSerialState_t ctrlSerialState;
static ctrlStatus_t ctrlStatus;
static uint8_t ctrlSerialBuf[15];
static uint8_t ctrlSerialCounter;
static bool_t ctrlSerialAvailable;

static wheelSpeeds_t wheelSpeeds;

static uint32_t ctrlTimeoutStamp;
static uint32_t wheelDriverStamp;
static uint32_t blinkStamp;

void controllerProcess(void);
void wheelDriverProcess(void);
void blinkProcess(void);

void updateCtrlSerialStatus(void);
uint8_t calcCtrlSerialChecksum(void);

void setup()
{
  DEBUG_SERIAL.begin(115200);
  CTRL_SERIAL.begin(9600);
  leftWheelSerial.begin(26315);
  rightWheelSerial.begin(26315);
  spinnerWheelSerial.begin(26315);
  
  pinMode(LED_PIN, OUTPUT);
  
  memset(&wheelSpeeds, 0, sizeof(wheelSpeeds));
  memset(&ctrlStatus, 0, sizeof(ctrlStatus));
  
  wheelDriverStamp = millis();
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
  if((millis() - wheelDriverStamp) >= WHEEL_DRIVER_INTERVAL)
  {
    wheelDriverStamp = millis();
    leftWheelSerial.write9(0x100);
    leftWheelSerial.write9(wheelSpeeds.left & 0x00FF);
    leftWheelSerial.write9((wheelSpeeds.left >> 8) & 0x00FF);
    leftWheelSerial.write9(wheelSpeeds.left & 0x00FF);
    leftWheelSerial.write9((wheelSpeeds.left >> 8) & 0x00FF);
    leftWheelSerial.write9(0x055);
    
    rightWheelSerial.write9(0x100);
    rightWheelSerial.write9(wheelSpeeds.right & 0x00FF);
    rightWheelSerial.write9((wheelSpeeds.right >> 8) & 0x00FF);
    rightWheelSerial.write9(wheelSpeeds.right & 0x00FF);
    rightWheelSerial.write9((wheelSpeeds.right >> 8) & 0x00FF);
    rightWheelSerial.write9(0x055);
    
    spinnerWheelSerial.write9(0x100);
    spinnerWheelSerial.write9(wheelSpeeds.spinner & 0x00FF);
    spinnerWheelSerial.write9((wheelSpeeds.spinner >> 8) & 0x00FF);
    spinnerWheelSerial.write9(wheelSpeeds.spinner & 0x00FF);
    spinnerWheelSerial.write9((wheelSpeeds.spinner >> 8) & 0x00FF);
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

