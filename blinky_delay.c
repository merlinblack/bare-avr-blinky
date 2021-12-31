#include <avr/io.h>
#include <util/delay.h>

#define LEDPIN PB3
#define LEDDDR DDB3

int main(void)
{
  DDRB |= _BV(LEDDDR);
  PORTB = 0x00;
  
  for (;;) {
    PORTB ^= _BV(LEDPIN);
    _delay_ms(1000);
  }

  return 0;
}
