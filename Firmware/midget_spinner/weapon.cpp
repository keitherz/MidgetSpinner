#include <arduino.h>
#include <SoftwareSerial9.h>

#include "radio_decoder.h"
#include "radio_decoder_cfg.h"

#include "weapon.h"
#include "weapon_cfg.h"

SoftwareSerial9 weaponControlSerial(K_WEAPON_CONTROL_RX_PIN, K_WEAPON_CONTROL_TX_PIN);

static const unsigned char UI8_WEAPON_ACTIVATION_PIN        = (unsigned char)K_WEAPON_ACTIVATION_PIN;

static const unsigned int  UI16_PREARM_LIMIT                = (unsigned int)K_PREARM_LIMIT;
static const unsigned int  UI16_PREARM_VALIDATION_DELAY     = (unsigned int)K_PREARM_VALIDATION_DELAY_VALUE;

static const unsigned int  UI16_ARM_ACTIVATE_LIMIT          = (unsigned int)K_ARM_ACTIVATE_LIMIT;
static const unsigned int  UI16_ARM_DEACTIVATE_LIMIT        = (unsigned int)K_ARM_DEACTIVATE_LIMIT;
static const unsigned int  UI16_ARM_VALIDATION_DELAY        = (unsigned int)K_ARM_VALIDATION_DELAY_VALUE;
static const unsigned int  UI16_WEAPON_ACTIVATION_DELAY     = (unsigned int)K_WEAPON_ACTIVATION_DELAY_VALUE;

static const unsigned int  UI16_WEAPON_SPEED_INCREMENT      = (unsigned int)K_WEAPON_SPEED_INCREMENT;
static const unsigned int  UI16_WEAPON_SPEED_DECREMENT      = (unsigned int)K_WEAPON_SPEED_DECREMENT;

static weapon_states_et   e_weapon_state;
static bool b_prearm_switch_active                          = false;
static bool b_arm_switch_active                             = false;

static          int       si16_weapon_speed_buff;
static          int       si16_weapon_ctrl_val;
static          int       si16_weapon_setpoint;
static unsigned int       ui16_weapon_arm_buff;
static unsigned int       ui16_weapon_prearm_buff;
static unsigned long      ui32_prearm_activation_validation;
static unsigned long      ui32_prearm_deactivation_validation;
static unsigned long      ui32_arm_activation_validation;
static unsigned long      ui32_arm_deactivation_validation;
static unsigned long      ui32_weapon_activation_delay;

static void updateControlSpeed(void);
static void stopWeapon(void);
static void setWeaponSpeed(int si16_speed);
static void updatePreArmSwitchStatus(void);
static void updateArmSwitchStatus(void);

void initWeapon(void)
{
  weaponControlSerial.begin(26315);
  pinMode(UI8_WEAPON_ACTIVATION_PIN, OUTPUT);
  digitalWrite(UI8_WEAPON_ACTIVATION_PIN, LOW);
  /* disable weapon motors */
  e_weapon_state            = DISARMED;
  b_prearm_switch_active    = false;
  b_arm_switch_active       = false;
  stopWeapon();
}

void weaponCycle(unsigned int ui16_weapon_speed, unsigned int ui16_weapon_arm, unsigned int ui16_weapon_prearm)
{
  si16_weapon_speed_buff    = map(ui16_weapon_speed, K_MIN_PULSE_WIDTH, K_MAX_PULSE_WIDTH, K_WEAPON_SPEED_MIN_VALUE, K_WEAPON_SPEED_MAX_VALUE);
  ui16_weapon_arm_buff      = ui16_weapon_arm;
  ui16_weapon_prearm_buff   = ui16_weapon_prearm;

  updatePreArmSwitchStatus();
  
  switch(e_weapon_state)
  {
    case DISARMED:
    {
      if(true == b_prearm_switch_active)
      {
        /* prearmed state */
        b_arm_switch_active = false;
        e_weapon_state = PREARMED;
      }
      else
      {
        /* disarmed state */
        b_arm_switch_active = false;
        stopWeapon();
      }
      break;
    }
    case PREARMED:
    {
      if(false == b_prearm_switch_active)
      {
        /* disarmed state */
        stopWeapon();
        e_weapon_state = DISARMED;
      }
      else if(true == b_arm_switch_active)
      {
        /* armed state */
        stopWeapon();
        e_weapon_state = ARMED;        
      }
      else
      {
        /* prearmed state */
        stopWeapon();
        updateArmSwitchStatus();
      }
      break;
    }
    case ARMED:
    {
      if(false == b_prearm_switch_active)
      {
        /* disarmed state */
        stopWeapon();
        e_weapon_state = DISARMED;
      }
      else if(false == b_arm_switch_active)
      {
        /* prearmed state */
        stopWeapon();
        e_weapon_state = PREARMED;        
      }
      else
      {
        /* armed state */
        updateArmSwitchStatus();

        /* do not turn on weapon if below specified limit */
        if(si16_weapon_speed_buff < K_SPEED_CUT_LIMIT)
        {
          stopWeapon();
        }
        else
        {
          setWeaponSpeed(si16_weapon_speed_buff);
        }
      }      
      break;
    }
    default:
    {
      initWeapon();
      break;
    }            
  }
}

