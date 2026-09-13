# 8.relay

Relay GPIO control example.

- Control pin: PB9
- Default assumption: high level turns the relay on
- Control: send `on` or `off` through USART2 and press Enter
- Safety: test with low-voltage loads only; never connect mains voltage