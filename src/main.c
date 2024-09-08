//Autor: Oliver Nemček, xnemce08
//Zdroje:
//https://github.com/nopnop2002/esp-idf-ssd1306/tree/master
//https://github.com/espressif/esp-idf/tree/279c8aeb8a312a178c73cf4b50a30798332ea79b/examples
//https://innovationyourself.com/esp32-bluetooth-low-energy-tutorial/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "sdkconfig.h"
#include <stdlib.h>
#include <string.h>
#include "ssd1306.h"
#include "font8x8_basic.h" 

#define tag "SSD1306"
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

SSD1306_t dev;
uint8_t ble_addr_type;
void ble_app_advertise(void);

int line = 0;
int transfer = 0;
int image_cursor = 0;
int image_size = 0;
int image_width = 0;
int image_height = 0;
uint8_t image[1024];

// 'VUT', 128x64px
uint8_t vut [1024] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x1f, 0xff, 0x80, 0x00, 0x7e, 0x0f, 0x80, 0x00, 0x03, 0xff, 
    0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x1f, 0xff, 0x80, 0x00, 0x7e, 0x0f, 0x80, 0x00, 0x03, 0xff, 
    0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x1f, 0xff, 0x80, 0x00, 0x7e, 0x0f, 0x80, 0x00, 0x03, 0xff, 
    0x00, 0x00, 0x00, 0xff, 0xff, 0xe0, 0x1f, 0xff, 0x80, 0x00, 0x7e, 0x0f, 0x80, 0x00, 0x03, 0xff, 
    0x00, 0x00, 0x00, 0xff, 0xff, 0xe0, 0x1f, 0xff, 0x87, 0xff, 0xfe, 0x0f, 0xff, 0x83, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0xff, 0xff, 0xe0, 0x1f, 0xff, 0x87, 0xff, 0xfe, 0x0f, 0xff, 0x83, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0xff, 0xff, 0xe0, 0x1f, 0xff, 0x87, 0xff, 0xfe, 0x0f, 0xff, 0x83, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0xff, 0xff, 0xe0, 0x1f, 0xff, 0x87, 0xff, 0xfe, 0x0f, 0xff, 0x83, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x1f, 0xff, 0x87, 0xff, 0xfe, 0x0f, 0xff, 0x83, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x1f, 0xff, 0x87, 0xff, 0xfe, 0x0f, 0xff, 0x83, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0xff, 0x87, 0xff, 0xfe, 0x0f, 0xff, 0x83, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0xff, 0x87, 0xff, 0xfe, 0x0f, 0xff, 0x83, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0xff, 0x80, 0x00, 0xfe, 0x0f, 0xff, 0x83, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0xff, 0x80, 0x00, 0xfe, 0x0f, 0xff, 0x83, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0xff, 0x80, 0x00, 0xfe, 0x0f, 0xff, 0x83, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0xff, 0x80, 0x00, 0xfe, 0x0f, 0xff, 0x83, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0xff, 0x87, 0xff, 0xfe, 0x0f, 0xff, 0x83, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0xff, 0x87, 0xff, 0xfe, 0x0f, 0xff, 0x83, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0xff, 0x87, 0xff, 0xfe, 0x0f, 0xff, 0x83, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0xff, 0x87, 0xff, 0xfe, 0x0f, 0xff, 0x83, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0xff, 0x87, 0xff, 0xfe, 0x0f, 0xff, 0x83, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0xff, 0x87, 0xff, 0xfe, 0x0f, 0xff, 0x83, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0xff, 0x87, 0xff, 0xfe, 0x0f, 0xff, 0x83, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0xff, 0x87, 0xff, 0xfe, 0x0f, 0xff, 0x83, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0xff, 0x87, 0xff, 0xfe, 0x0f, 0xff, 0x83, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0xff, 0x87, 0xff, 0xfe, 0x0f, 0xff, 0x83, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0xff, 0x87, 0xff, 0xfe, 0x0f, 0xff, 0x83, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0xff, 0x87, 0xff, 0xfe, 0x0f, 0xff, 0x83, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

uint8_t batman[] = {
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0x9F, 0xF9, 0xFF,
    0xFE, 0x3E, 0x7C, 0x7F,
    0xF8, 0x3C, 0x3C, 0x1F,
    0xF0, 0x1C, 0x38, 0x0F,
    0xF0, 0x00, 0x00, 0x0F,
    0xE0, 0x00, 0x00, 0x07,
    0xE0, 0x00, 0x00, 0x07,
    0xF0, 0x00, 0x00, 0x0F,
    0xF0, 0xC4, 0x23, 0x0F,
    0xF9, 0xFE, 0x7F, 0x9F,
    0xFC, 0xFE, 0x7F, 0x3F,
    0xFF, 0xFF, 0xFF, 0xFF
};

