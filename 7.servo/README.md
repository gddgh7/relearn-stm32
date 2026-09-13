# 7.servo

Servo control example using timer PWM.

- PWM output: PB8, TIM4 channel 3
- PWM frequency: 50 Hz
- Pulse range: 500 to 2500 us, approximately 0 to 180 degrees
- Control: send `0`, `90`, or `180` through USART2 and press Enter