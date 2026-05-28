#ifndef THREEEYES_TASK_H
#define THREEEYES_TASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_private/esp_clk.h"
#include "driver/mcpwm_cap.h"
#include "driver/gpio.h"
#include <inttypes.h>
#include "threeeyes.h"

portTASK_FUNCTION(Threeeyes, args);
#endif // THREEEYES_TASK_H