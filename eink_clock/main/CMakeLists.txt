idf_component_register(SRCS "main.c" "epd_driver.c" "wifi.c" "rtc.c" "clock.c" "font.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_wifi esp_netif esp_event driver nvs_flash esp_driver_gpio esp_driver_spi)
