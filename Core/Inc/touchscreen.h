#include "stm32412g_discovery_ts.h"
#include "stm32f4xx_hal.h"
#include "stm32412g_discovery_lcd.h"

#include <stdint.h>

#define SWIPE_MAX_TIME 500
#define SWIPE_THRESHOLD 25
void calibration_TS(TS_StateTypeDef* TS_State);
TS_GestureIdTypeDef getGestureID(TS_StateTypeDef* TS_State);