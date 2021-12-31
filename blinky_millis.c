#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// prescaler is set so timer0 ticks every 64 clock cycles, and the overflow handler
// is called every 256 ticks.

#define MICROSECONDS_PER_TIMER0_OVERFLOW \
  (64UL * 256UL * 100000UL / ((F_CPU + 5UL) / 10UL))

// the whole number of milliseconds per timer0 overflow
#define MILLIS_INC (MICROSECONDS_PER_TIMER0_OVERFLOW / 1000U)

// the fractional number of milliseconds per timer0 overflow
#define FRACT_INC ((MICROSECONDS_PER_TIMER0_OVERFLOW % 1000U) >> 3)

// shift right to fit in a byte
#define FRACT_MAX (1000U >> 3)

volatile unsigned long timer0_millis = 0;
volatile unsigned char timer0_fract = 0;

#if defined(__AVR_ATtiny13__) || defined(__AVR_ATtiny13A__) || defined(__AVR_ATtiny85__)
ISR(TIM0_OVF_vect)
#else
ISR(TIMER0_OVF_vect)
#endif
{
  // Copy to local vars so they can go in registers
  unsigned long m = timer0_millis;
  unsigned char f = timer0_fract;

  f += FRACT_INC;
  if (f >= FRACT_MAX) {
    f -= FRACT_MAX;
    m += MILLIS_INC + 1;
  }
  else {
    m += MILLIS_INC;
  }

  timer0_millis = m;
  timer0_fract = f;
}


void init_timer()
{
  sei();
  // Set timer0 prescaler to 64
  TCCR0B |= _BV(CS01) | _BV(CS00);
  // Set timer0 interrupt mask - TimerOverflowInterruptEnable
  TIMSK0 |= _BV(TOIE0);
}

unsigned long millis()
{
  unsigned long m;
  uint8_t oldSREG = SREG;

  // copy millis into m atomically by disabling interrupts
  cli();
  m = timer0_millis;
  SREG = oldSREG;

  return m;
}

#define LEDPIN PB3
#define LEDDDR DDB3

int main(void)
{
  OSCCAL += 0;
  init_timer();

  DDRB |= _BV(LEDDDR);
  PORTB = 0x00;

  unsigned long lastTime = 0;

  for (;;) {
    unsigned long currentTime = millis();

    if (currentTime > lastTime + 1000) {
      PORTB ^= _BV(LEDPIN);
      lastTime = currentTime;
    }

    // Do other stuff

  }

  return 0;
}
