# Daily Development Project

日常开发项目

## 项目结构

```
daily-dev/
├── esp32_wifi_sta/      # ESP32-C3 WiFi + LED + ST7735 项目
├── docs/                 # 文档
├── tests/                # 测试
├── scripts/              # 脚本工具
└── README.md             # 项目说明
```

## ESP32-C3 WiFi + ST7735 项目

### 功能特性
- ✅ WiFi 连接（SSID: 350love）
- ✅ LED 控制（GPIO12/GPIO13，独立任务）
- ✅ ST7735 1.8 寸 TFT 屏幕驱动（SPI 接口）
- ✅ FreeRTOS 多任务架构

### 硬件接线

#### ST7735 屏幕接线（SPI）
```
ST7735          ESP32-C3
------          --------
VCC    ------>  3.3V (管脚 26)
GND    ------>  GND (管脚 25)
MOSI   ------>  GPIO3 (管脚 03)
SCLK   ------>  GPIO2 (管脚 02)
CS     ------>  GPIO7 (管脚 23)
DC     ------>  GPIO6 (管脚 22)
RST    ------>  GPIO10 (管脚 21)
```

#### LED 接线
- D4 LED -> GPIO12
- D5 LED -> GPIO13

### ESP-IDF 开发

#### 环境配置

```bash
export IDF_PATH=/home/zm/esp-idf
export IDF_PYTHON_ENV_PATH=$HOME/.espressif/python_env/idf6.1_py3.13_env
export ESP_IDF_VERSION=6.1
export PATH=$IDF_PYTHON_ENV_PATH/bin:$PATH
export PATH=$HOME/.espressif/tools/xtensa-esp-elf/esp-15.2.0_20251204/xtensa-esp-elf/bin:$HOME/.espressif/tools/riscv32-esp-elf/esp-15.2.0_20251204/riscv32-esp-elf/bin:$PATH
```

#### 编译烧录

```bash
cd esp32_wifi_sta
idf.py set-target esp32c3
idf.py build
idf.py -p /dev/ttyACM0 flash monitor
```

## 快速开始

待补充
