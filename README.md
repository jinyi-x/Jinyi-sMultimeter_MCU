# Jinyi's 4 Digits And Half Multimeter
Notice: Current repo ONLY contains ESP8266 driver, PCB and PC software are in other repos  
Warning: PCB repo and software repo are private yet, cause I've not done its document work.But it SHOULD public soon.

## Development Requirements
    1. ESP8266_RTOS_SDK
    2. CMake / make
    3. Any Editor You Familiar

## Compile

```bash
 mkdir build
 cd build
 cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=1
 make -j
```