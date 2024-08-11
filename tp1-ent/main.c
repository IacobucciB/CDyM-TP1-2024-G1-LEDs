#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>

typedef uint8_t bool_t;
#define true 1
#define false 0

#define CHECK_MSEC 5
#define PRESS_MSEC 10
#define RELEASE_MSEC 10
bool_t RawKeyPressed()
{
	return (PINC & (1<<PINC0));
}
bool_t DebouncedKeyPress = false;

void DebouncedKeySwitch(bool_t *Key_changed, bool_t *Key_pressed)
{
	static uint8_t Count = RELEASE_MSEC / CHECK_MSEC;
	bool_t RawState;
	*Key_changed = false;
	*Key_pressed = DebouncedKeyPress;
	RawState = RawKeyPressed();
	if (RawState == DebouncedKeyPress)
	{
		// Set the timer which allows a change from current state
		if (DebouncedKeyPress)
		{
			Count = RELEASE_MSEC / CHECK_MSEC;
		}
		else
		{
			Count = PRESS_MSEC / CHECK_MSEC;
		}
	}
	else
	{
		// Key has changed - wait for new state to become stable
		if (--Count == 0)
		{
			// Timer expired - accept change
			DebouncedKeyPress = RawState;
			*Key_changed = true;
			*Key_pressed = DebouncedKeyPress;
			// Reset the timer
			if (DebouncedKeyPress)
			{
				Count = RELEASE_MSEC / CHECK_MSEC;
			}
			else
			{
				Count = PRESS_MSEC / CHECK_MSEC;
			}
		}
	}
}

char press_detect(char debounce)
{
	static char old_debounce;
	if (debounce != old_debounce)
	{
		old_debounce = debounce;
		if(debounce == 0)
		{
			return 1;
		}
	}
	return 0;
}

int main (void)
{
	/* Setup */
	DDRC &= ~(1<<PORTC);
	PORTC |= 1<<PORTC0;
	DDRB = 0xFF;
	unsigned char sequence = 1;
	PORTB = 0b10000001;
	uint8_t P0_3;
	uint8_t P4_7;
	uint8_t P0_7;

	bool_t key_changed;
	bool_t key_pressed;
	
	/* Loop */
	while(1)
	{
		DebouncedKeySwitch(&key_changed, &key_pressed);
		if (press_detect(key_pressed))
		{
			sequence = !sequence;
			PORTB = 0x00;
			if (sequence)
			{
				PORTB = 0b10000001;
			}
			else
			{
				PORTB = 0b00000001;
			}
		}
		
		
		if (sequence)
		{
			if (PORTB == 0b00011000)
			{
				PORTB = 0b10000001;
			}
			else
			{
				P0_3 = PORTB & 0b00001111;
				P0_3 = P0_3 << 1;
				
				P4_7 = PORTB & 0b11110000;
				P4_7 = P4_7 >> 1;
				
				PORTB = P0_3 | P4_7;
			}
			_delay_ms(100);
		}
		else
		{
			
			if (PORTB == 0b10000000)
			{
				PORTB = 0b00000001;
			}
			else
			{
				P0_7 = 0b00000001;
				P0_7 = PORTB << 1;

				PORTB = P0_7;
			}
			
			_delay_ms(100);
		}
	}
	return 0;
}

