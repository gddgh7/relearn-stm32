# 10.buzzer

Passive buzzer PWM example.

- Buzzer output: PB9, TIM4 channel 4
- PWM frequency: about 2 kHz
- Control: send `on` or `off` through USART2 and press Enter
- Note: PB9 is shared with the relay, so use this example separately