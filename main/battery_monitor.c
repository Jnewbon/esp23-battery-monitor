/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/temperature_sensor.h"
#include "driver/gpio.h"

static const char *TAG = "example";
#define LED 4



unsigned char rotate_right(unsigned char byte, int n) {
    return (byte >> n) | (byte << (8 - n));
}

void print_char_as_binary(char c) {
    // Iterate through all 8 bits of the char (assuming char is 1 byte)
    for (int i = 7; i >= 0; i--) {
        // Print the bit (0 or 1) at the i-th position
        ESP_LOGI("Binary", "%d", (c >> i) & 1);
    }
    ESP_LOGI("Binary", "");  // Print a newline after the binary number
}

void app_main(void)
{

    unsigned char byte = 3;

    ESP_LOGI(TAG, "Install temperature sensor, expected temp ranger range: 10~50 ℃");
    temperature_sensor_handle_t temp_sensor = NULL;
    temperature_sensor_config_t temp_sensor_config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(10, 50);
    ESP_ERROR_CHECK(temperature_sensor_install(&temp_sensor_config, &temp_sensor));

    for( int i = 0; i <=8; ++i) {
        gpio_reset_pin(LED+i);
        gpio_set_direction(LED+i, GPIO_MODE_OUTPUT);
    }

    ESP_LOGI(TAG, "Enable temperature sensor");
    ESP_ERROR_CHECK(temperature_sensor_enable(temp_sensor));

    ESP_LOGI(TAG, "Read temperature");
    int cnt = 3000;
    float tsens_value;

    while (cnt--) {
        print_char_as_binary(byte);
        gpio_set_level(LED+0, (byte >> 0) & 1);
        gpio_set_level(LED+1, (byte >> 1) & 1);
        gpio_set_level(LED+2, (byte >> 2) & 1);
        gpio_set_level(LED+3, (byte >> 3) & 1);
        gpio_set_level(LED+4, (byte >> 4) & 1);
        gpio_set_level(LED+5, (byte >> 5) & 1);
        gpio_set_level(LED+6, (byte >> 6) & 1);
        gpio_set_level(LED+7, (byte >> 7) & 1);
        byte = rotate_right(byte, 1);

        ESP_ERROR_CHECK(temperature_sensor_get_celsius(temp_sensor, &tsens_value));
        ESP_LOGI(TAG, "Temperature value %.02f ℃", tsens_value);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
