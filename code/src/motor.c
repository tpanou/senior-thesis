
#include "motor.h"

#include <avr/io.h>

/**
* @ingroup motor
* @brief Contains flags concerning the current status of the motors.
*
* See #MTR_STATUS(), #MTR_IS_Z, #MTR_RESET, #MTR_RESET_X_DONE, #MTR_RESET_Y_DONE
* and #MTR_RESET_Z_DONE.
*/
static uint8_t motor_status = 0;

/**
* @ingroup motor
* @brief The current (absolute) position of the device apparatus.
*
* #motor_reset() should be invoked to initialize the state of both the
* (physical) device and of this variable.
*
* To move the apparatus to a new position, #motor_set() should be invoked. Also,
* see #new_pos.
*/
static Position cur_pos;

/**
* @ingroup motor
* @brief The new position the device will attempt to get to.
*
* It is updated via #motor_set() which will also activate the motors.
*/
static Position new_pos = {.x = 0, .y = 0 , .z = 0};

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

static int8_t motor_update() {
    uint8_t steps       =  0;

    /* Motion along axes X and Y takes precedence over motion along axis Z, when
    * no submersion is requested on the latter. Also, only motion along axes X
    * and Y may be combined with one another. */
    if((new_pos.x != cur_pos.x || new_pos.y != cur_pos.y)
     && new_pos.z <= cur_pos.z) {

        /* Relative offset from #cur_pos on axes X and Y. */
        int16_t rel_x   =  (int16_t)new_pos.x - (int16_t)cur_pos.x;
        int16_t rel_y   =  (int16_t)new_pos.y - (int16_t)cur_pos.y;

        /* Enable lines for PWM propagation to motor Y and set its speed. */
        if(rel_y) {
            setup_axis(AXIS_Y, rel_y); /* @c rel_y is used only for its sign. */
        }

        /* Enable lines for PWM propagation to motor X and set its speed. Motor
        * X may only be set-up after motor Y, as noted in #setup_axis(). */
        if(rel_x) {
            setup_axis(AXIS_X, rel_x); /* @c rel_x is used only for its sign. */
        }

        rel_x           =  abs(rel_x);
        rel_y           =  abs(rel_y);

        if(rel_x > 0 && rel_y > 0) {
            /* Request the common amount of steps between @c rel_x and
            * @c rel_y. */
            steps       =  rel_x >= rel_y ? GRID_TO_STEP(rel_y)
                                          : GRID_TO_STEP(rel_x);

        } else {
            /* If one of @c rel_x and @c rel_y is @c 0, then calculate the
            * amount of steps solely based on the other (non-zero) value. */
            steps       =  rel_x > 0 ? GRID_TO_STEP(rel_x)
                                     : GRID_TO_STEP(rel_y);
        }

    /* Configure motion along axis Z. */
    } else if(new_pos.z != cur_pos.z) {

        int16_t rel_z   =  (int16_t)new_pos.z - cur_pos.z;
        setup_axis(AXIS_Z, rel_z); /* @c rel_z is used only for its sign. */
        steps = GRID_TO_STEP(abs(rel_z));
        motor_status   |=  _BV(MTR_IS_Z);

    } else {
        /* Already at #new_pos. */
        motor_status   &= ~_BV(MTR_IS_Z);
        return -1;
    }

    setup_lock(steps);
    motor_start();
    return 0;
}

static void setup_lock(uint8_t steps) {

    /* Clear any previous steps. */
    TCNT0           =  0; /* Isn't this done automatically? */

    /* Number of steps to count. A transition from white to black counts as one
    * step. */
    OCR0A           =  steps;

    /* Toggle @c OC0A pin (ie, #MTR_nLOCK) on compare-match from high to low.
    * This way, the PWM signal is no longer propagated to the motors after the
    * specified amount of steps has been performed (AutoLock mechanism). In
    * order to generate output on @c OC0A (or even, produce an interrupt) based
    * on a *custom* value, CTC mode should be used instead of Normal mode (which
    * always counts up to @c MAX). */
    TCCR0A         |=  _BV(COM0A0) | _BV(WGM01);

    /* Use @c T0 pin as a clock source. @c T0 is connected to the (multiplexed)
    * @c STEP output from the optical encoders. The counter is configured to
    * increment on every falling edge, ie, on a transition from a white stripe
    * of the encoder belt to a black one. Force an initial compare match to set
    * @c OC0A and disable the lock. */
    TCCR0B         |=  _BV(FOC0A) | _BV(CS02) | _BV(CS01);
}

static void setup_axis(MotorAxis axis, MotorDir dir) {
    uint16_t speed;
    uint8_t  is_inc     =  dir >= MTR_INC;

    switch(axis) {
        case AXIS_Y:
            speed       =  is_inc ? MTR_Y_INC : MTR_Y_DEC;

            /* Axis Y uses OCR1A for PWM generation. */
            OCR1A   =  speed;   /* Set angular velocity. */
            MTR_ROUTE_Y();
            MUX_ENABLE();       /* Enable rotary encoder. */
            PWM_Y_ENABLE();     /* Override port operation for PWM waveform. */
        break;

        case AXIS_X:
        case AXIS_Z:

            /* Propagate PWM signal to either axis X or Z. */
            if(axis == AXIS_Z) {
                speed       =  is_inc ? MTR_Z_INC : MTR_Z_DEC;
                MTR_ROUTE_Z();
            } else {
                speed       =  is_inc ? MTR_X_INC : MTR_X_DEC;
                MTR_ROUTE_X();
            }

            /* Axes X and Z use OCR1B for PWM generation. */
            OCR1B       = speed;
            MUX_ENABLE();       /* Enable rotary encoder. */
            PWM_XZ_ENABLE();    /* Override port operation for PWM signal. */
        break;
    }
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
