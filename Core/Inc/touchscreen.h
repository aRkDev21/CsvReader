#include "stm32412g_discovery_ts.h"
#include "stm32f4xx_hal.h"
#include "stm32412g_discovery_lcd.h"

#include <stdint.h>

#define SWIPE_MAX_TIME 500
#define SWIPE_THRESHOLD 25
#define CLICK_THRESHOLD 5

#define GEST_ID_CLICK 0x99

extern uint8_t is_tracking;

void calibrate_coords(uint16_t *calib_x, uint16_t *calib_y);
TS_GestureIdTypeDef getGestureID(TS_StateTypeDef* TS_State, uint16_t* click_x, uint16_t* click_y);