#include "touchscreen.h"
#include "stm32412g_discovery_ts.h"
#include "stm32f4xx_hal.h"
#include <stdint.h>

uint8_t is_tracking = 0;

void calibrate_coords(uint16_t *calib_x, uint16_t *calib_y) {
    const int16_t x1 = 10, y1 = 40, // top left
                x2 = 235, y2 = 30, // top right
                x3 = 6, y3 = 205, // bot left
                x4 = 235, y4 = 180; // bot right

    int16_t x_left = x1 + ( (*calib_y-y1)*(x3-x1) )/(y3-y1);
    int16_t x_right = x2 + ( (*calib_y-y2)*(x4-x2) )/(y4-y2);
    
    if (x_right == x_left) x_right++;
    *calib_x = ((int32_t)(*calib_x-x_left)*239)/(x_right - x_left);

    int16_t y_top = y1 + ((*calib_x-x1)*(y2-y1))/(x2-x1);
    int16_t y_bot = y3 + ((*calib_x-x3)*(y4-y3))/(x4-x3);

    if (y_bot == y_top) y_bot++;
    *calib_y = ((int32_t)(*calib_y-y_top)*239)/(y_bot - y_top);

    if (*calib_x < 0)   *calib_x = 0;
    if (*calib_x > 239) *calib_x = 240;
    if (*calib_y < 0)   *calib_y = 0;
    if (*calib_y > 239) *calib_y = 240;
}

TS_GestureIdTypeDef getGestureID(TS_StateTypeDef* TS_State, uint16_t* click_x, uint16_t* click_y) {
    static uint32_t gesture_start_time = 0;
    static uint16_t x_start = 0, y_start = 0;
    static uint16_t x_last = 0, y_last = 0;

    if (TS_State->touchDetected) {
        x_last = TS_State->touchX[0];
        y_last = TS_State->touchY[0];

        if (!is_tracking) {
            is_tracking = 1;
            x_start = x_last;
            y_start = y_last;
            gesture_start_time = HAL_GetTick();
        }
    }

    else {
        if (is_tracking) {
            is_tracking = 0;
            uint32_t duration = HAL_GetTick() - gesture_start_time;

            if (duration <= SWIPE_MAX_TIME) {
                int16_t delta_x = x_last - x_start;
                int16_t delta_y = y_last - y_start;
                int16_t abs_dx = (delta_x > 0) ? delta_x : -delta_x;
                int16_t abs_dy = (delta_y > 0) ? delta_y : -delta_y;

                if (abs_dx > abs_dy && abs_dx >= SWIPE_THRESHOLD) {
                    return (delta_x > 0) ? GEST_ID_MOVE_RIGHT : GEST_ID_MOVE_LEFT;
                }
                else if (abs_dy > abs_dx && abs_dy >= SWIPE_THRESHOLD) {
                    return (delta_y > 0) ? GEST_ID_MOVE_DOWN : GEST_ID_MOVE_UP;
                }

                else if (abs_dx < CLICK_THRESHOLD && abs_dy < CLICK_THRESHOLD) {
                    *click_x = x_last;
                    *click_y = y_last;
                    return GEST_ID_CLICK;
                }
            }
        }
    }

    return GEST_ID_NO_GESTURE;
}