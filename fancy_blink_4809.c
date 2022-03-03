#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#define TIME_TRACKING_TIMER_COUNT (F_CPU / 1000) // Should correspond to exactly 1 ms, i.e. millis()
volatile unsigned long timer_millis = 0;

ISR(TCB3_INT_vect)
{
    timer_millis++;

    /** clear flag **/
    TCB3.INTFLAGS = TCB_CAPT_bm;
}

void init_timer()
{
  _PROTECTED_WRITE(CLKCTRL_MCLKCTRLB, 0x00);
  TCB3.CTRLB     = TCB_CNTMODE_INT_gc;
  TCB3.CCMP      = TIME_TRACKING_TIMER_COUNT - 1;
  TCB3.INTCTRL  |= TCB_CAPT_bm;
  TCB3.CTRLA     = TCB_CLKSEL_CLKDIV1_gc;
  TCB3.CTRLA    |= TCB_ENABLE_bm;
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

void usart_send_char(char c)
{
    /* Wait for TX register to ready for another byte */
    while (!(USART0.STATUS & USART_DREIF_bm))
    {
        ;
    }
    /* Send byte */
    USART0.TXDATAL = c;
}

int usart_print_char(char c, FILE *stream)
{
    usart_send_char(c);
    return 0;
}

FILE usart_stream = FDEV_SETUP_STREAM(usart_print_char, NULL, _FDEV_SETUP_WRITE);

#define USART_BAUD_RATE(BAUD_RATE) ((uint16_t)((float)F_CPU * 64 / (16 * (float)BAUD_RATE) + 0.5))

void init_usart()
{
    /* Set baud rate */
    USART0.BAUD = USART_BAUD_RATE(115200);
    /* Enable TX for USART0 */
    USART0.CTRLB |= USART_TXEN_bm;
    /* Set TX pin to output */
    PORTA.DIR |= PIN0_bm;
    /* Redirect stdout */
    stdout = &usart_stream;
}

/************************************************************************************************/

int times[] = { 250, 250, 250, 1000, 50, 50, 50, 50, 50, 50, 50, 2000, 0 };

#define LED_PIN_bm PIN7_bm
#define LED_PORT PORTA

int main(void)
{
  init_timer();
  init_usart();
  sei();

  LED_PORT.DIRSET = LED_PIN_bm;
  LED_PORT.OUTSET = LED_PIN_bm;

  printf("Booting Fancy Blink - with printf\r\nCompiled: %s %s\r\n", __DATE__, __TIME__);

  unsigned long lastTime = 0;
  uint8_t t_index = 0;

  for (;;) {
    unsigned long currentTime = millis();

    if (currentTime > lastTime + times[t_index]) {
      LED_PORT.OUTTGL = LED_PIN_bm;
      lastTime = currentTime;
      t_index++;
      if (!times[t_index])
        t_index = 0;

      printf("Millis: %lu\r\n", currentTime);
    }

    // Do other stuff

  }

  return 0;
}
