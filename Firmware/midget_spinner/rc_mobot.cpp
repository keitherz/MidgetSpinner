#include <arduino.h>

#include "radio_decoder.h"
#include "radio_decoder_cfg.h"

#include "rc_mobot.h"
#include "rc_mobot_cfg.h"

static  int       si16_forward_speed_buff;
static  int       si16_turn_speed_buff;
static  int       si16_left_motor_speed;
static  int       si16_right_motor_speed;

static void stopMotors(void);
static void setLeftMotorSpeed(int si16_speed);
static void setRightMotorSpeed(int si16_speed);

void initMobot(void)
{
  /* disable mobot motors */
  stopMotors();
}

void mobotCycle(unsigned int ui16_mobot_forward_speed, unsigned int ui16_mobot_turn_speed)
{
  si16_forward_speed_buff   = map(ui16_mobot_forward_speed, K_MIN_PULSE_WIDTH, K_MAX_PULSE_WIDTH, K_MIN_MOTOR_SPEED, K_MAX_MOTOR_SPEED);
  si16_turn_speed_buff      = map(ui16_mobot_turn_speed, K_MIN_PULSE_WIDTH, K_MAX_PULSE_WIDTH, K_MIN_MOTOR_SPEED, K_MAX_MOTOR_SPEED);

  /* compute left motor speed */
  si16_left_motor_speed = si16_forward_speed_buff >> 1 + si16_forward_speed_buff >> 2 - si16_turn_speed_buff >> 2;        // 3/4 forward speed, 1/4 turn speed
  setLeftMotorSpeed(si16_left_motor_speed);
  
  /* compute right motor speed */
  si16_right_motor_speed = si16_forward_speed_buff >> 1 + si16_forward_speed_buff >> 2 + si16_turn_speed_buff >> 2;       // 3/4 forward speed, 1/4 turn speed
  setRightMotorSpeed(si16_right_motor_speed);  
}

static void stopMotors(void)
{
  
}

static void setLeftMotorSpeed(int si16_speed)
{
  
}

static void setRightMotorSpeed(int si16_speed)
{
  
}
