
#include "motor.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

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
static Position new_pos;

/**
* @ingroup motor
* @brief Pointer to function to call on various motor events.
*/
static void (*motor_callback)(Position pos, uint8_t status);

/**
* @ingroup motor
* @brief Operating (maximum) limits of the device.
*
* The operating limits are managed via motor_get_max() and motor_set_max() and
* may be set to anything less than or equal to #GRID_X_LEN, #GRID_Y_LEN and
* #GRID_Z_LEN.
*/
static Position max_pos = {.x = GRID_X_LEN, .y = GRID_Y_LEN, .z = GRID_Z_LEN};

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

    /* Enable pin change interrupts on pins connected to the limit switches. */
    PCICR          |=  _BV(LMT_PCIE);
    LMT_PCMSK      |=  LMT_PCMSK_VAL;
}

void motor_set_callback(void (*callback)(Position pos, uint8_t event)) {
    motor_callback  =  callback;
}

void motor_reset() {

    /* Begin with resetting axis Z if no reset is already in progress. It is
    * imperative to first retract on axis Z separately from the others. Once
    * that has been dealt with, axes X and Y may follow. */
    if(bit_is_clear(motor_status, MTR_RESET)) {
        motor_stop();
        motor_status   |=  _BV(MTR_RESET) | _BV(MTR_IS_Z);
        setup_axis(AXIS_Z, MTR_INC);
        LOCK_DISABLE();
        motor_start();

    /* Axis Z has been reset. Now, axes X and Y may follow. */
    } else if(bit_is_set(motor_status, MTR_RESET_Z_DONE)) {
        /* Remove flag denoting axis Z is resetting. */
        motor_status   &= ~(_BV(MTR_IS_Z) | _BV(MTR_RESET_Z_DONE));

        /* Reset axes X and Y. */
        setup_axis(AXIS_Y, MTR_DEC);
        setup_axis(AXIS_X, MTR_DEC);
        LOCK_DISABLE();     /* Manually enable PWM propagation. */
        motor_start();

    /* All resetting stages have been completed. Reset #cur_pos and flags. */
    } else if(bit_is_set(motor_status, MTR_RESET_X_DONE)
           && bit_is_set(motor_status, MTR_RESET_Y_DONE)) {

        cur_pos.x       =  0;
        cur_pos.y       =  0;
        cur_pos.z       =  max_pos.z;
        motor_stop();

        LOCK_ENABLE();

        /* If this resetting cycle was initiated as a response to a limit being
        * engaged while under normal motor operation, retry reaching #new_pos
        * anew. */
        if(bit_is_set(motor_status, MTR_LIMIT)) {
            if(motor_update()) motor_stop();
        } else {
            new_pos     =  cur_pos;
        }

        motor_status   &= ~(_BV(MTR_RESET) | _BV(MTR_LIMIT)
                          | _BV(MTR_RESET_X_DONE) | _BV(MTR_RESET_Y_DONE));
        motor_status   |=  _BV(MTR_IS_RST_FRESH);

    /* Reset is in progress. */
    } else {
    }
}

void motor_get_max(Position* max) {
    max->x      =  max_pos.x;
    max->y      =  max_pos.y;
    max->z      =  max_pos.z;
}

int8_t motor_set_max(Position* max) {
    if(max->x > GRID_X_LEN || max->y > GRID_Y_LEN || max->z > GRID_Z_LEN) {
        return -1;
    }
    max_pos.x   =  max->x;
    max_pos.y   =  max->y;
    max_pos.z   =  max->z;
    motor_reset();
    return 0;
}

int8_t motor_set(Position target) {

    /* Fail, if the motors are resetting or otherwise operated upon. */
    if(bit_is_set(motor_status, MTR_RESET) || PWM_IS_ON()) return -1;

    /* Fail, if target coordinates lay outside the available device space. */
    if(target.x > max_pos.x || target.y > max_pos.y || target.z > max_pos.z) {
        return -1;
    }

    new_pos     =  target;

    return motor_update();
}

