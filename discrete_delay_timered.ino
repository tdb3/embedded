// Timer-based pin delay.
// Echoes the state of a line input pin (PB0) to a line output pin (PD7), 
// delayed by user-configurable delay (PC3 through PC0), ranging from 10ms (0b0000),  to 160ms (0b1111).
// Delay configuration pins (PC3 through PC0) have pullup resistors (so tying none to GND results in a delay of 160ms).
// Fairly precise, since a timer is used.
// However, line input pin fluctuations within the delay window will produce unexpected results,
// for example, missed input pulse.
// DO NOT induce pin fluctuations within the delay window.
// Accuracy depends on the accuracy of the microcontroller's oscillator.
// Includes built-in 250ms pin toggling on pin PD6 (e.g., this can be tied to PB0 to have PB0 receive a 2Hz square wave).
// Accreditation should be done via logic analyzer / oscilloscope, or similar.


byte outputValue = 0; // stores the state that will be used to set the line output pin
int lineDelayIn10msChunks = 0;
int calculated_OCR1A = 0;


void setup() {
  // Use port C (C3, C2, C1, C0) as the 4-bit nibble used to determine delay
  // These are atmega328p pins 26/25/24/23 or labeled pins A3/A2/A1/A0 (17/16/15/14) respectively
  DDRC &= ~((1 << PC3) | (1 << PC2) | (1 << PC1) | (1 << PC0));  // set as inputs (set direction value to 0)
  PORTC |= ((1 << PC3) | (1 << PC2) | (1 << PC1) | (1 << PC0));  // enable internal pullup resistors

  // Use port B (B0) as the line input pin
  // This is atmega328p pin 12, or labeled pin 8 respectively
  DDRB &= ~(1 << PB0);  // set as input
  PORTB |= (1 << PB0);  // enable internal pullup resistor (not strictly needed for driven line, but done to prevent ambiguity)

  // Use port D (D7) as the line output pin
  // This is atmega328p pin 11, or labeled pin 7 respectively, and atmega328p pin 10, or labeled pin 6 respectively
  DDRD |= (1 << PD7) | (1 << PD6);  // set delayed output (PD7) and built-in toggle (PD6) as outputs.
  PORTD &= ~((1 << PD7) | (1 << PD6));  // initially set both to 0.

  lineDelayIn10msChunks = ((0x0f & PINC)+1); //((0x0f & PINC)+1) * 10;
  calculated_OCR1A = (lineDelayIn10msChunks * F_CPU / 6400)-1; // For 8Mhz, 10ms would correspond to 1*8000000/6400=1250 (16mhz, 1*16000000/6400=1250), 160ms 16*8000000/6400=20000 (16Mhz, 16*16000000/6400=40000)

  // setup pin state change interrupt for the line input pin
  PCICR |= (1 << PCIE0);  // enable pin change interrupts on port B
  PCMSK0 |= (1 << PCINT0);  // corresponds to PB0

  // Set Timer1 values
  TCCR1A = 0; // not needed, set to 0
  TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10);  // Turn on CTC mode, and set the clk/64 prescaler.
  
  sei(); // enable interrupts
}


ISR(PCINT0_vect) {
  
  // Set Timer1 to correspond to the chosen delay 
  cli(); // temporarily disable interrupts

  TCNT1 = 0; // initialize counter to 0
  OCR1A = calculated_OCR1A;
  TIFR1 = (1<<OCF1A); // Clear flag, if set
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt.

  outputValue = PINB & (1 << PB0);

  sei();
}


ISR(TIMER1_COMPA_vect) {
  
  // Set the line output pin
  if(outputValue) {
    PORTD |= (1 << PD7);
  } else {
    PORTD &= ~(1 << PD7);
  }

  // Disable this timer interrupt (will be enabled on next pin state change)
  TIMSK1 &= ~(1 << OCIE1A);
}

void loop() {
  delay(250);
  PORTD ^= (1 << PD6);
}
