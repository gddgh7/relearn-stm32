# Bluetooth control example

This example uses a Bluetooth module connected to STM32 USART1 to receive serial commands and control the green LED.

## Connection

- Bluetooth TX -> PB10 (USART3_TX)
- Bluetooth RX -> PB11 (USART3_RX)
- Common GND
- Module power: 3.3 V or logic-compatible with STM32

## Behavior

- send `on` -> green LED turns on
- send `off` -> green LED turns off
- module replies with the command result over the same UART

## Serial setting

- Baud rate: 9600
- Data bits: 8
- Parity: none
- Stop bits: 1
