/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <vector>
#include <esp_err.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <esp_matter.h>
#include <esp_matter_console.h>
#include <esp_matter_ota.h>

#include <app_priv.h>
#include <app_reset.h>
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/ESP32/OpenthreadLauncher.h>
#endif

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>

static const char *TAG = "app_main";
uint16_t light_endpoint_id = 0;

std::vector<gpio_num_t> channel = {GPIO_NUM_13, GPIO_NUM_14, GPIO_NUM_27, GPIO_NUM_26, GPIO_NUM_25};
static uint16_t configured_lights = 0;
static light_endpoint light_list[5];

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;

constexpr auto k_timeout_seconds = 300;

#if CONFIG_ENABLE_ENCRYPTED_OTA
extern const char decryption_key_start[] asm("_binary_esp_image_encryption_key_pem_start");
extern const char decryption_key_end[] asm("_binary_esp_image_encryption_key_pem_end");

static const char *s_decryption_key = decryption_key_start;
static const uint16_t s_decryption_key_len = decryption_key_end - decryption_key_start;
#endif // CONFIG_ENABLE_ENCRYPTED_OTA

static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg)
{
    switch (event->Type) {
    case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
        ESP_LOGI(TAG, "Interface IP Address changed");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
        ESP_LOGI(TAG, "Commissioning complete");
        break;

    case chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired:
        ESP_LOGI(TAG, "Commissioning failed, fail safe timer expired");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStarted:
        ESP_LOGI(TAG, "Commissioning session started");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStopped:
        ESP_LOGI(TAG, "Commissioning session stopped");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowOpened:
        ESP_LOGI(TAG, "Commissioning window opened");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowClosed:
        ESP_LOGI(TAG, "Commissioning window closed");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricRemoved:
        {
            ESP_LOGI(TAG, "Fabric removed successfully");
            if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0)
            {
                chip::CommissioningWindowManager & commissionMgr = chip::Server::GetInstance().GetCommissioningWindowManager();
                constexpr auto kTimeoutSeconds = chip::System::Clock::Seconds16(k_timeout_seconds);
                if (!commissionMgr.IsCommissioningWindowOpen())
                {
                    /* After removing last fabric, this example does not remove the Wi-Fi credentials
                     * and still has IP connectivity so, only advertising on DNS-SD.
                     */
                    CHIP_ERROR err = commissionMgr.OpenBasicCommissioningWindow(kTimeoutSeconds,
                                                    chip::CommissioningWindowAdvertisement::kDnssdOnly);
                    if (err != CHIP_NO_ERROR)
                    {
                        ESP_LOGE(TAG, "Failed to open commissioning window, err:%" CHIP_ERROR_FORMAT, err.Format());
                    }
                }
            }
        break;
        }

    case chip::DeviceLayer::DeviceEventType::kFabricWillBeRemoved:
        ESP_LOGI(TAG, "Fabric will be removed");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricUpdated:
        ESP_LOGI(TAG, "Fabric is updated");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricCommitted:
        ESP_LOGI(TAG, "Fabric is committed");
        break;
    default:
        break;
    }
}

static esp_err_t app_identification_cb(identification::callback_type_t type, uint16_t endpoint_id, uint8_t effect_id,
                                       uint8_t effect_variant, void *priv_data)
{
    ESP_LOGI(TAG, "Identification callback: type: %u, effect: %u, variant: %u", type, effect_id, effect_variant);
    return ESP_OK;
}

static esp_err_t app_attribute_update_cb(attribute::callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id,
                                         uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data)
{
    esp_err_t err = ESP_OK;

    if (type == PRE_UPDATE) {
        /* Driver update */
        app_driver_handle_t driver_handle = (app_driver_handle_t)priv_data;
        err = app_driver_attribute_update(driver_handle, endpoint_id, cluster_id, attribute_id, val);
    }

    return err;
}




static esp_err_t create_light(struct light_gpio* light, node_t* node)
{
    esp_err_t err = ESP_OK;

    /* Initialize driver */
    app_driver_handle_t light_handle = app_driver_light_init(light);

    /* Create a new endpoint. */
    on_off_light::config_t light_config;
    endpoint_t *endpoint = on_off_light::create(node, &light_config, ENDPOINT_FLAG_NONE, light_handle);

    

    /* These node and endpoint handles can be used to create/add other endpoints and clusters. */
    if (!node || !endpoint)
    {
        ESP_LOGE(TAG, "Matter node creation failed");
        err = ESP_FAIL;
        return err;
    }

    for (int i = 0; i < configured_lights; i++) {
        if (light_list[i].light == light) {
            break;
        }
    }

    /* Check for maximum physical buttons that can be configured. */
    if (configured_lights < channel.size()) {
        light_list[configured_lights].light = light;
        light_list[configured_lights].endpoint = endpoint::get_id(endpoint);
        app_driver_light_set_defaults(endpoint::get_id(endpoint));
        configured_lights++;
    }
    else
    {
        ESP_LOGI(TAG, "Cannot configure more lights");
        err = ESP_FAIL;
        return err;
    }

    static uint16_t light_endpoint_id = 0;
    light_endpoint_id = endpoint::get_id(endpoint);
    
    ESP_LOGI(TAG, "Light created with endpoint_id %d", light_endpoint_id);

    // cluster::fixed_label::config_t fl_config;

    // cluster_t *fl_cluster = cluster::fixed_label::create(endpoint, &fl_config, CLUSTER_FLAG_SERVER);
    // cluster::user_label::config_t ul_config;
    // cluster_t *ul_cluster = cluster::user_label::create(endpoint, &ul_config, CLUSTER_FLAG_SERVER);

    // /* Add additional features to the node */
    // cluster_t *cluster = cluster::get(endpoint, OnOff::Id);
    cluster_t *cluster = cluster::create(endpoint,cluster::on_off::feature::lighting::get_id(),CLUSTER_FLAG_SERVER);
    esp_matter_attr_val_t val = esp_matter_invalid(NULL);
    attribute_t *attribute = attribute::create(cluster,OnOff::Attributes::OnOff::Id,ATTRIBUTE_FLAG_NONE,val);
    return err;
}

