# Daily Development Project

日常开发项目

## 项目结构

```
daily-dev/
├── esp32_led_blink/      # ESP32-C3 LED 控制项目
├── docs/                 # 文档
├── tests/                # 测试
├── scripts/              # 脚本工具
└── README.md             # 项目说明
```

## ESP-IDF 开发

### 环境配置

```bash
export IDF_PATH=/home/zm/esp-idf
export IDF_PYTHON_ENV_PATH=$HOME/.espressif/python_env/idf6.1_py3.13_env
export ESP_IDF_VERSION=6.1
export PATH=$IDF_PYTHON_ENV_PATH/bin:$PATH
export PATH=$HOME/.espressif/tools/xtensa-esp-elf/esp-15.2.0_20251204/xtensa-esp-elf/bin:$HOME/.espressif/tools/riscv32-esp-elf/esp-15.2.0_20251204/riscv32-esp-elf/bin:$PATH
```

### 编译烧录

```bash
cd esp32_led_blink
idf.py set-target esp32c3
idf.py build
idf.py -p /dev/ttyACM0 flash monitor
```

## 快速开始

待补充