int8_t motor_get(Position *pos) {
    if(bit_is_set(motor_status, MTR_RESET) || PWM_IS_ON()) return -1;
    pos->x      =  cur_pos.x;
    pos->y      =  cur_pos.y;
    pos->z      =  cur_pos.z;
    return 0;
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

    /* Due to unknown reasons, when the lock (#MTR_nLOCK) is manually operated
    * (while Timer/Counter0 is, of course, disabled), there seems to be a false
    * outcome of @c FOC0A. In particular, though @c OC0A is initially set,
    * after a few cycles, it returns to @c 0. The small delay below is enough to
    * to determine whether the forced compare match has rendered a logic high.
    * If not, it is forced again. */
    _delay_us(100);
    if(bit_is_clear(MTR_nLOCK_PIN, MTR_nLOCK))  TCCR0B    |=  _BV(FOC0A);
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

    /* Deactivate rotary encoders. */
    MUX_DISABLE();

    /* Reset PWM duty cycle settings. */
    OCR1A           =  0;
    OCR1B           =  0;

    /* Release Lock pin from @c OC0A so it may be operated by software, if and
    * when needed. */
    TCCR0A         &= ~(_BV(COM0A0) | _BV(COM0A1));

    LOCK_ENABLE();
}

static MotorAxis motor_backtrack() {
    MotorAxis   axis;   /* _BV() of one of #MTR_RESET_[X|Y|Z]_DONE. */

    /* Remove Clock Select to stop counter and make updates in @c OCR1A/B. */
    MTR_PWM_STOP();

    /* Limit X or Z detected. Axes X and Z limit switches share the same
    * circuitry; identify which one has reached its limit. */
    if(IS_LMT_nXZ()) {

        /* Reverse the (direction) of the angular velocity. */
        if(bit_is_set(motor_status, MTR_IS_Z)) {
            OCR1B       =  OCR1B == MTR_Z_INC ? MTR_Z_DEC : MTR_Z_INC;
            axis        =  AXIS_Z;

        } else {

            OCR1B       =  OCR1B == MTR_X_INC ? MTR_X_DEC : MTR_X_INC;
            axis        =  AXIS_X;
        }
        BCK_XZ_PORT    |=  _BV(BCK_XZ);
        MTR_PWM_START();

        /* Wait until the limit switch is disengaged (active low pin). */
        loop_until_bit_is_set(LMT_nXZ_PIN, LMT_nXZ);
        /* Then wait for a logic low from the rotary encoder (black stripe). */
        loop_until_bit_is_clear(MUX_2Z_PIN, MUX_2Z);

        MTR_PWM_STOP();
        BCK_XZ_PORT    &= ~_BV(BCK_XZ);
        OCR1B       =  MTR_BRAKE;
        MTR_PWM_START();

    /* Limit Y detected. */
    } else if(IS_LMT_nY()) {

        OCR1A           =  OCR1A == MTR_Y_INC ? MTR_Y_DEC : MTR_Y_INC;
        axis            =  AXIS_Y;

        BCK_Y_PORT     |=  _BV(BCK_Y);
        MTR_PWM_START();

        loop_until_bit_is_set(LMT_nY_PIN, LMT_nY);

        /* Motors X and Y reset at the same time while feedback is available
        * from the rotary encoder for X (#MTR_ROUTE_X()). Now that backtracking
        * is required on axis Y, it should receive feedback from its appropriate
        * encoder. */
        MTR_ROUTE_Y();
        loop_until_bit_is_clear(MUX_2Z_PIN, MUX_2Z);

        MTR_PWM_STOP();

        /* Reinstate feedback from motor X rotary encoder. Even if motor X has
        * already been reset, #motor_reset() is responsible for deactivating all
        * necessary circuitry, including that of MTR_ROUTE_X(). */
        MTR_ROUTE_X();
        BCK_Y_PORT     &= ~_BV(BCK_Y);
        OCR1A           =  MTR_BRAKE;
        MTR_PWM_START();

    } else {
        /*  */
        puts("@");
        axis            =  0;
    }

    return axis;
}

