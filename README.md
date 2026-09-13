# relearn-stm32

学习 STM32 外设的实践项目，记录从 GPIO 到常用通信接口的配置、调试和应用。

## 硬件平台

- KEYSKING F1 学习板
- 主控：STM32F103C8T6
- 板载 USB 转串口：CH343P
- 外部晶振：8 MHz，系统时钟配置为 72 MHz

## 开发环境

- VS Code
- STM32CubeMX
- CMake
- ARM GNU Toolchain
- STM32 HAL 库

## 学习例程

| 目录 | 外设 | 主要知识点 |
| --- | --- | --- |
| `1.led` | GPIO | GPIO 输出、LED 闪烁、HAL 基础调用 |
| `2.key` | GPIO | 按键轮询、低电平有效、软件消抖、状态翻转 |
| `3.usart` | USART2 | 串口初始化、收发、命令解析、回显 |
| `4.tim` | TIM3 | PWM 输出、占空比调节、呼吸灯效果 |
| `5.adc` | ADC1 | 模拟输入、电位器采样、ADC 值与电压换算 |
| `6.i2c` | I2C1 | I2C 总线初始化、7 位地址扫描、设备检测 |
| `7.servo` | TIM4 PWM | 舵机控制、50 Hz PWM、脉宽与角度对应 |
| `8.relay` | GPIO | 继电器开关控制、串口命令、低压安全测试 |
| `9.motor` | GPIO | DRV8833 电机方向控制、串口命令、停止状态 |

## 当前构建例程

CMake 当前默认构建 `9.motor`。每次切换例程时，修改
`cmake/stm32cubemx/CMakeLists.txt` 中的应用程序源文件路径，然后重新构建并烧录。

## 串口参数

使用 CH343P 查看串口输出时，配置为：

```text
115200 baud, 8 data bits, no parity, 1 stop bit
```