static void updateControlSpeed(void)
{
  if(si16_weapon_setpoint > si16_weapon_ctrl_val)
  {
    si16_weapon_ctrl_val += UI16_WEAPON_SPEED_INCREMENT;
  }
  else if(si16_weapon_setpoint < si16_weapon_ctrl_val)
  {
    si16_weapon_ctrl_val -= UI16_WEAPON_SPEED_DECREMENT;
  }
  else
  {
    /* do nothing */
  }

  /* speed value saturation */
  if(si16_weapon_ctrl_val > si16_weapon_setpoint)
  {
    si16_weapon_ctrl_val = si16_weapon_ctrl_val;
  }
  else if(si16_weapon_ctrl_val < si16_weapon_setpoint)
  {
    si16_weapon_ctrl_val = si16_weapon_ctrl_val;
  }
  else
  {
    /* do nothing */
  }
  
  weaponControlSerial.write9(0x100);
  weaponControlSerial.write9(si16_weapon_ctrl_val & 0x00FF);
  weaponControlSerial.write9((si16_weapon_ctrl_val >> 8) & 0x00FF);
  weaponControlSerial.write9(si16_weapon_ctrl_val & 0x00FF);
  weaponControlSerial.write9((si16_weapon_ctrl_val >> 8) & 0x00FF);
  weaponControlSerial.write9(0x055);
}

static void stopWeapon(void)
{
  si16_weapon_setpoint = 0;
  updateControlSpeed();
}

static void setWeaponSpeed(int si16_speed)
{
  si16_weapon_setpoint = si16_speed;
  updateControlSpeed();
}

static void updatePreArmSwitchStatus(void)
{
  if(ui16_weapon_prearm_buff > UI16_PREARM_LIMIT)
  {
    ui32_prearm_deactivation_validation = UI16_PREARM_VALIDATION_DELAY;
    
    if(0 != ui32_prearm_activation_validation)
    {
      ui32_prearm_activation_validation--;
    }
    else
    {
      b_prearm_switch_active = true;
    }
  }
  else
  {
    ui32_prearm_activation_validation = UI16_PREARM_VALIDATION_DELAY;

    if(0 != ui32_prearm_deactivation_validation)
    {
      ui32_prearm_deactivation_validation--;
    }
    else
    {
      b_prearm_switch_active = false;
    }
  }
}

static void updateArmSwitchStatus(void)
{
  if((ui16_weapon_arm_buff > UI16_ARM_ACTIVATE_LIMIT) && (si16_weapon_speed_buff < K_SPEED_CUT_LIMIT))
  {
    ui32_arm_deactivation_validation  = UI16_ARM_VALIDATION_DELAY;

    if(0 != ui32_arm_activation_validation)
    {
      ui32_arm_activation_validation--;
    }
    else
    {
      b_arm_switch_active = true;
    }
  }
  else if(ui16_weapon_arm_buff < UI16_ARM_DEACTIVATE_LIMIT)
  {
    ui32_arm_activation_validation    = UI16_ARM_VALIDATION_DELAY;
    digitalWrite(UI8_WEAPON_ACTIVATION_PIN, LOW);

    if(0 != ui32_arm_deactivation_validation)
    {
      ui32_arm_deactivation_validation--;
    }
    else
    {
      b_arm_switch_active = false;
    }
  }
  else
  {
    ui32_arm_deactivation_validation  = UI16_ARM_VALIDATION_DELAY;
    ui32_arm_activation_validation    = UI16_ARM_VALIDATION_DELAY;
  }

  if(true == b_arm_switch_active)
  {
    if(ui32_weapon_activation_delay != 0)
    {
      ui32_weapon_activation_delay--;
      digitalWrite(UI8_WEAPON_ACTIVATION_PIN, HIGH);
    }
    else
    {
      digitalWrite(UI8_WEAPON_ACTIVATION_PIN, LOW);
    }
  }
  else
  {
    ui32_weapon_activation_delay      = UI16_WEAPON_ACTIVATION_DELAY;
    digitalWrite(UI8_WEAPON_ACTIVATION_PIN, LOW);
  }
}

