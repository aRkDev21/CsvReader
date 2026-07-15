// Функция опроса тачскрина с автокалибровкой "на лету" при первом запуске
#include "touchscreen.h"
#include "stm32412g_discovery_ts.h"
#include "stm32f4xx_hal.h"
#include <stdint.h>

void calibration_TS(TS_StateTypeDef* TS_State) {

}

TS_GestureIdTypeDef getGestureID(TS_StateTypeDef* TS_State) {
    static uint8_t is_tracking = 0;
    static uint8_t x_start = 0, y_start = 0;
    static uint32_t gesture_start_time = 0;

    if (TS_State->touchDetected) { // exti this
        if (!is_tracking) {
            is_tracking = 1;
            x_start = TS_State->touchX[0];
            y_start = TS_State->touchY[0];
            gesture_start_time = HAL_GetTick();
        }
    }

    else {
        if (is_tracking) {
            is_tracking = 0;
            uint32_t duration = HAL_GetTick() - gesture_start_time;

            if (duration <= SWIPE_MAX_TIME) {
                int16_t delta_x = TS_State->touchX[0] - x_start;
                int16_t delta_y = TS_State->touchY[0] - y_start;
                int16_t abs_dx = (delta_x > 0) ? delta_x : -delta_x;
                int16_t abs_dy = (delta_y > 0) ? delta_y : -delta_y;

                if (abs_dx > abs_dy) {
                    if (abs_dx >= SWIPE_THRESHOLD) {
                        return (delta_x > 0) ? GEST_ID_MOVE_RIGHT : GEST_ID_MOVE_LEFT;
                    }
                }
                else {
                    if (abs_dy >= SWIPE_THRESHOLD) {
                        return (delta_y > 0) ? GEST_ID_MOVE_DOWN : GEST_ID_MOVE_UP;
                    }
                }
            }
        }
    }

    return GEST_ID_NO_GESTURE;
}