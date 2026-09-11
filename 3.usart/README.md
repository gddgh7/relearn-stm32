# 3.usart

USART2 echo example for the KEYSKING F1 board and onboard CH343P USB-UART.

- TX: PA2
- RX: PA3
- Format: 115200 baud, 8 data bits, 1 stop bit, no parity
- Behavior: sends a startup message, echoes received bytes, and controls the green LED with `on` or `off`