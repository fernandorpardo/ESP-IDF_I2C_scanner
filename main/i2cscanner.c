 
 /**
 ESP-IDF_I2C_scanner
 
 This a re-work of the I2C scanner from nkolban
 
 https://github.com/nkolban/esp32-snippets/tree/master/i2c/scanner
 
 to addapt the code to the ESP-IDF I2C "new" driver
 
 The project takes as baseline the  
 ESP-IDF's hello_world example (c) 2010-2022 Espressif Systems
 
 Code is genearted using 
 ESP-IDF v5.4-dev-2194-gd7ca8b94c8
 for linux running on a Raspberry
 
 The target platform is ESP32-H2-DevKitM-1
 
 
 The new (by Nov'24) driver is much more loquacious than the old one. This is probably because it is still in a debugging phase, 
 and it is likely the issue now reported will change shoortly. 
 By default for a failling write operaction that does not result in an ACK at bus level, which is the expected behavoiur 
 for detecting an unattended address, the driver shows 3 traces
 
 
E (3130) i2c.master: I2C transaction unexpected nack detected
E (3140) i2c.master: s_i2c_synchronous_transaction(904): I2C transaction failed
E (3150) i2c.master: i2c_master_multi_buffer_transmit(1130): I2C transaction failed

The last two are mutted in  menuconfig
	Compiler options --->
	[*] Disable messages in ESP_RETURN_ON_* and ESP_EXIT_ON_* macros
	
for the first one you need to set  
	struct i2c_master_bus_t -> bypass_nack_log= true;

i2c_master_bus_t is defined in i2c_private.h

Once you get rid of the traces the otput is clean and looks like this

     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- -- -- -- --
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
30: -- -- -- -- -- -- -- -- -- -- -- -- 3c -- -- --
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
70: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

 
---------------------------------------------------------------------------------------------------

To generate the code 

. $HOME/esp/esp-idf/export.sh

idf.py set-target esp32h2

idf.py menuconfig
	Compiler options --->
	[*] Disable messages in ESP_RETURN_ON_* and ESP_EXIT_ON_* macros

idf.py build

Execute
idf.py -p /dev/ttyACM0 flash

idf.py -p /dev/ttyACM0 monitor


**/

#include <driver/i2c.h>
#include <esp_log.h>


#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "driver/i2c_master.h"
#include "driver/i2c_types.h"
#include "i2c_private.h"	// IDF_PATH + /components/esp_driver_i2c/i2c_private.h"


#define I2C_MASTER_SDA_IO 3
#define I2C_MASTER_SCL_IO 4

i2c_master_bus_handle_t bus_handle;
i2c_master_dev_handle_t dev_handle;	

i2c_master_bus_config_t i2c_mst_config = {
	.clk_source = I2C_CLK_SRC_DEFAULT,
	.i2c_port = I2C_NUM_0,
	.scl_io_num = I2C_MASTER_SCL_IO,
	.sda_io_num = I2C_MASTER_SDA_IO,
	.glitch_ignore_cnt = 7,
	.flags.enable_internal_pullup = true,
};
	

#define DATA_LENGTH 1

void i2cscanner(void) 
{
	uint8_t data_wr[DATA_LENGTH];
	data_wr[0]= 0xaa;	
	
	printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
	printf("00:         ");
	
	for (uint8_t i=3; i<= 0x7F; i++) 
	{
		i2c_device_config_t dev_cfg = {
			.dev_addr_length = I2C_ADDR_BIT_LEN_7,
			.device_address = i,
			.scl_speed_hz = 100000,
//			.flags.disable_ack_check= true,
		};		
		
		i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle);	 
		
		esp_err_t err= i2c_master_transmit(dev_handle, data_wr, sizeof(data_wr), 100);

		if (i%16 == 0) {
			printf("\n%.2x:", i);
		}
		if (err == ESP_OK) {
			printf(" %.2x", i);
		} else {
			printf(" --");
		}
		i2c_master_bus_rm_device(dev_handle);
	}
	printf("\n");
} // i2cscanner()

void app_main(void)
{
    printf("Hello world!\n");

    // Print chip information 
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), %s%s%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed");
        return;
    }

    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

	// i2c bus init
	ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));
	bus_handle->bypass_nack_log= true;
	
	while (1)
	{
		i2cscanner();

		for (int i = 10; i >= 0; i--) {
			printf("Next loop in %d seconds...\n", i);
			vTaskDelay(1000 / portTICK_PERIOD_MS);
		}
		printf("Repeat now\n");
		fflush(stdout);
	} 
	
//    esp_restart();
} // app_main()


// END OF FILE