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
void weaponCycle(unsigned int ui16_weapon_speed, unsigned int ui16_weapon_arm, unsigned int ui16_weapon_prearm);  // ch3, ch4, ch5

#endif
