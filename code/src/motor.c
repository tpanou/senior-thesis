
#include "motor.h"
#include "defs.h"

#include <avr/io.h>

void motor_init() {

    /* Set backtrack control line as output. */
    BCK_Y_DDR      |=  _BV(BCK_Y);
    BCK_XZ_DDR     |=  _BV(BCK_XZ);

    /* Backtrack MOSFETs should be off under normal conditions. */
    BCK_Y_PORT     &= ~_BV(BCK_Y);
    BCK_XZ_PORT    &= ~_BV(BCK_XZ);

    MTR_nLOCK_DDR  |=  _BV(MTR_nLOCK); /* Set motor AutoLock pin as output. */

    /* Set motor signal pins as output. */
    MTR_Y_DDR      |=  _BV(MTR_Y);
    MTR_XZ_DDR     |=  _BV(MTR_XZ);

    /* Trigger an interrupt whenever the specified amount of steps has been
    * completed. Steps are compared against @c OCR0A. */
    TIMSK0         |=  _BV(OCIE0A);

    /* Set signal from the limit switches as input. */
    LMT_nXZ_DDR    &= ~_BV(LMT_nXZ);
    LMT_nY_DDR     &= ~_BV(LMT_nY);

    /* Set the value that produces a 50Hz PWM frequency. */
    ICR1            =  MTR_TOP;

    /* Operate PFCPWM with @c TOP value being set in @c ICR1 (WGM13:0 = 8). */
    TCCR1B          =  _BV(WGM13);
}

static void motor_start() {
    /* This is what actually enables PWM generation and should be called after
    * preparing the Timer/Counters (velocity settings). */
    MTR_PWM_START();
}


static void motor_stop() {

    /* Stop motor PWM generation (Clock Select bits set to @c 0). */
    MTR_PWM_STOP();

    /* Stop step counter. */
    TCCR0B         &=  ~(_BV(CS02) | _BV(CS01) | _BV(CS00));

    /* Release ports from PWM operation. */
    PWM_Y_DISABLE();
    PWM_XZ_DISABLE();

    /* Deactivate roatry encoders. */
    MUX_DISABLE();

    /* Reset PWM duty cycle settings. */
    OCR1A           =  0;
    OCR1B           =  0;

    /* Release Lock pin from @c OC0A so it may be operated by software, if and
    * when needed. */
    TCCR0A         &= ~(_BV(COM0A0) | _BV(COM0A1));
}
