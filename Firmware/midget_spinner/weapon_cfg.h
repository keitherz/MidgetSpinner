#ifndef WEAPON_CFG
#define WEAPON_CFG

#include "midget_spinner_cfg.h"

#define K_SPEED_CUT_LIMIT                     10

#define K_PREARM_LIMIT                        1500
#define K_PREARM_VALIDATION_DELAY             200        // 200mS

#define K_ARM_ACTIVATE_LIMIT                  1900
#define K_ARM_DEACTIVATE_LIMIT                1100
#define K_ARM_VALIDATION_DELAY                3000      // 3Sec

#define K_PREARM_VALIDATION_DELAY_VALUE       (K_PREARM_VALIDATION_DELAY/K_LOOP_CYCLE)
#define K_ARM_VALIDATION_DELAY_VALUE          (K_ARM_VALIDATION_DELAY/K_LOOP_CYCLE)

#endif
