/*************************************************************************

Wall clock control for old Swiss train station clocks

processor: ATtin25
crystal: 3.6864 Mhz

H-Bridge (L293) controller on PB0 and PB1, button on PB2 with internal pull-up resistor.

Interrupt Vectors:
https://www.nongnu.org/avr-libc/user-manual/group__avr__interrupts.html

*************************************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>

#define OUTPUT_1 (1<<PB0)
#define OUTPUT_2 (1<<PB1)
#define OUTPUTS (OUTPUT_1 | OUTPUT_2)
#define BUTTON (1<<PB2)


// clock: volatile ticker
#define PULSES_PER_SECOND 25
#define PULSE_WIDTH 2
#define BUTTON_LONG 25
#define clock OCR0B

#define SECOND (1<<0)
#define FAST (1<<1)
volatile uint8_t ticker = 0;
// #define ticker OCR1B
// OCR1B for a faster code and less use of memory


void init () {
  DDRB |= OUTPUTS; // define outputs
  PORTB |= BUTTON; // set pull up resistors


  /* System-clock (counter1) -------------------------------------------------
  For a description of fuse-bits see Makefile
  Fuses: CKDIV8 is on, Clock divided by 8
  Fuses: CKSEL3:0 -> 1101 -> define Crystal with more than 3 Mhz, less than 8 Mhz
  Fuses: SUT1:0 -> 11 -> slowly rising power
  
  configure the counter1 for accurate bouncing intervals
  3686400 hz / CKDIV8 / CounterPrescaler / OutputCompareRegister = 25 hz (40ms)
  3686400 hz / 8 / 1024 / 18 = 25 hz
  */

  TCCR1 = (1<<CS10) | (1<<CS11) | (1<<CS13) | (1<<CTC1); // Clear on compare match wich OCR1C, CS10 11 and 13: prescaler 1024
  OCR1C = 17; // <- set clock-interval on this counter compare register!
  // timer output compare match (with OCR1A) interrupt enable:
  // counts until OCR1C, resets to 0, and then OCR1A (set to 0 by default) triggers an interrupt.
  // There is no clearing at OCR1A, no CTC defined, but clearing defined for OCR1C
  // An alternative would be to use TOV1 interrupt (TIM1_OVF_vect), counter overflow interrupt.
  TIMSK = (1<<OCIE1A);

  /* Another alternative is to use counter 0 instead of counter 1, ISR then is called TIM0_COMPA_vect
  TCCR0A = (1<<WGM01); // CTC
  TCCR0B = (1<<CS00) | (1<<CS02); // prescaler 1024
  OCR0A = 17;
  TIMSK = (1<<OCIE0A);
  */

  sei (); // enable interrupts
}

uint8_t isPressed() {
  return ~(PINB) & (BUTTON);
}
uint8_t isDriving() {
  return PINB & OUTPUTS;
}
void clearOutputs() {
PORTB &= ~(OUTPUTS); // set both sides of the H-Bridge to 0V.
}

void toggleMinute() {
  clock = 0; // reset seconds to 0
  static uint8_t tock = 0;
  clearOutputs();
  if (tock) { // "TOCK"
    tock = 0;
    PORTB |= OUTPUT_2;
  } else { // "TICK"
    tock = 1;
    PORTB |= OUTPUT_1;
  }
}

// 25 Hz, 40 ms
ISR (TIM1_COMPA_vect) {
  clock++;
  ticker |= FAST;
  if (clock == PULSES_PER_SECOND) {
    ticker |= SECOND;
    clock = 0; // maybe not necessary, better for precision
  }
}

int main (void) {
   init ();

   static uint8_t buttonDelay;
   static uint8_t buttonPressed;
   static uint8_t isSecondPress;
   static uint8_t minuteCount = 0;

   while (1)
   {
      // Seconds: System tick from Interrupt
      if (ticker & SECOND) {
        ticker &= ~(SECOND);
        isSecondPress = 0; // reset secondPress after waiting for 1 second
        minuteCount++;
      }

      if (minuteCount == 60) {
        minuteCount = 0;
        toggleMinute(); // second is resetted by interrupt
      }

      if (ticker & FAST) {
        ticker &= ~(FAST);

        if(isPressed()) {
          buttonDelay++;
          buttonPressed = 1;
          // if the button is pressed long enough, and if outputs are off
          if ((buttonDelay >= BUTTON_LONG) && !isDriving() && (clock >= PULSE_WIDTH)) {
            toggleMinute();
          }
          if (isDriving() && (clock >= PULSE_WIDTH)) { // compare match for pulse width
            clearOutputs(); // pulses outputs only if button is pressed.
          }
        // Button not pressed, but was pressed before = release detect
        // It's no simple release if button was pressed for a long time
        } else if (buttonPressed == 1 && buttonDelay < BUTTON_LONG) {
          buttonPressed = 0; // only one release counted
          buttonDelay = 0; // no more long presses after release
          if (isSecondPress) {
            toggleMinute();
          } else {
            clock = 0; // reset second only after first press
            minuteCount = 0;
            isSecondPress = 1;
          }
        // after a long press release, clear all button states. Maybe with if statement? ->
        // } else if (buttonDelay >= BUTTON_LONG) {
        } else {
          buttonPressed = 0;
          buttonDelay = 0;
        }
      }

   }
   return 0;
}