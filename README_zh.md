# 锦衣の四位半

Notice: 本Repo仅包含ESP8266驱动部分，PCB文件与上位机分别在其他Repo中  
Warning: 其他Repo由于某些问题暂时为Private状态，不过应该会很快Public

## 开发环境
    1. ESP8266_RTOS_SDK
    2. Cmake / make
    3. 一个用的顺手的编辑器

## 编译

```bash
mkdir build
cd build
cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=1
make -j
```
注：-DCMAKE_EXPORT_COMPILE_COMMANDS 可以不加，本参数仅用于生成compile commands db，用于某些语言服务器

## 文件结构
    1. MacroDefinition.hpp: 配置 与预定义的ADC Register Mask
    2. Network.hpp: Smartconfig与网络通信相关
    3. PeripheralController.hpp: ADC与MUX驱动
    4. main.cpp: 主程序

# 注意：该软件版本仍有极其严重的BUG，在某些特定硬件版本上将导致ESP8266无限重启