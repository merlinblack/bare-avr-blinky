#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>

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

#define LED_PIN_bm PIN0_bm
#define LED_PORT PORTF

int times[] = { 250, 250, 250, 1000, 50, 50, 50, 50, 50, 50, 50, 2000, 0 };

void update_led()
{
    static unsigned long lastTime = 0;
    static uint8_t t_index = 0;

    unsigned long currentTime = millis();

    if (currentTime > lastTime + times[t_index]) {
      LED_PORT.OUTTGL = LED_PIN_bm;
      lastTime = currentTime;
      t_index++;
      if (!times[t_index])
        t_index = 0;

      //printf("Millis: %lu\r\n", currentTime);
    }
}

char* uint_to_binary_string(uint16_t i)
{
    static char buffer[17];
    char *ptr = &buffer[0];
    uint8_t bit = 15;

    while (bit < 254)
    {
        *ptr++ = (i & (1<<bit)) ? '1' : '0';
        bit--;
    }
    *ptr = 0; // Null terminate the string.

    return &buffer[0];
}    

int main(void)
{
  init_timer();
  init_usart();
  sei();

  LED_PORT.DIRSET = LED_PIN_bm;
  LED_PORT.OUTSET = LED_PIN_bm;

  PORTF.DIRSET |= PIN1_bm | PIN2_bm;
  PORTD.DIRSET = 0xff; /* All output */

  uint8_t bitPosition = 0;
  bool up = true;
  unsigned long nextTime = 0;

  printf("Booting Double Latcher - with printf\r\nCompiled: %s %s\r\n", __DATE__, __TIME__);

  for (;;) {
      update_led();

      unsigned long currentTime = millis();
      if (currentTime > nextTime)
      {
          nextTime = currentTime+50;

          uint16_t mask = 0;

          mask |= (1<<bitPosition);

          printf("bitPosition: %2x - %5u - mask: %s - up: %s\r\n",
                 bitPosition,
                 mask,
                 uint_to_binary_string(mask),
                 up ? "true" : "false" );

          VPORTD.OUT = (mask & 0xff);
          PORTF.OUTSET = PIN1_bm;
          PORTF.OUTCLR = PIN1_bm;
          _delay_ms(1);

          VPORTD.OUT = (mask >> 8 & 0xff);
          PORTF.OUTSET = PIN2_bm;
          PORTF.OUTCLR = PIN2_bm;
          _delay_ms(1);

          VPORTD.OUT = (bitPosition);

          if (up)
              bitPosition++;
          else
              bitPosition--;

          if (bitPosition < 1)
              up = true;
          if (bitPosition > 14)
              up = false;
      }

      // Do other stuff
  }

  return 0;
}