int get_endpoint(light_gpio* light) {
    for (int i = 0; i < configured_lights; i++) {
        if (light_list[i].light == light) {
            return light_list[i].endpoint;
        }
    }
    return -1;
}







extern "C" void app_main()
{
    esp_err_t err = ESP_OK;
    /* Initialize the ESP NVS layer */
    nvs_flash_init();


    /* Initialize driver */
    // app_driver_handle_t light_handle = app_driver_light_init();
    app_driver_handle_t button_handle = app_driver_button_init();
    app_reset_button_register(button_handle);

    /* Create a Matter node and add the mandatory Root Node device type on endpoint 0 */
    node::config_t node_config;
    node_t *node = node::create(&node_config, app_attribute_update_cb, app_identification_cb);
    for(int i=0;i < channel.size();i++){
        struct light_gpio light;
        light.GPIO_PIN_VALUE = channel.at(i);
        ESP_LOGW(TAG,"Creating light %i with GPIO %i",i,light.GPIO_PIN_VALUE);
        err |= create_light(&light,node);
    }
    // extended_color_light::config_t light_config;
    // light_config.on_off.on_off = DEFAULT_POWER;
    // light_config.on_off.lighting.start_up_on_off = nullptr;
    // light_config.level_control.current_level = DEFAULT_BRIGHTNESS;
    // light_config.level_control.lighting.start_up_current_level = DEFAULT_BRIGHTNESS;
    // light_config.color_control.color_mode = EMBER_ZCL_COLOR_MODE_COLOR_TEMPERATURE;
    // light_config.color_control.enhanced_color_mode = EMBER_ZCL_ENHANCED_COLOR_MODE_COLOR_TEMPERATURE;
    // light_config.color_control.color_temperature.startup_color_temperature_mireds = nullptr;
    // endpoint_t *endpoint = extended_color_light::create(node, &light_config, ENDPOINT_FLAG_NONE, light_handle);

    // /* These node and endpoint handles can be used to create/add other endpoints and clusters. */
    // if (!node || !endpoint) {
    //     ESP_LOGE(TAG, "Matter node creation failed");
    // }

    // light_endpoint_id = endpoint::get_id(endpoint);
    // ESP_LOGI(TAG, "Light created with endpoint_id %d", light_endpoint_id);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    /* Set OpenThread platform config */
    esp_openthread_platform_config_t config = {
        .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
        .port_config = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG(),
    };
    set_openthread_platform_config(&config);
#endif

    /* Matter start */
    err = esp_matter::start(app_event_cb);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Matter start failed: %d", err);
    }
    
    /* Starting driver with default values */
    // app_driver_light_set_defaults(light_endpoint_id);
    nvs_handle_t handle;
    nvs_open_from_partition(CONFIG_CHIP_FACTORY_NAMESPACE_PARTITION_LABEL, "chip-factory", NVS_READWRITE, &handle);

    int32_t out_value = 0;
    if (nvs_get_i32(handle, "fl-sz/1", &out_value) == ESP_ERR_NVS_NOT_FOUND)
    {
       nvs_set_i32(handle, "fl-sz/1", 2);
       nvs_set_str(handle, "fl-k/1/0", "myEP1LBL1");
       nvs_set_str(handle, "fl-v/1/0", "valEP1LBL1");
       nvs_set_str(handle, "fl-k/1/1", "myEP1LBL2");
       nvs_set_str(handle, "fl-v/1/1", "valEP1LBL2");
    }

    nvs_commit(handle);
    nvs_close(handle);


#if CONFIG_ENABLE_ENCRYPTED_OTA
    err = esp_matter_ota_requestor_encrypted_init(s_decryption_key, s_decryption_key_len);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialized the encrypted OTA, err: %d", err);
    }
#endif // CONFIG_ENABLE_ENCRYPTED_OTA

#if CONFIG_ENABLE_CHIP_SHELL
    esp_matter::console::diagnostics_register_commands();
    esp_matter::console::wifi_register_commands();
    esp_matter::console::init();
#endif
}
