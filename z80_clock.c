#include <avr/io.h>
#include <avr/interrupt.h>
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
	TCB3.CTRLB		 = TCB_CNTMODE_INT_gc;
	TCB3.CCMP			= TIME_TRACKING_TIMER_COUNT - 1;
	TCB3.INTCTRL	|= TCB_CAPT_bm;
	TCB3.CTRLA		 = TCB_CLKSEL_CLKDIV1_gc;
	TCB3.CTRLA		|= TCB_ENABLE_bm;
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

typedef struct {
	// Name for debugging
	char name[5];
	unsigned long time_pressed;
	bool pressed;
	uint8_t mask;
	PORT_t *port;
} button;

#define DEBOUNCE_TIME 5

bool buttonPressedAndReleased(button *btn, unsigned long currentTime)
{
	uint8_t value = btn->port->IN & btn->mask;

	if (btn->time_pressed == 0) {
		if (!value) {
			btn->time_pressed = currentTime;
		}
	}
	else {
		if (!btn->pressed && currentTime > btn->time_pressed + DEBOUNCE_TIME && !value) {
			btn->pressed = true;
			printf( "%lu Button %s pressed.\r\n", currentTime, btn->name );
		}
		if (value) {
			btn->time_pressed = 0;
			if (btn->pressed) {
				btn->pressed = false;
				printf( "%lu Button %s released.\r\n", currentTime, btn->name );
				return true; // Button was pressed, now released.
			}
		}
	}
	return false;
}

#define CLOCK_PIN_bm PIN0_bm
#define CLOCK_PORT PORTF

#define STEP_PIN_bm PIN1_bm
#define STEP_PORT PORTF

#define MODE_PIN_bm PIN2_bm
#define MODE_PORT PORTF

int main(void)
{
	init_timer();
	init_usart();
	sei();

	CLOCK_PORT.DIRSET = CLOCK_PIN_bm;
	CLOCK_PORT.OUTSET = CLOCK_PIN_bm;
	STEP_PORT.DIRCLR = STEP_PIN_bm;
	STEP_PORT.PIN1CTRL |= PORT_PULLUPEN_bm;
	MODE_PORT.DIRCLR = MODE_PIN_bm;
	MODE_PORT.PIN2CTRL |= PORT_PULLUPEN_bm;

	button step_btn = { "Step", 0, false, STEP_PIN_bm, &STEP_PORT };
	button mode_btn = { "Mode", 0, false, MODE_PIN_bm, &MODE_PORT };

	printf("Booting Z80 test clock\r\nCompiled: %s %s\r\n", __DATE__, __TIME__);

	unsigned long lastTime = 0;
	bool stepMode = true;
	bool run = false;

	for (;;) {
		unsigned long currentTime = millis();

		if (run && currentTime > lastTime + 250) {
			CLOCK_PORT.OUTTGL = CLOCK_PIN_bm;
			lastTime = currentTime;
			if (stepMode)
				run = false;
		}

		if (run == false && stepMode == true && buttonPressedAndReleased(&step_btn, currentTime)) {
			lastTime = currentTime;
			run = true;
			CLOCK_PORT.OUTSET = CLOCK_PIN_bm;
		}

		if (buttonPressedAndReleased(&mode_btn, currentTime)) {
			stepMode = !stepMode;
			if (stepMode) {
				CLOCK_PORT.OUTCLR = CLOCK_PIN_bm;
			}
			else {
				run = true;
			}
		}
		printf("%10lu - %s %s\r", 
				currentTime, 
				(stepMode) ? "Step" : "Run",
				(run) ? "Running" : "       "
				);
	}

	return 0;
}