// Write data to ESP32 defined as server
static int device_write(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    char * data = (char *)ctxt->om->om_data;
    data[ctxt->om->om_len] = '\0';
    if(transfer == 1){                                                              //image transfer in progress
        for(int i = 0; i<20; i++){
            if(image_cursor == image_size){
                ssd1306_clear_screen(&dev, false);
                ssd1306_bitmaps(&dev, 0, 0, image, image_width, image_height, false);
                line = 0;
                transfer = 0;
                image_cursor = 0;
                break;
            }
            image[image_cursor] = ctxt->om->om_data[i];
            printf("Prijate data pre index obrazka %d : 0x%x\n", image_cursor, image[image_cursor]);
            image_cursor++;       
        }
    }
    else if(ctxt->om->om_data[0] == 1){                                             //beginning of image transfer
        image_width = ctxt->om->om_data[1];
        image_height = ctxt->om->om_data[2];
        if(image_height > SCREEN_HEIGHT || image_width > SCREEN_WIDTH){
            ssd1306_clear_screen(&dev, false);
            ssd1306_display_text(&dev, 1, "Nevalidna", 9, false);
            ssd1306_display_text(&dev, 2, "velkost obrazka", 15, false);
            ssd1306_display_text(&dev, 4, "Prenos nebol", 12, false);
            ssd1306_display_text(&dev, 5, "zahajeny", 8, false);
            printf("Nevalidna velkost obrazka. Prenos nebol zahajeny. \n");
            line = 0;
        }else{
            transfer = 1;
            image_size = (image_height * image_width) / 8;
            ssd1306_clear_screen(&dev, false);
            ssd1306_display_text(&dev, 1, "prenos", 6, false);
            ssd1306_display_text(&dev, 2, "obrazka...", 10, false);
        }
    }
    else if (strcmp(data, "CLEAR")==0){
        ssd1306_clear_screen(&dev, false);
        line = 0;
    }
    else if(strcmp(data, "VUT")==0){
        ssd1306_clear_screen(&dev, false);
		ssd1306_bitmaps(&dev, 0, 0, vut, 128, 64, false);
        line = 0;
    }
    else if(strcmp(data, "BATMAN")==0){
        ssd1306_clear_screen(&dev, false);
		ssd1306_bitmaps(&dev, 0, 0, batman, 32, 13, false);
        line = 0;
    }
    else{
        if (line == 0){
            ssd1306_clear_screen(&dev, false);
        }
        if(line > 7){
            ssd1306_clear_screen(&dev, false);
            line = 0;
        }
        ssd1306_display_text(&dev, line, data, strlen(data), false);
        if(strlen(data) > 16){
            line++;
            ssd1306_display_text(&dev, line, &data[16], strlen(data)-16, false);
        }
        line++;
    }
    return 0;
}

// Array of pointers to other service definitions
// UUID - Universal Unique Identifier
static const struct ble_gatt_svc_def gatt_svcs[] = {
    {.type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = BLE_UUID16_DECLARE(0x180),                 // Define UUID for device type
     .characteristics = (struct ble_gatt_chr_def[]){
         {.uuid = BLE_UUID16_DECLARE(0xDEAD),           // Define UUID for writing
          .flags = BLE_GATT_CHR_F_WRITE,
          .access_cb = device_write},
         {0}}},
    {0}};

// BLE event handling
static int ble_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type)
    {
    // Advertise if connected
    case BLE_GAP_EVENT_CONNECT:
        ESP_LOGI("GAP", "BLE GAP EVENT CONNECT %s", event->connect.status == 0 ? "OK!" : "FAILED!");
        if (event->connect.status != 0)
        {
            ble_app_advertise();
        }
        break;
    // Advertise again after completion of the event
    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI("GAP", "BLE GAP EVENT DISCONNECTED");
        ble_app_advertise();
        break;
    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI("GAP", "BLE GAP EVENT");
        ble_app_advertise();
        break;
    default:
        break;
    }
    return 0;
}

// Define the BLE connection
void ble_app_advertise(void)
{
    // GAP - device name definition
    struct ble_hs_adv_fields fields;
    const char *device_name;
    memset(&fields, 0, sizeof(fields));
    device_name = ble_svc_gap_device_name(); // Read the BLE device name
    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;
    ble_gap_adv_set_fields(&fields);

    // GAP - device connectivity definition
    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND; // connectable or non-connectable
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN; // discoverable or non-discoverable
    ble_gap_adv_start(ble_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_gap_event, NULL);
}

// The application
void ble_app_on_sync(void)
{
    ble_hs_id_infer_auto(0, &ble_addr_type); // Determines the best address type automatically
    ble_app_advertise();                     // Define the BLE connection
}

// The infinite task
void host_task(void *param)
{
    nimble_port_run(); // This function will return only when nimble_port_stop() is executed
}


void app_main()
{
	spi_master_init(&dev, CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO, CONFIG_CS_GPIO, CONFIG_DC_GPIO, CONFIG_RESET_GPIO);
	ssd1306_init(&dev, SCREEN_WIDTH, SCREEN_HEIGHT);
	ssd1306_clear_screen(&dev, false);
	ssd1306_contrast(&dev, 0xff);
    ssd1306_display_text(&dev, 2, "   BLE server", 13, false);
    ssd1306_display_text(&dev, 3, "   spusteny", 12, false);
    ssd1306_display_text(&dev, 7, "Cakam na spravy", 18, false);

    nvs_flash_init();                                   // Initialize NVS flash
    nimble_port_init();                                 // Initialize the host stack
    ble_svc_gap_device_name_set("xnemce08 - IMP");      // Initialize NimBLE configuration - server name
    ble_svc_gap_init();                                 // Initialize NimBLE configuration - gap service
    ble_svc_gatt_init();                                // Initialize NimBLE configuration - gatt service
    ble_gatts_count_cfg(gatt_svcs);                     // Initialize NimBLE configuration - config gatt services
    ble_gatts_add_svcs(gatt_svcs);                      // Initialize NimBLE configuration - queues gatt services.
    ble_hs_cfg.sync_cb = ble_app_on_sync;               // Initialize application
    nimble_port_freertos_init(host_task);               // Run the thread
}