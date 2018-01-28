#include "weapon.h"
#include "weapon_cfg.h"

static const unsigned int  UI16_PREARM_LIMIT            = (unsigned int)K_PREARM_LIMIT;
static const unsigned int  UI16_PREARM_VALIDATION_DELAY = (unsigned int)K_PREARM_VALIDATION_DELAY;

static const unsigned int  UI16_ARM_ACTIVATE_LIMIT      = (unsigned int)K_ARM_ACTIVATE_LIMIT;
static const unsigned int  UI16_ARM_DEACTIVATE_LIMIT    = (unsigned int)K_ARM_DEACTIVATE_LIMIT;
static const unsigned int  UI16_ARM_VALIDATION_DELAY    = (unsigned int)K_ARM_VALIDATION_DELAY;

static weapon_states_et   e_weapon_state;
static bool b_prearm_switch_active                      = false;
static bool b_arm_switch_active                         = false;

static unsigned int       ui16_weapon_speed_buff;
static unsigned int       ui16_weapon_arm_buff;
static unsigned int       ui16_weapon_prearm_buff;
static unsigned long      ui32_prearm_activation_validation;
static unsigned long      ui32_prearm_deactivation_validation;
static unsigned long      ui32_arm_activation_validation;
static unsigned long      ui32_arm_deactivation_validation;

static void updatePreArmSwitchStatus(void);
static void updateArmSwitchStatus(void);

void initWeapon(void)
{
  /* disable weapon motors */
  e_weapon_state = DISARMED;
  stopWeapon();
}

void weaponCycle(unsigned int ui16_weapon_speed, unsigned int ui16_weapon_arm, unsigned int ui16_weapon_prearm)
{
  ui16_weapon_speed_buff   = ui16_weapon_speed;
  ui16_weapon_arm_buff     = ui16_weapon_arm;
  ui16_weapon_prearm_buff  = ui16_weapon_prearm;

  updateArmSwitchStatus();
  
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

void stopWeapon(void)
{
  
}

void setWeaponSpeed(char si8_speed)
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
  if(ui16_weapon_arm_buff > UI16_ARM_ACTIVATE_LIMIT)
  {
    ui32_arm_deactivation_validation = K_ARM_VALIDATION_DELAY;

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
    ui32_arm_activation_validation = K_ARM_VALIDATION_DELAY;

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
    /* do nothing */
  }
}

