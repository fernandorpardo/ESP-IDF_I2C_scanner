# ESP-IDF I2C scanner
This is I2C scanner such the one from ["i2c/scanner"](https://github.com/nkolban/esp32-snippets/tree/master/i2c/scanner), but for the new I2C driver of ESP-IDF 
 
The project takes as baseline the ESP-IDF's hello_world example (c) 2010-2022 Espressif Systems
 
Code is genearted using **ESP-IDF v5.4-dev-2194-gd7ca8b94c8** for linux running on a Raspberry.
 
The target platform is ESP32-H2-DevKitM-1

## Description
The new (by Nov'24) driver is much more loquacious than the old one. This is probably because it is still in a debugging phase, and it is likely the issue now reported will change shoortly.

By default, for a unsuccessful write operaction that does not result in an ACK at bus level, which is the expected behavoiur for detecting an unattended address, the driver shows 3 traces


E (3130) i2c.master: I2C transaction unexpected nack detected

E (3140) i2c.master: s_i2c_synchronous_transaction(904): I2C transaction failed

E (3150) i2c.master: i2c_master_multi_buffer_transmit(1130): I2C transaction failed




The last two are mutted in menuconfig - Compiler options
	[*] Disable messages in ESP_RETURN_ON_* and ESP_EXIT_ON_* macros
	
For the first one you need to set  
	struct i2c_master_bus_t -> bypass_nack_log= true;

i2c_master_bus_t is defined in i2c_private.h that you need to include.

Once you get rid of the traces the otput is clean and looks like this

```
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- -- -- -- --
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
30: -- -- -- -- -- -- -- -- -- -- -- -- 3c -- -- --
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
70: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
```

## Build
Follow the regular process to generate the flash image and download into the target device ["Configure Your Projec"](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/linux-macos-setup.html#configure-your-project)
