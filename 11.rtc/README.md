# RTC clock example

This example demonstrates the STM32F1 RTC peripheral using the backup domain clock.

## Highlights

- RTC initialization with 24-hour format
- Backup-domain clock retention
- Initial time set once on a fresh board
- UART2 output of the current date and time every second

## Hardware

- RTC: internal backup clock inside STM32F103C8T6
- Serial: USART2 on PA2 (TX) and PA3 (RX)

## Notes

- The RTC keeps running as long as VBAT/backup domain power is maintained.
- The example prints the current time using the default serial config: 115200 8N1.
