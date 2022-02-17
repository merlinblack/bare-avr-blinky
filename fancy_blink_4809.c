#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define TIME_TRACKING_TIMER_COUNT (F_CPU / 1000) // Should correspond to exactly 1 ms, i.e. millis()
volatile unsigned long timer_millis = 0;

ISR(TCB0_INT_vect)
{
    timer_millis++;

    /** clear flag **/
    TCB0.INTFLAGS = TCB_CAPT_bm;
}

void init_timer()
{
  _PROTECTED_WRITE(CLKCTRL_MCLKCTRLB, 0x00);
  TCB0.CTRLB     = TCB_CNTMODE_INT_gc;
  TCB0.CCMP      = TIME_TRACKING_TIMER_COUNT - 1;
  TCB0.INTCTRL  |= TCB_CAPT_bm;
  TCB0.CTRLA     = TCB_CLKSEL_CLKDIV1_gc;
  TCB0.CTRLA    |= TCB_ENABLE_bm;
  sei();
}

unsigned long millis()
{
  unsigned long m;
  uint8_t oldSREG = SREG;

  // copy millis into 'm' atomically by disabling interrupts
  cli();
  m = timer_millis;
  SREG = oldSREG;

  return m;
}

int times[] = { 250, 250, 250, 1000, 50, 50, 50, 50, 50, 50, 50, 2000, 0 };

#define LEDPIN 7
#define LEDDDR PORTA.DIR
#define LEDPORT PORTA.OUT

int main(void)
{
  init_timer();

  LEDDDR |= _BV(LEDPIN);
  LEDPORT = _BV(LEDPIN);

  unsigned long lastTime = 0;
  uint8_t t_index = 0;

  for (;;) {
    unsigned long currentTime = millis();

    if (currentTime > lastTime + times[t_index]) {
      LEDPORT ^= _BV(LEDPIN);
      lastTime = currentTime;
      t_index++;
      if (!times[t_index])
        t_index = 0;
    }

    // Do other stuff

  }

  return 0;
}
