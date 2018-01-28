#include <arduino.h>

#include "radio_decoder.h"
#include "radio_decoder_cfg.h"

#include "weapon.h"
#include "weapon_cfg.h"

static const unsigned int  UI16_PREARM_LIMIT            = (unsigned int)K_PREARM_LIMIT;
static const unsigned int  UI16_PREARM_VALIDATION_DELAY = (unsigned int)K_PREARM_VALIDATION_DELAY_VALUE;

static const unsigned int  UI16_ARM_ACTIVATE_LIMIT      = (unsigned int)K_ARM_ACTIVATE_LIMIT;
static const unsigned int  UI16_ARM_DEACTIVATE_LIMIT    = (unsigned int)K_ARM_DEACTIVATE_LIMIT;
static const unsigned int  UI16_ARM_VALIDATION_DELAY    = (unsigned int)K_ARM_VALIDATION_DELAY_VALUE;

static weapon_states_et   e_weapon_state;
static bool b_prearm_switch_active                      = false;
static bool b_arm_switch_active                         = false;

static          int       si16_weapon_speed_buff;
static unsigned int       ui16_weapon_arm_buff;
static unsigned int       ui16_weapon_prearm_buff;
static unsigned long      ui32_prearm_activation_validation;
static unsigned long      ui32_prearm_deactivation_validation;
static unsigned long      ui32_arm_activation_validation;
static unsigned long      ui32_arm_deactivation_validation;

static void stopWeapon(void);
static void setWeaponSpeed(int si16_speed);
static void updatePreArmSwitchStatus(void);
static void updateArmSwitchStatus(void);

void initWeapon(void)
{
  /* disable weapon motors */
  e_weapon_state            = DISARMED;
  b_prearm_switch_active    = false;
  b_arm_switch_active       = false;
  stopWeapon();
}

void weaponCycle(unsigned int ui16_weapon_speed, unsigned int ui16_weapon_arm, unsigned int ui16_weapon_prearm)
{
  si16_weapon_speed_buff    = map(ui16_weapon_speed, K_MIN_PULSE_WIDTH, K_MAX_PULSE_WIDTH, 0, K_MAX_MOTOR_SPEED);
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

static void stopWeapon(void)
{
  
}

static void setWeaponSpeed(int si16_speed)
{
  
}

static void updatePreArmSwitchStatus(void)
{
  if(ui16_weapon_prearm_buff > UI16_PREARM_LIMIT)
  {
    ui32_prearm_deactivation_validation = K_PREARM_VALIDATION_DELAY;
    
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
    ui32_prearm_activation_validation = K_PREARM_VALIDATION_DELAY;

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
    ui32_arm_deactivation_validation  = K_ARM_VALIDATION_DELAY;

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
    ui32_arm_activation_validation    = K_ARM_VALIDATION_DELAY;

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
    ui32_arm_deactivation_validation  = K_ARM_VALIDATION_DELAY;
    ui32_arm_activation_validation    = K_ARM_VALIDATION_DELAY;
  }
}

