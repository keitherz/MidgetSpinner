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

static unsigned char      ui8_weapon_speed_buff;
static unsigned char      ui8_weapon_arm_buff;
static unsigned char      ui8_weapon_prearm_buff;

static void updatePreArmSwitchStatus(void);
static void updateArmSwitchStatus(void);

void initWeapon(void)
{
  /* disable weapon motors */
  e_weapon_state = DISARMED;
  disableWeapon();
}

void weaponCycle(unsigned char ui8_weapon_speed, unsigned char ui8_weapon_arm, unsigned char ui8_weapon_prearm)
{
  ui8_weapon_speed_buff   = ui8_weapon_speed;
  ui8_weapon_arm_buff     = ui8_weapon_arm;
  ui8_weapon_prearm_buff  = ui8_weapon_prearm;

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
        disableWeapon();
      }
      break;
    }
    case PREARMED:
    {
      if(false == b_prearm_switch_active)
      {
        /* disarmed state */
        disableWeapon();
        e_weapon_state = DISARMED;
      }
      else if(true == b_arm_switch_active)
      {
        /* armed state */
        disableWeapon();
        e_weapon_state = ARMED;        
      }
      else
      {
        /* prearmed state */
        disableWeapon();
        updateArmSwitchStatus();
      }
      break;
    }
    case ARMED:
    {
      if(false == b_prearm_switch_active)
      {
        /* disarmed state */
        disableWeapon();
        e_weapon_state = DISARMED;
      }
      else if(false == b_arm_switch_active)
      {
        /* prearmed state */
        disableWeapon();
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

void disableWeapon(void)
{
  
}

static void updatePreArmSwitchStatus(void)
{
  if(ui8_weapon_prearm_buff > UI16_PREARM_LIMIT)
  {
    b_prearm_switch_active = true;
  }
  else
  {
    b_prearm_switch_active = false;
  }
}

static void updateArmSwitchStatus(void)
{
  if(ui8_weapon_arm_buff > UI16_ARM_ACTIVATE_LIMIT)
  {
    b_arm_switch_active = true;
  }
  else if(ui8_weapon_arm_buff < UI16_ARM_DEACTIVATE_LIMIT)
  {
    b_arm_switch_active = false;
  }
  else
  {
    /* do nothing */
  }
}

