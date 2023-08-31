/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <esp_log.h>
#include <stdlib.h>
#include <string.h>

#include <device.h>
#include <esp_matter.h>
#include <led_driver.h>
#include <vector>
#include <app_priv.h>
#include "driver/gpio.h"
using namespace chip::app::Clusters;
using namespace esp_matter;

static const char *TAG = "app_driver";
extern uint16_t light_endpoint_id;
std::vector<gpio_num_t> channels = {GPIO_NUM_13, GPIO_NUM_14, GPIO_NUM_27, GPIO_NUM_26, GPIO_NUM_25};
std::vector<uint16_t> endpointss = {1,2,3,4,5};
/* Do any conversions/remapping for the actual value here */
static esp_err_t app_driver_light_set_power(led_driver_handle_t handle, esp_matter_attr_val_t *val)
{
    return led_driver_set_power(handle, val->val.b);
}

static esp_err_t app_driver_light_set_brightness(led_driver_handle_t handle, esp_matter_attr_val_t *val)
{
    int value = REMAP_TO_RANGE(val->val.u8, MATTER_BRIGHTNESS, STANDARD_BRIGHTNESS);
    return led_driver_set_brightness(handle, value);
}

static esp_err_t app_driver_light_set_hue(led_driver_handle_t handle, esp_matter_attr_val_t *val)
{
    int value = REMAP_TO_RANGE(val->val.u8, MATTER_HUE, STANDARD_HUE);
    ESP_LOGW(TAG,"Setting HUE %i",value);
    return led_driver_set_hue(handle, value);
}

static esp_err_t app_driver_light_set_saturation(led_driver_handle_t handle, esp_matter_attr_val_t *val)
{
    int value = REMAP_TO_RANGE(val->val.u8, MATTER_SATURATION, STANDARD_SATURATION);
    ESP_LOGW(TAG,"Setting SATURATION %i",value);
    return led_driver_set_saturation(handle, value);
}

static esp_err_t app_driver_light_set_temperature(led_driver_handle_t handle, esp_matter_attr_val_t *val)
{
    uint32_t value = REMAP_TO_RANGE_INVERSE(val->val.u16, STANDARD_TEMPERATURE_FACTOR);
    return led_driver_set_temperature(handle, value);
}

static void app_driver_button_toggle_cb(void *arg, void *data)
{
    ESP_LOGI(TAG, "Toggle button pressed");
    uint16_t endpoint_id = light_endpoint_id;
    uint32_t cluster_id = OnOff::Id;
    uint32_t attribute_id = OnOff::Attributes::OnOff::Id;

    node_t *node = node::get();
    endpoint_t *endpoint = endpoint::get(node, endpoint_id);
    cluster_t *cluster = cluster::get(endpoint, cluster_id);
    attribute_t *attribute = attribute::get(cluster, attribute_id);

    esp_matter_attr_val_t val = esp_matter_invalid(NULL);
    attribute::get_val(attribute, &val);
    val.val.b = !val.val.b;
    attribute::update(endpoint_id, cluster_id, attribute_id, &val);
}

static void app_driver_light_toggle_cb(void *arg,void *data){
    ESP_LOGI(TAG, "Toggle light ");
    uint16_t endpoint_id = light_endpoint_id;
    uint32_t cluster_id = OnOff::Id;
    uint32_t attribute_id = OnOff::Attributes::OnOff::Id;

    node_t *node = node::get();
    endpoint_t *endpoint = endpoint::get(node, endpoint_id);
    cluster_t *cluster = cluster::get(endpoint, cluster_id);
    attribute_t *attribute = attribute::get(cluster, attribute_id);

    esp_matter_attr_val_t val = esp_matter_invalid(NULL);
    attribute::get_val(attribute, &val);
    val.val.b = !val.val.b;
    attribute::update(endpoint_id, cluster_id, attribute_id, &val);
}

esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val)
{
    esp_err_t err = ESP_OK;
    ESP_LOGW(TAG,"Changed on endpoint %i cluster %lu attribute %lu value %i",endpoint_id,cluster_id,attribute_id,val->val.i8);
    for(int i=0;i < endpointss.size();i++){
        if (endpoint_id == endpointss.at(i)) {
            led_driver_handle_t handle = (led_driver_handle_t)driver_handle;

            if (cluster_id == OnOff::Id) {
                if (attribute_id == OnOff::Attributes::OnOff::Id) {
                    ESP_LOGW(TAG,"Changing value %i -> %i",endpoint_id,val->val.i8);
                    gpio_set_level(channels.at(i),val->val.i8);
                    err = app_driver_light_set_power(handle, val);
                }
            }
            // } else if (cluster_id == LevelControl::Id) {
            //     if (attribute_id == LevelControl::Attributes::CurrentLevel::Id) {
            //         err = app_driver_light_set_brightness(handle, val);
            //     }
            // } else if (cluster_id == ColorControl::Id) {
            //     if (attribute_id == ColorControl::Attributes::CurrentHue::Id) {
            //         err = app_driver_light_set_hue(handle, val);
            //     } else if (attribute_id == ColorControl::Attributes::CurrentSaturation::Id) {
            //         err = app_driver_light_set_saturation(handle, val);
            //     } else if (attribute_id == ColorControl::Attributes::ColorTemperatureMireds::Id) {
            //         err = app_driver_light_set_temperature(handle, val);
            //     }
            // }
        }

    }
    return err;
}

esp_err_t app_driver_light_set_defaults(uint16_t endpoint_id)
{
    esp_err_t err = ESP_OK;
    void *priv_data = endpoint::get_priv_data(endpoint_id);
    led_driver_handle_t handle = (led_driver_handle_t)priv_data;
    node_t *node = node::get();
    endpoint_t *endpoint = endpoint::get(node, endpoint_id);
    cluster_t *cluster = NULL;
    attribute_t *attribute = NULL;
    esp_matter_attr_val_t val = esp_matter_invalid(NULL);

    // /* Setting brightness */
    // cluster = cluster::get(endpoint, OnOff::Id);
    // attribute = attribute::get(cluster, OnI::Attributes::CurrentLevel::Id);
    // attribute::get_val(attribute, &val);
    // err |= app_driver_light_set_brightness(handle, &val);

    // /* Setting color */
    // cluster = cluster::get(endpoint, ColorControl::Id);
    // attribute = attribute::get(cluster, ColorControl::Attributes::ColorMode::Id);
    // attribute::get_val(attribute, &val);
    // if (val.val.u8 == EMBER_ZCL_COLOR_MODE_CURRENT_HUE_AND_CURRENT_SATURATION) {
    //     /* Setting hue */
    //     attribute = attribute::get(cluster, ColorControl::Attributes::CurrentHue::Id);
    //     attribute::get_val(attribute, &val);
    //     err |= app_driver_light_set_hue(handle, &val);
    //     /* Setting saturation */
    //     attribute = attribute::get(cluster, ColorControl::Attributes::CurrentSaturation::Id);
    //     attribute::get_val(attribute, &val);
    //     err |= app_driver_light_set_saturation(handle, &val);
    // } else if (val.val.u8 == EMBER_ZCL_COLOR_MODE_COLOR_TEMPERATURE) {
    //     /* Setting temperature */
    //     attribute = attribute::get(cluster, ColorControl::Attributes::ColorTemperatureMireds::Id);
    //     attribute::get_val(attribute, &val);
    //     err |= app_driver_light_set_temperature(handle, &val);
    // } else {
    //     ESP_LOGE(TAG, "Color mode not supported");
    // }

    /* Setting power */
    // attribute_t *attribute = on_off::attribute::create_on_off(cluster, default_on_off);
    cluster = cluster::get(endpoint, OnOff::Id);
    attribute = attribute::get(cluster, OnOff::Attributes::OnOff::Id);

    attribute::get_val(attribute, &val);
    err |= app_driver_light_set_power(handle, &val);

    return err;
}







app_driver_handle_t app_driver_light_init(light_gpio * light)
{
    /* Initialize led */
    esp_rom_gpio_pad_select_gpio(light->GPIO_PIN_VALUE);
    gpio_set_direction(light->GPIO_PIN_VALUE, GPIO_MODE_OUTPUT);
    // config.gpio = light->GPIO_PIN_VALUE;
    led_driver_config_t config = led_driver_get_config();
    led_driver_handle_t handle = led_driver_init(&config);

    return (app_driver_handle_t)handle;
}

app_driver_handle_t app_driver_button_init()
{
    /* Initialize button */
    button_config_t config = button_driver_get_config();
    button_handle_t handle = iot_button_create(&config);
    iot_button_register_cb(handle, BUTTON_PRESS_DOWN, app_driver_button_toggle_cb, NULL);
    return (app_driver_handle_t)handle;
}
