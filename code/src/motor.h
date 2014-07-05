/**
* @file
* @addtogroup motor
* @{
*/

#ifndef MOTOR_H_INCL
#define MOTOR_H_INCL

#include "defs.h"
#include <inttypes.h>

/**
* @brief Denotes the direction of motion.
*
* Also, see #MTR_X_INC.
*/
typedef enum {
    MTR_INC     =  0,
    MTR_DEC     = -1
} MotorDir;

/**
* @brief The available device space over axis X.
*/
#define GRID_X_LEN (13)

/**
* @brief The available device space over axis Y.
*/
#define GRID_Y_LEN (15)

/**
* @brief The available device space over axis Z.
*/
#define GRID_Z_LEN (4)

/**
* @brief Activates the PWM lock, disabling signal propagation when @c OC0A is
* disconnected from pin #MTR_nLOCK.
*
*/
#define LOCK_ENABLE()       MTR_nLOCK_PORT &= ~_BV(MTR_nLOCK)

/**
* @brief Disables the PWM lock, allowing its propagation to the motors when
* @c OC0A is disconnected from pin #MTR_nLOCK.
*/
#define LOCK_DISABLE()      MTR_nLOCK_PORT |=  _BV(MTR_nLOCK)

/**
* @brief Enables generation of PWM signal on pin #MTR_Y by connecting @c OC1A to
* the port.
*
* Mode of operation is Set/Clear on compare-match. *Atmel p.132.*
*/
#define PWM_Y_ENABLE()      TCCR1A         |=  _BV(COM1A1)

/**
* @brief Disables generation of PWM signal on pin #MTR_Y by disconnecting @c
* OC1A from the port.
*/
#define PWM_Y_DISABLE()     TCCR1A         &= ~_BV(COM1A1)

/**
* @brief Enables generation of PWM signal on pin #MTR_XZ by connecting @c OC1B
* to the port.
*
* Mode of operation is Set/Clear on compare-match. *Atmel p.132.*
*/
#define PWM_XZ_ENABLE()     TCCR1A         |=  _BV(COM1B1)

/**
* @brief Disables generation of PWM signal on pin #MTR_Y by disconnecting @c
* OC1A from the port.
*/
#define PWM_XZ_DISABLE()    TCCR1A         &= ~_BV(COM1B1)

/**
* @brief Condition to determine whether any Timer/Counter1 prescaler bits are
* set.
*/
#define PWM_IS_ON()       (TCCR1B & (_BV(CS12) | _BV(CS11) | _BV(CS10)))

/**
* @brief Activates PWM generation.
*
* This macro should be called once:
* - The desired duty cycle has been set in @c OCR1A and/or @c OCR1B (for motor Y
*   and XZ, respectively).
* - @c OC1A and/ or @c OC1B has been connected to the port (#PWM_XZ_ENABLE() and
*   #PWM_Y_ENABLE()).
* - An optical encoder has been selected (#MTR_ROUTE_XZ() or #MTR_ROUTE_Y()).
* - The multiplexer has been chip-selected (#MUX_ENABLE()).
*/
#define MTR_PWM_START()     TCCR1B         |=  MTR_PRESCALER

/**
* @brief Stop motor PWM generation.
*
* Bits @c CS12, @c CS11 and @c CS10 of TCCR1B are zeroed-out.
*/
#define MTR_PWM_STOP() \
TCCR1B         &=  ~(_BV(CS12)\
                    |_BV(CS11)\
                    |_BV(CS10))

/**
* @brief Route signals from and to motor X.
*
* Sets the appropriate signals so that the PWM signal on #MTR_XZ pin is
* propagated to motor X instead of motor Z (the two being multiplexed on the
* same pin). This automatically enables the corresponding rotary encoder, the
* steps of which can be seen on #MUX_2Z pin.
*/
#define MTR_ROUTE_X() \
MUX_S0_PORT    &= ~_BV(MUX_S0);\
MUX_S1_PORT    |=  _BV(MUX_S1)

