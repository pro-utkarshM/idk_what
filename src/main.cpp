/*
=========================================================================
  Ultrasonic Parametric Speaker Source Code for STM32
  Copyright (C) 2019,2020 Gene Ruebsamen

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

==========================================================================
*/

#include <Arduino.h>

// 1800 = 40Khz, 3000 = 24Khz
#define PWM_OVERFLOW 1800
#define PWM_OUT PA8       // PWM output
#define PWM_OUT_COMP PB13 // complementary output
#define ANALOG_PIN PA7
#define ADC_MAX_VALUE 4095U
#define PWM_MIDPOINT (PWM_OVERFLOW / 2U)

HardwareTimer hTimer1 = HardwareTimer(1);
volatile uint16_t latestSample = 0;
uint8_t count = 0;

void isr(void);

uint8_t  sine_wave[16] = {
0x80,0xb0,0xda,0xf5,0xff,0xf5,0xda,0xb0,
0x80,0x4f,0x25,0x0a,0x00,0x0a,0x25,0x4f
};

void setup() {
  pinMode(ANALOG_PIN, INPUT_ANALOG);
  pinMode(PWM_OUT, PWM);
  pinMode(PWM_OUT_COMP, PWM);
  pinMode(PC13, OUTPUT);

 /*
  * Maple-core TIM1 setup:
  * - PWM is emitted on PA8 via pwmWrite()
  * - channel 4 compare generates the carrier-rate interrupt
  */
  hTimer1.pause();
  hTimer1.setPrescaleFactor(1);
  hTimer1.setOverflow(PWM_OVERFLOW);
  hTimer1.setMode(4, TIMER_OUTPUT_COMPARE);
  hTimer1.setCompare(4, PWM_OVERFLOW);
  hTimer1.attachInterrupt(4, isr);

  timer_dev *t = TIMER1;
  timer_reg_map r = t->regs;

  // Enable TIM1 complementary output so PB13 mirrors the PA8 drive stage.
  bitSet(r.adv->CCER, 0);
  bitSet(r.adv->CCER, 2);

  hTimer1.refresh();
  hTimer1.resume();
  pwmWrite(PWM_OUT, PWM_MIDPOINT);
}

void loop() {
    // Blink the LED so we know we haven't crashed
    digitalWrite(PC13, HIGH);
    delay(1000);
    digitalWrite(PC13, LOW);
    delay(1000);
}

/* 
 * Fire Interrupt on Timer Overflow 
 * MODULATION SCHEME - change output duty cycle based on adc reading
 */
void isr(void) {
  latestSample = analogRead(ANALOG_PIN);
  const uint16_t pDuty = (latestSample * (PWM_MIDPOINT - 1U)) / ADC_MAX_VALUE;
  pwmWrite(PWM_OUT, PWM_MIDPOINT + pDuty);

  // alternate modulation #2 - louder but worse quality
  //uint16_t pDuty = (uint16_t)map(buffer[0],0,4095,0,PWM_OVERFLOW);
  //pwmWrite(PWM_OUT,pDuty-1);

  // alternate #3
  //pwmWrite(PWM_OUT,(((PWM_OVERFLOW-2) * buffer[0]/4096))+1);


  // slower method performing an ADC on each interrupt
  //int16_t pDuty = (PWM_OVERFLOW -2)/2 * sine_wave[count++]/255;
  //int16_t pDuty = (PWM_OVERFLOW -2)/2 * ((float)analogRead(PA7)/4095);
  //int16_t pDuty = (PWM_OVERFLOW -2)/2 * ((float)analogRead(PA7)/4095);
  //pwmWrite(PWM_OUT,pDuty + (PWM_OVERFLOW / 2));

  // sine wave test
  //int16_t pDuty = PWM_OVERFLOW/2 * sine_wave[count++]/128;
  //pwmWrite(PWM_OUT,pDuty);
  //if(count >= 15) 
  //  count = 0;
}
