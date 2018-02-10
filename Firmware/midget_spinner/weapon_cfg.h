#ifndef WEAPON_CFG
#define WEAPON_CFG

#include "midget_spinner_cfg.h"

#define K_WEAPON_CONTROL_RX_PIN               2
#define K_WEAPON_CONTROL_TX_PIN               3
#define K_WEAPON_ACTIVATION_PIN               9

#define K_SPEED_CUT_LIMIT                     100

#define K_PREARM_LIMIT                        1500
#define K_PREARM_VALIDATION_DELAY             200        // 200mS

#define K_ARM_ACTIVATE_LIMIT                  1800
#define K_ARM_DEACTIVATE_LIMIT                1100
#define K_ARM_VALIDATION_DELAY                3000      // 3Sec

#define K_WEAPON_ACTIVATION_DELAY             3000

#define K_WEAPON_SPEED_MAX_VALUE              1500
#define K_WEAPON_SPEED_MIN_VALUE              0

#define K_WEAPON_SPEED_INCREMENT              50
#define K_WEAPON_SPEED_DECREMENT              100

#define K_PREARM_VALIDATION_DELAY_VALUE       (K_PREARM_VALIDATION_DELAY/K_LOOP_CYCLE)
#define K_ARM_VALIDATION_DELAY_VALUE          (K_ARM_VALIDATION_DELAY/K_LOOP_CYCLE)
#define K_WEAPON_ACTIVATION_DELAY_VALUE       (K_WEAPON_ACTIVATION_DELAY/K_LOOP_CYCLE)

#endif