/**
* @brief Route signals from and to motor Y.
*
* Enables the rotary encoder of motor Y, the steps of which can be seen on
* #MUX_2Z pin. Unlike motors X and Z, simply disabling the AutoLock while
* starting PWM generation is sufficient to set this motor in motion.
* Consequently, if running this motor along with another (and in particular
* motor X), calling this macro could/should be omitted.
*/
#define MTR_ROUTE_Y() \
MUX_S0_PORT    &= ~_BV(MUX_S0);\
MUX_S1_PORT    &= ~_BV(MUX_S1)

/**
* @brief Route signals from and to motor Z.
*
* Sets the appropriate signals so that the PWM signal on #MTR_XZ ping is
* propagated to motor Z instead of motor X (the two being multiplexed on the
* same pin). This automatically enables the corresponding rotary encoder, the
* steps of which can be seen on #MUX_2Z pin.
*/
#define MTR_ROUTE_Z() \
MUX_S0_PORT    |=  _BV(MUX_S0);\
MUX_S1_PORT    &= ~_BV(MUX_S1)

/**
* @brief Condition to determine whether a Y-axis limit switch has been engaged.
*/
#define IS_LMT_nY()       ((LMT_nY_PIN   &  _BV(LMT_nY))  == 0)

/**
* @brief Condition to determine whether an X- or Z-axis limit switch has been
* engaged.
*/
#define IS_LMT_nXZ()      ((LMT_nXZ_PIN  &  _BV(LMT_nXZ)) == 0)

/**
* @brief Velocity setting for OCR1B to move along the positive direction on axis
* X.
*
* With a total of 5000 increments in Timer/Counter1, a PWM signal with,
* approximately, 5 to 10% duty cycle is produced with pulses that remain high
* for 250 to 500 increments, respectively. Pulses below 7.5% (ie, from 250 up to
* approx. 375) result in a Counter-Clockwise (CCW) rotation; the lower the
* value, the greater the angular velocity. Similarly, increments in the range
* 375 up to 500 produce Clockwise (CW) rotation. In this case, the greater the
* value, the greater the angular velocity, as well. Increments around 375 force
* the motor to retain its position (brakes).
*
* In practice, the values around the middle point (7.5% duty cycle) are *not*
* symmetric, in the sense that, for example, 350 and 400 do *not* necessarily
* produce angular velocities of the same magnitude. The exact behaviour is
* strictly dependent on each motor and could differ even among motors of the
* same model. In the current configuration, calibration tests have been
* performed to determine duty cycles that result in similar velocities for axes
* X and Y. This was deemed necessary to ensure smooth operation when motion in
* both these axes is requested (see #setup_axis() and #setup_lock()).
*
* While the motors are rotating, the belt and pinion linear actuator converts
* rotational to linear motion in an either increasing or decreasing direction;
* it is the positioning of the motors that determines what CW and CCW rotation
* corresponds to. In case of axes X and Z, CW rotation increases the position
* (ie, moves the apparatus in a greater position in device-space coordinates),
* while the opposite holds true for axis Y.
*
* Values producing duty cycles on axes X, Y and Z:
* - Axis X:
*   - Increment: @c 411
*   - Decrement: @c 345
* - Axis Y:
*   - Increment: @c 350
*   - Decrement: @c 412
* - Axis Z:
*   - Increment: @c 415
*   - Decrement: @c 365
*/
#define MTR_X_INC  (411)

/**
* @brief Velocity setting for OCR1B to move along the negative direction on axis
* X.
*
* For details, see #MTR_X_INC.
*/
#define MTR_X_DEC  (345)

/**
* @brief Velocity setting for OCR1A to move along the positive direction on axis
* Y.
*
* For details, see #MTR_X_INC.
*/
#define MTR_Y_INC  (350)

/**
* @brief Velocity setting for OCR1A to move along the negative direction on axis
* Y.
*
* For details, see #MTR_X_INC.
*/
#define MTR_Y_DEC  (412)

/**
* @brief Velocity setting for OCR1B to move along the positive direction on axis
* Z.
*
* For details, see #MTR_X_INC.
*/
#define MTR_Z_INC  (417)
/**
* @brief Velocity setting for OCR1B to move along the negative direction on axis
* Z.
*
* For details, see #MTR_X_INC.
*/
#define MTR_Z_DEC  (358)

