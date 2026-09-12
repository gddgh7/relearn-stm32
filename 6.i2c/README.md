# 6.i2c

I2C1 bus scanner example.

- SCL: PB6
- SDA: PB7
- Speed: 100 kHz
- Output: USART2 through the onboard CH343P
- Behavior: scans 7-bit addresses from `0x01` to `0x7F`