/**
* @ingroup motor
* @brief Responds to the completion of the specified amount of steps.
*
* It is responsible to update #cur_pos based on the amount of the steps
* performed. #motor_update() is used to determine whether further motor
* operation is required to reach the position specified by #new_pos. If not, the
* motor circuitry is deactivated.
*/
ISR(TIMER0_COMPA_vect) {
    uint8_t offset;     /* The number of grid-steps performed. */

    MTR_PWM_STOP();     /* Stop PWM generation. */
    PWM_XZ_DISABLE();   /* Release signal pins from Output Compare Registers. */
    PWM_Y_DISABLE();

    /* Determine the amount of transitions on the grid. This is the relative
    * offset from #cur_pos not taking into consideration the direction of
    * motion. */
    offset          =  STEP_TO_GRID(OCR0A);

    /* If @c OCR1A is set, a PWM signal was generated and propagated to motor Y.
    * Update #cur_pos.y by the amount of steps performed. */
    if(OCR1A) {
        /* Alter #cur_pos.y @c by @c offset in the appropriate direction. */
        cur_pos.y  += OCR1A == MTR_Y_INC ? offset : -offset;

        /* Remove any settings of PWM generation. */
        OCR1A       =  0;
    }

    /* If @c OCR1B is set, a PWM signal was generated and propagated to either
    * motor X or Z. Update #cur_pos.x or #cur_pos.z by the amount of steps
    * performed after determining which motor (X or Z) was being operated. */
    if(OCR1B) {

        if(bit_is_set(motor_status, MTR_IS_Z)) {
            motor_status   &= ~_BV(MTR_IS_Z);
            cur_pos.z      +=  OCR1B == MTR_Z_INC ? offset : -offset;

        } else {
            cur_pos.x      +=  OCR1B == MTR_X_INC ? offset : -offset;
        }

        /* Remove any settings of PWM generation. */
        OCR1B       =  0;
    }

    /* Ensure there are no more steps to perform. If there are not any,
    * completely disable the motor circuits. */
    if(motor_update()) {
        motor_stop();

        /* The motors have successfully reached their destination. Remove
        * #MTR_FRESH_RST flag as they are not reset any more. */
        motor_status   &= ~_BV(MTR_IS_RST_FRESH);

        /* Inform that motors have reached their destination. */
        MTR_CALL(cur_pos, MTR_EVT_OK);
    }
}

/**
* @ingroup motor
* @brief Responds to limit switch interrupts.
*
* If a limit is engaged under normal motor operation (ie, while attempting to
* reach #new_pos), it means the device has failed to properly track its state.
* A #motor_reset() is forced in this case. Later, the device is configured to
* reach #new_pos anew.
*
* This ISR also heavily affects the motor resetting cycle (#motor_reset()) by
* setting flags #MTR_RESET_X_DONE, #MTR_RESET_Y_DONE and #MTR_RESET_Z_DONE.
*/
ISR(PCINT1_vect) {

    /* Force a small delay during which the signal stabilizes. */
    _delay_ms(50);

    /* This ISR is executed whenever there is a Pin Change, that is from @c 1 to
    * 0 *and* vice versa! In the event of a pin settling back to @c 1 (idle)
    * after a switch has been disengaged, simply ignore it. */
    if(!IS_LMT_nXZ() && !IS_LMT_nY()) return;

    /* Deactivate step counter for it is not needed while backtracking or
    * resetting. */
    TCCR0A         &= ~(_BV(COM0A0) | _BV(COM0A1) | _BV(WGM01));
    LOCK_DISABLE();

    MotorAxis axis  =  motor_backtrack();

    /* Limit while in motor reset. */
    if(bit_is_set(motor_status, MTR_RESET)) {

        switch(axis) {
            case AXIS_X:
                motor_status       |=  _BV(MTR_RESET_X_DONE);
                break;
            case AXIS_Y:
                motor_status       |=  _BV(MTR_RESET_Y_DONE);
                break;
            case AXIS_Z:
                motor_status       |=  _BV(MTR_RESET_Z_DONE);
                break;
            default:
                puts("\n >> BACKTRACK ERROR : no axis <<\n");
                return;
        }

    /* Limits have been engaged after two successive resets. */
    } else if(bit_is_set(motor_status, MTR_IS_RST_FRESH)) {
        /* Special case: If an unexpected limit occurs right after resetting, it
        * means that position is unreachable (probably because somebody has
        * altered the physical limits). Dealing with this also means the device
        * will not fall into an infinite loop should an insurmountable obstacle
        * happen in its path. */

        /* Maybe it would be of interest to redefine the device-space dimensions
        * based on the actual current working space. Otherwise, no particular
        * action should be taken, other than *not* setting #MTR_LIMIT (which
        * indicates that getting to #new_pos should be attempted again after a
        * motor reset. */
        puts("Destination unreachable");
        motor_stop();

    /* Limit engaged while under normal motor operation. */
    } else {
        motor_status       |=  _BV(MTR_LIMIT);
        motor_stop();
        puts("Unexpected limit");
    }
    motor_reset();
}
