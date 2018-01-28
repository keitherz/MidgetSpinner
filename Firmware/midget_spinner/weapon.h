#ifndef WEAPON
#define WEAPON

typedef enum
{
  DISARMED
  ,PREARMED
  ,ARMED
  ,NUM_OF_WEAPON_STATES 
}weapon_states_et;


void initWeapon(void);
void disableWeapon(void);
void weaponCycle(unsigned char ui8_weapon_speed, unsigned char ui8_weapon_arm, unsigned char ui8_weapon_prearm);  // ch3, ch4, ch5
#endif
