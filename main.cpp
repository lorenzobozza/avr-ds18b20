/*
 *	Firmware - PID Controller PWM Fan (ATtiny85)
 *	Designed by Lorenzo Bozza
 * 	
 * 	MIT License
 *	Copyright (c) 2020 Lorenzo Bozza @ https://lorenzobozza.com/
 * 	OneWire Library: custom version based on Jim Studt's work
 * 	DallasTemperature Library with WConstants
 *
 *	Permission is hereby granted, free of charge, to any person obtaining a copy
 *	of this software and associated documentation files (the "Software"), to deal
 *	in the Software without restriction, including without limitation the rights
 *	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *	copies of the Software, and to permit persons to whom the Software is
 *	furnished to do so, subject to the following conditions:
 *
 *	The above copyright notice and this permission notice shall be included in all
 *	copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *	SOFTWARE.
 */

// vscode macros (ignore during compiling)
#ifndef RMDF
	#define __AVR_ATtiny85__ // Target
	#define F_CPU 8000000UL  // Clock
#endif

// Includes
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include "oneWire/oneWire.h" // Custom for C-AVR
#include "ds18b20/DallasTemperature.h"

OneWire oneWire(&PINB, (1 << PB3));
DallasTemperature sensor(&oneWire);

int main(void) {
	//sensor.begin();

	// PID parameters
	const float kP = 2.2f;
	const float kI = 1.4f;
	const float kD = 2.6f;

	// Main variables
	float temp = 0.0f;
	float err = 0.0f;
	float integrator = 0.0f;
	float derivator = 0.0f;
	uint8_t timer = 4;

	// Setup TIM1
	DDRB |= (1 << DDB1); //Pin 1 Out
	TCNT1 = 0;
	TCCR1 = 0;
	TCCR1 |= (1 << CTC1) | (1 << COM1A1) | (1 << PWM1A); //(12.3.1)
	GTCCR |= (1 << PSR1); //(12.3.2)
	TCCR1 |= (1 << CS11) | (1 << CS10); //table 12-5
	OCR1C = 80; // Clock Compare
	OCR1A = 10; // Duty Cycle Compare

	// Loop
	for (;;)
	{ 
		sensor.requestTemperatures();

		_delay_ms(1000);

		temp = sensor.getTempCByIndex(0);

		err = temp - 20; // temp-20 = set in above 20°C

		err = (kP*err) + (kI*integrator) + (kD*(err-derivator));
		
		if(err < 5)
		{
			OCR1A = 0U; // Duty Cycle 0%
		}
		else if (err > 60)
		{
			OCR1A = 80U; // Duty Cycle 100%
		}
		else
		{
			OCR1A = (uint8_t)err;
		}

		// Update parameters (every 4s)
		if( ++timer > 4 )
		{
			// Integrative (temp-28 = set in above 28°C)
			if( (temp-28 > 0) || (integrator > 0) ) // Stop integrating with both error and summation below 0.1
				integrator += temp - 28;

			if( integrator > 60 ) // Prevent stall
				integrator = 60;
			
			// Derivative
			derivator = temp - 20;

			timer = 0;
		}
	}
	
	return 0;
}
