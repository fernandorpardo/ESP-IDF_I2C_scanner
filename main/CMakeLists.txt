idf_component_register(
    SRC_DIRS  
	"."
    INCLUDE_DIRS
	"$ENV{IDF_PATH}/components/esp_driver_i2c"
	REQUIRES driver
	PRIV_REQUIRES spi_flash
)
