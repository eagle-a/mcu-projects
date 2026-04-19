idf_component_register(SRCS "main.c" "st7735.c" "wifi.c" "led.c" "lvgl_port.c" "rtc.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_wifi esp_netif esp_event driver nvs_flash esp_driver_gpio esp_driver_spi lvgl spiffs)