/**
* @brief Converts the given amount of *grid-steps* to *pulse-steps*.
*
* Calculates the number of pulses needed to perform the given amount of
* transitions in device coordinate system.
*
* Also, see #GRID_X_LEN, #GRID_Y_LEN and #GRID_Z_LEN.
*/
#define GRID_TO_STEP(x)   (x*4 - 1)

/**
* @brief Converts the given amount of *pulse-steps* to *grid-steps*.
*
* Calculates the number of grid transitions the given amount of pulses
* corresponds to in device coordinate system.
*
* Also, see #GRID_X_LEN, #GRID_Y_LEN and #GRID_Z_LEN.
*/
#define STEP_TO_GRID(x)   ((x + 1)/4)

/**
* @brief Flag-bit of #motor_status to indicate the motors are currently
* resetting.
*
* Also, see #motor_status, #MTR_STATUS(), #MTR_IS_Z, #MTR_RESET_X_DONE,
* #MTR_RESET_Y_DONE and #MTR_RESET_Z_DONE.
*/
#define MTR_RESET           0

/**
* @brief Flag-bit of #motor_status to distinguish between motors X and Z.
*
* Motors X and Z receive their PWM signal from the same μC pin (although the
* signal is actually propagated to only one at a time -- see #MTR_XZ in relation
* to #MTR_ROUTE_X() and #MTR_ROUTE_Z()). This flag is a means to identify which
* motor is being operated upon.
*
* Also, see #motor_status and #MTR_STATUS().
*/
#define MTR_IS_Z            1

/**
* @brief Flag-bit of #motor_status to indicate the position of motor X has been
* reset.
*
* This flag is set by the ISR that responds to limit switch interrupts and is
* used by #reset_motor() to determine the state of the motor reset process.
* Motors X and Y are reset concurrently and when both their corresponding
* flag-bits (#MTR_RESET_X_DONE and #MTR_RESET_Y_DONE) are set, the operation is
* finalized by #reset_motor().
*
* Also, see #motor_status, #MTR_STATUS(), #MTR_RESET, #MTR_IS_Z and
* #MTR_RESET_Z_DONE.
*/
#define MTR_RESET_X_DONE    2

/**
* @brief Flag-bit of #motor_status to indicate the position of motor Y has been
* reset.
*
* This flag is set by the ISR that responds to limit switch interrupts and is
* used by #reset_motor() to determine the state of the motor reset process.
* Motors X and Y are reset concurrently and when both their corresponding
* flag-bits (#MTR_RESET_X_DONE and #MTR_RESET_Y_DONE) are set, the operation is
* finalized by #reset_motor().
*
* Also, see #motor_status, #MTR_STATUS(), #MTR_RESET, #MTR_IS_Z and
* #MTR_RESET_Z_DONE.
*/
#define MTR_RESET_Y_DONE    3

/**
* @brief Flag-bit of #motor_status to indicate the position of motor Z has been
* reset.
*
* Like #MTR_RESET_X_DONE and #MTR_RESET_Y_DONE, this flag is set by the ISR that
* responds to limit switch interrupts and is used by #reset_motor() to determine
* the state of the motor reset process.
*
* Motor Z is always the first motor to be reset independently from the other
* two. Once this is set (ie, motor Z has been reset), #reset_motor() will
* initiate resetting of motors X and Y.
*
* Also, see #motor_status, #MTR_STATUS(), #MTR_RESET, #MTR_IS_Z,
* #MTR_RESET_X_DONE and #MTR_RESET_Y_DONE.
*/
#define MTR_RESET_Z_DONE    4

/**
* @brief Flag-bit of #motor_status to indicate a limit has been engaged while
* under normal motor operation.
*
* This will designate to #motor_reset() that is should update the position to
* #new_pos after the reset cycle has been completed.
*/
#define MTR_LIMIT           6

/**
* @brief Convenience macro to determine whether a particular bit in
* #motor_status is set.
*
* Also, see #motor_status #MTR_STATUS(), #MTR_RESET, #MTR_IS_Z,
* #MTR_RESET_X_DONE and #MTR_RESET_Y_DONE.
*/
#define MTR_STATUS(x)      (motor_status & x)

/**
* @brief Initializes all pins and registers used for motor operation.
*
* Motors are operated through #motor_reset(), #motor_set() and #motor_get().
*/
void motor_init();

