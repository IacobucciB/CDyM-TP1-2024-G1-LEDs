#include <avr/io.h> // Definición de Registros del microcontrolador
#define F_CPU 16000000UL // Especifico la frecuencia de reloj del MCU en 16MHz
#include <util/delay.h>	// Retardos por software – Macros: depende de F_CPU

typedef uint8_t bool_t;	// Creacion de bool_t
#define true 1
#define false 0


// Antirrebote con un algoritmo de conteo
#define CHECK_MSEC	 5	// Leer hardware cada 5 ms
#define PRESS_MSEC	 10	// Tiempo estable antes de registrarse presionado
#define RELEASE_MSEC 10	// Tiempo estable antes de registrarse liberado
// Leer el estado del Hardware
bool_t RawKeyPressed()
{
	return (PINC & (1<<PINC0)); // Corrected reading of PINC0
}
// Esto mantiene el estado antirrebote del boton
bool_t DebouncedKeyPress = false;
// Funcion llamada cada CHECK_MSEC para el antirrebote
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

// Función para detectar pulsaciones de botón basada en un mecanismo de debounce.
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
	DDRC &=~(1<<PC0); // Establece el puerto PORTC0 como entrada (0) y deja los 7 restantes sin cambios
	PORTC |= 1<<PORTC0;	// Seteo el PORTC0 con pull-up
	DDRB = 0xFF; // Establece todo los bits del Puerto B como salida. b0 y b7 – b1 y b6 – b2 y b5 – b3 y b4
	unsigned char secuencia = 1; // Guardar en que secuencia se encuentra el programa
	PORTB = 0b10000001; // La primera secuencia es LEDs hacia el centro
	uint8_t P0_3;
	uint8_t P4_7;
	uint8_t P0_7; // Variables auxiliares para el shifteo de bits, la numeracion representa el numero de LED

	bool_t key_changed;
	bool_t key_pressed;	// Variables que guardaran los cambios del boton con el algoritmo antirrebote
	
	/* Loop */
	while(1)
	{
		DebouncedKeySwitch(&key_changed, &key_pressed);
		if (press_detect(key_pressed))
		{
			secuencia = !secuencia;
			PORTB = 0x00;
			if (secuencia)
			{
				PORTB = 0b10000001; // Establece los LEDs para iniciar la secuencia hacia el centro
			}
			else
			{
				PORTB = 0b00000001; // Establece los LEDs para iniciar la secuencia descendente
			}
		}
		
		
		if (secuencia)
		{
			// b0 y b7 – b1 y b6 – b2 y b5 – b3 y b4
			if (PORTB == 0b00011000) // Si esta en el final de la secuencia vuelvo a arrancarla
			{
				PORTB = 0b10000001;
			}
			else
			{
				// Rotar 0-3 a la izquierda y 4-7 a derecha afuera hacia adentro
				P0_3 = PORTB & 0b00001111; // Tomo los valores de los primeros 4
				P0_3 = P0_3 << 1; // Roto hacia la izquierda
				
				
				P4_7 = PORTB & 0b11110000; // Tomo los valores de los ultimos 4
				P4_7 = P4_7 >> 1; // Roto hacia la derecha
				
				PORTB = P0_3 | P4_7; // Realizo un OR entre los rotados para asignarlo a PORTB
				
				
				
			}
			_delay_ms(100); // Retardo para la visualizacion
		}
		else
		{
			
			// b7-b6-b5-b4-b3-b2-b1-b0.
			if (PORTB == 0b10000000) // Si esta en el final de la secuencia vuelvo a arrancarla
			{
				PORTB = 0b00000001;
			}
			else
			{
				P0_7 = 0b00000001;
				P0_7 = PORTB << 1; // Roto hacia la izquierda de forma sucesiva un solo bit

				PORTB = P0_7; // Lo asigno a PORTB
			}
			
			_delay_ms(100); // Retardo para la visualizacion
		}
		
		
		
	}
	
	return 0;
}

