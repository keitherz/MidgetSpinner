#include <arduino.h>

#include "radio_decoder.h"
#include "radio_decoder_cfg.h"

#include "rc_mobot.h"
#include "rc_mobot_cfg.h"

static const unsigned char UI8_LEFT_MOTOR_FORWARD_PIN     = (unsigned char)K_LEFT_MOTOR_FORWARD_PIN;
static const unsigned char UI8_LEFT_MOTOR_REVERSE_PIN     = (unsigned char)K_LEFT_MOTOR_REVERSE_PIN;
static const unsigned char UI8_RIGHT_MOTOR_FORWARD_PIN    = (unsigned char)K_RIGHT_MOTOR_FORWARD_PIN;
static const unsigned char UI8_RIGHT_MOTOR_REVERSE_PIN    = (unsigned char)K_RIGHT_MOTOR_REVERSE_PIN;

static const unsigned char UI8_MOTORS_ENABLE_PIN          = (unsigned char)K_MOTORS_ENABLE_PIN;

static  int       si16_forward_speed_buff;
static  int       si16_turn_speed_buff;
static  int       si16_left_motor_speed;
static  int       si16_right_motor_speed;

static void stopMotors(void);
static void setLeftMotorSpeed(int si16_speed);
static void setRightMotorSpeed(int si16_speed);

void initMobot(void)
{
  /* default enable pin state */
  digitalWrite(UI8_MOTORS_ENABLE_PIN, HIGH);  // active low
  
  /* pin mode initialization */
  pinMode(UI8_LEFT_MOTOR_FORWARD_PIN, OUTPUT);
  pinMode(UI8_LEFT_MOTOR_REVERSE_PIN, OUTPUT);
  pinMode(UI8_RIGHT_MOTOR_FORWARD_PIN, OUTPUT);
  pinMode(UI8_RIGHT_MOTOR_REVERSE_PIN, OUTPUT);
  pinMode(UI8_MOTORS_ENABLE_PIN, OUTPUT);
  
  /* disable mobot motors */
  stopMotors();
}

void mobotCycle(unsigned int ui16_mobot_forward_speed, unsigned int ui16_mobot_turn_speed)
{
  si16_forward_speed_buff   = map(ui16_mobot_forward_speed, K_MIN_PULSE_WIDTH, K_MAX_PULSE_WIDTH, K_MIN_MOTOR_SPEED, K_MAX_MOTOR_SPEED);
  si16_turn_speed_buff      = map(ui16_mobot_turn_speed, K_MIN_PULSE_WIDTH, K_MAX_PULSE_WIDTH, K_MIN_MOTOR_SPEED, K_MAX_MOTOR_SPEED);

  /* compute left motor speed */
  si16_left_motor_speed = (si16_forward_speed_buff >> 1) + (si16_forward_speed_buff >> 2) - (si16_turn_speed_buff >> 2);        // 3/4 forward speed, 1/4 turn speed
  setLeftMotorSpeed(si16_left_motor_speed);
  
  /* compute right motor speed */
  si16_right_motor_speed = (si16_forward_speed_buff >> 1) + (si16_forward_speed_buff >> 2) + (si16_turn_speed_buff >> 2);       // 3/4 forward speed, 1/4 turn speed
  setRightMotorSpeed(si16_right_motor_speed);
}

static void stopMotors(void)
{
  digitalWrite(UI8_MOTORS_ENABLE_PIN, HIGH);
  analogWrite(UI8_LEFT_MOTOR_FORWARD_PIN, 0);
  analogWrite(UI8_LEFT_MOTOR_REVERSE_PIN, 0);
  analogWrite(UI8_RIGHT_MOTOR_FORWARD_PIN, 0);
  analogWrite(UI8_RIGHT_MOTOR_REVERSE_PIN, 0);
}

static void setLeftMotorSpeed(int si16_speed)
{
  if(si16_speed >= 0)
  {
    analogWrite(UI8_LEFT_MOTOR_FORWARD_PIN, si16_speed);
    analogWrite(UI8_LEFT_MOTOR_REVERSE_PIN, 0);
  }
  else
  {
    analogWrite(UI8_LEFT_MOTOR_FORWARD_PIN, 0);
    analogWrite(UI8_LEFT_MOTOR_REVERSE_PIN, -si16_speed);
  }
  
  digitalWrite(UI8_MOTORS_ENABLE_PIN, LOW);
}

static void setRightMotorSpeed(int si16_speed)
{
  if(si16_speed >= 0)
  {
    analogWrite(UI8_RIGHT_MOTOR_FORWARD_PIN, si16_speed);
    analogWrite(UI8_RIGHT_MOTOR_REVERSE_PIN, 0);
  }
  else
  {
    analogWrite(UI8_RIGHT_MOTOR_FORWARD_PIN, 0);
    analogWrite(UI8_RIGHT_MOTOR_REVERSE_PIN, -si16_speed);
  }
  
  digitalWrite(UI8_MOTORS_ENABLE_PIN, LOW);
}