/**
* @brief Resets the motors to a known state (homing to absolute zero).
*
* This function should be invoked before attempting to use the motors
* (#motor_set() or #motor_get()).
*/
void motor_reset();

/**
* @brief Activates the appropriate motors in order to reach #new_pos.
*
* It determines which motors should be operated and for how many steps. Motors X
* and Y may be set-up to operate concurrently but only for the same distance
* (irrespective of direction); once the common part has been completed, a new
* invocation of this function will configure the other motor to perform the
* remainder of steps. Note that #cur_pos should be updated before this new
* invocation occurs. Motor Z takes precedence over motors X and Y when its new
* position (#new_pos.z) is greater than its current (#cur_pos.z) (that is, when
* the sensor head is lowered, it must first be retracted before moving to a new
* X-Y position).
*
* This function should only be called while all motors are at rest
* (#PWM_IS_ON()). Generally, this occurs from within #motor_set(), which will
* make sure no operation is already in process, and from the ISR that responds
* to the completion of the specified amount of steps (@c TIMER0_COMPA_vect).
* Typically, 3 to 4 invocations suffice; a return value of @c -1 designates that
* #cur_pos has reached #new_pos and no set-up has been performed. In this case,
* all the unnecessary circuitry (Timer/Counters and rotary encoder) should be
* disabled by the callee.
*
* This function is non-blocking and is not safe to call while the motors are
* being operated upon.
*
* @returns @c 0 upon activating the motors; @c -1 otherwise.
*/
static int8_t motor_update();

/**
* @brief Request the AutoLock to be enabled for the specified amount of steps.
*
* This function initiates the step counter, ie, any pulses sensed on pin #MUX_2Z
* after calling this function will increment the step counter. Once the amount
* of steps specified by @p steps has been reached, #MTR_nLOCK will be enabled by
* the underline hardware blocking the transmission of the PWM signal on #MTR_XZ
* and #MTR_Y. Also, an Output Compare Match Interrupt will trigger (for vector
* @c TIMER0_COMPA_vect) which, ultimately, completely disables the
* Timer/Counters.
*
* The AutoLock mechanism utilizes Timer/Counter0 for counting the steps and is,
* thus, guaranteed that only the requested amount of steps will be performed,
* even if the MCU is busy attending to other tasks when this occurs. Once the
* MCU is available to execute the aforementioned ISR, it will either forward
* another request or completely disable the Timer/Counters.
*
* @param[in] steps The amount of steps to count before the AutoLock is enabled.
*   An additional transition is always performed, ie passing in @c 1 will
*   trigger AutoLock after two steps and so on. This is due to "Compare Match
*   Blocking by TCNT0 Write" feature as described by *Atmel p97*. As a special
*   case, passing in @c 0 allows a maximum of 256 steps to be performed.
*/
static void setup_lock(uint8_t steps);

/**
* @brief Prepares motion on the specified axis and direction.
*
* This configures PWM generation and propagation to the specified motor and the
* settings for an appropriate velocity and rotary encoder feedback. Calling this
* function is generally followed by a call to #motor_start().
*
* Because #MUX_S0 and #MUX_S1 select between motors X and Z apart from which
* rotary encoder to activate, if concurrent motion is required (for instance, on
* both axes X and Y), axis Y should be the *first* one to set-up to avoid
* disabling propagation to the other. In this configuration, it is the rotary
* encoder of the *last* axis set-up that will provide feedback. Note that
* #MotorAxis values may not be OR-ed together!

* Motors X and Z share the same signal line (on pin @c OC1A) and, thus, may not
* be operated at the same time.
*
* @param[in] axis The axis to set-up.
* @param[in] dir The direction of motion.
*/
static void setup_axis(MotorAxis axis, MotorDir dir);

/**
* @brief Wrapper around #MTR_PWM_START().
*
* This function is provided as a complement to #motor_stop(). As in the case of
* #MTR_PWM_START(), which is what actually enables PWM generation, this function
* should be called *after* applying the desired velocity settings using
* #setup_axis().
*/
static void motor_start();

/**
* @brief Disables the step counter, the PWM timer and the rotary encoder.
*/
static void motor_stop();

#endif /* MOTOR_H_INCL */
/** @} */
