#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include "esp_zigbee_core.h"

#define TAG "BatteryMonitor"

#define ADC_CHANNEL         ADC_CHANNEL_0     // GPIO0
#define ADC_ATTEN           ADC_ATTEN_DB_11   // ~0-3.6V
#define DIVIDER_RATIO       5.0
#define SLEEP_TIME_SEC      300
#define REPORT_CLUSTER_ID ESP_ZB_ZCL_CLUSTER_ID_POWER_CONFIG
#define ATTRIBUTE_BATTERY_VOLTAGE 0x0020 // standard attribute for battery voltage

adc_oneshot_unit_handle_t adc_handle;
adc_cali_handle_t cali_handle;

void init_adc()
{
    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_cfg, &adc_handle));

    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ADC_CHANNEL, &config));
}

void init_adc_calibration()
{
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle));
}

float read_battery_voltage()
{
    int raw = 0;
    const int samples = 64;
    for (int i = 0; i < samples; ++i) {
        int val = 0;
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_CHANNEL, &val));
        raw += val;
    }
    raw /= samples;

    int voltage_mv = 0;
    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(cali_handle, raw, &voltage_mv));

    float battery_voltage = (voltage_mv / 1000.0f) * DIVIDER_RATIO;
    return battery_voltage;
}

static void report_battery_voltage_zigbee(float voltage) {
    uint8_t battery_voltage_ds = (uint8_t)(voltage * 10); // Convert to decivolts (ZCL standard: 1 unit = 100mV)

    esp_zb_zcl_report_attr_cmd_t report = {
        .address_mode = ESP_ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT,
        .clusterID = REPORT_CLUSTER_ID,
        .zcl_basic_cmd.dst_endpoint = 1,
        .zcl_basic_cmd.src_endpoint = 1,
        .zcl_basic_cmd.attrID = ATTRIBUTE_BATTERY_VOLTAGE,  // Battery voltage attribute ID
        .zcl_basic_cmd.attrType = ESP_ZB_ZCL_ATTR_TYPE_U8,   // Attribute type: uint8_t (U8)
        .zcl_basic_cmd.attrValue = &battery_voltage_ds,      // The value of the battery
        .zcl_basic_cmd.attrValueLen = sizeof(uint8_t),       // Length of the value
    };

    ESP_ERROR_CHECK(esp_zb_zcl_report_attr_cmd_send(&report));
    ESP_LOGI(TAG, "Reported battery voltage: %.1fV (ZCL: %d)", voltage, battery_voltage_ds);
}

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing ADC and Calibration...");
    init_adc();
    init_adc_calibration();

    esp_zb_cfg_t zb_nwk_cfg = ESP_ZB_ZC_CONFIG();
    ESP_ERROR_CHECK(esp_zb_init(&zb_nwk_cfg));
    esp_zb_start(false);  // Don't start as coordinator if using Zigbee2MQTT
    ESP_LOGI(TAG, "Zigbee started");

    float battery_voltage = read_battery_voltage();
    ESP_LOGI(TAG, "Battery Voltage: %.2f V", battery_voltage);
    report_battery_voltage_zigbee(battery_voltage);

    // Delay a bit to ensure Zigbee transmission completes
    vTaskDelay(pdMS_TO_TICKS(3000));

    // Clean up
    ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(cali_handle));
    ESP_ERROR_CHECK(adc_oneshot_del_unit(adc_handle));

    
    // Sleep setup
    ESP_LOGI(TAG, "Sleeping for %d seconds...", SLEEP_TIME_SEC);
    esp_sleep_enable_timer_wakeup(SLEEP_TIME_SEC * 1000000ULL);
    esp_deep_sleep_start();
}
