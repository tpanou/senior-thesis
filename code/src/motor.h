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
#define GRID_X_LEN (10)

/**
* @brief The available device space over axis Y.
*/
#define GRID_Y_LEN (11)

/**
* @brief The available device space over axis Z.
*/
#define GRID_Z_LEN (4)

/**
* @brief The time it takes to move one unit on any axis.
*
* In seconds.
*/
#define MTR_UNIT_TIME       1

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
* - An optical encoder has been selected (#MTR_ROUTE_X, #MTR_ROUTE_Z() or
*   #MTR_ROUTE_Y()).
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
* Velocity setting for either OCR1A or OCR1B to stop and hold current position.
*
* This corresponds to braking. For details, see #MTR_X_INC.
*/
#define MTR_BRAKE  (380)

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
* Also, see #motor_status, #MTR_IS_Z, #MTR_RESET_X_DONE,
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
* Also, see #motor_status, #MTR_RESET, #MTR_RESET_X_DONE,
* #MTR_RESET_Y_DONE and #MTR_RESET_Z_DONE.
*/
#define MTR_IS_Z            1

/**
* @brief Flag-bit of #motor_status to indicate the position of motor X has been
* reset.
*
* This flag is set by the ISR that responds to limit switch interrupts and is
* used by #motor_reset() to determine the state of the motor reset process.
* Motors X and Y are reset concurrently and when both their corresponding
* flag-bits (#MTR_RESET_X_DONE and #MTR_RESET_Y_DONE) are set, the operation is
* finalized by #motor_reset().
*
* Also, see #motor_status, #MTR_RESET, #MTR_IS_Z and
* #MTR_RESET_Z_DONE.
*/
#define MTR_RESET_X_DONE    2

/**
* @brief Flag-bit of #motor_status to indicate the position of motor Y has been
* reset.
*
* This flag is set by the ISR that responds to limit switch interrupts and is
* used by #motor_reset() to determine the state of the motor reset process.
* Motors X and Y are reset concurrently and when both their corresponding
* flag-bits (#MTR_RESET_X_DONE and #MTR_RESET_Y_DONE) are set, the operation is
* finalized by #motor_reset().
*
* Also, see #motor_status, #MTR_RESET, #MTR_IS_Z and
* #MTR_RESET_Z_DONE.
*/
#define MTR_RESET_Y_DONE    3

/**
* @brief Flag-bit of #motor_status to indicate the position of motor Z has been
* reset.
*
* Like #MTR_RESET_X_DONE and #MTR_RESET_Y_DONE, this flag is set by the ISR that
* responds to limit switch interrupts and is used by #motor_reset() to determine
* the state of the motor reset process.
*
* Motor Z is always the first motor to be reset independently from the other
* two. Once this is set (ie, motor Z has been reset), #motor_reset() will
* initiate resetting of motors X and Y.
*
* Also, see #motor_status, #MTR_RESET, #MTR_IS_Z,
* #MTR_RESET_X_DONE and #MTR_RESET_Y_DONE.
*/
#define MTR_RESET_Z_DONE    4

/**
* @brief Flag-bit of #motor_status to indicate that no other actions has been
* performed since the last motor reset.
*
* It is cleared once #new_pos has been reached (ie, all steps have been
* performed). This flag is important in determining whether a particular
* position is unreachable. A position is deemed 'unreachable' and no further
* attempts take place when the first attempt to reach it after a fresh reset
* results in engaging a limit.
*/
#define MTR_IS_RST_FRESH    5

/**
* @brief Flag-bit of #motor_status to indicate a limit has been engaged while
* under normal motor operation.
*
* This will designate to #motor_reset() that is should update the position to
* #new_pos after the reset cycle has been completed.
*/
#define MTR_LIMIT           6

/**
* @brief A request to reposition the motors has been successfully completed.
*
* The position supplied on this event contains the current motor position. Upon
* invocation, the motors are quiescent and a new position may be requested.
*/
#define MTR_EVT_OK          1

/**
* @brief A request to reposition the motors has been successfully completed.
*
* The position supplied on this event contains the new position that will be
* attempted. Upon invocation, the motors are quiescent and the current position
* may be requested.
*/
#define MTR_EVT_BUSY        2

/**
* @brief Call motor callback if it has been set.
*/
#define MTR_CALL(pos, evt)       \
if(motor_callback)(*motor_callback)(pos, evt)

/**
* @brief Initializes all pins and registers used for motor operation.
*
* Motors are operated through #motor_reset(), #motor_set() and #motor_get().
*/
void motor_init();

/**
* @brief Function to call on various motor-related events.
*
* For a list of advertised events, see the various MTR_EVT_* macros.
*
* @param[in] callback The callback function receives the position of the motors
*   and an event code.
*/
void motor_set_callback(void (*callback)(Position pos, uint8_t event));

/**
* @brief Resets the motors to a known state (homing to absolute zero).
*
* This function should be invoked before attempting to use the motors
* (#motor_set() or #motor_get()). Any operation in progress at the time of
* invocation occurs will be terminated.
*/
void motor_reset();

/**
* @brief Return the current device operating limits.
*
* @param[out] max Variable in which to return the current maximum acceptable X,
*   Y and Z coordinates.
*/
void motor_get_max(Position* max);

/**
* @brief Set the operating limits of the device.
*
* Generally, the operating limits should not exceed the physical limits of the
* device (ie, #GRID_X_LEN, #GRID_Y_LEN and #GRID_Z_LEN). If such is the case,
* the limits are not altered. Successfully modifying the operating limits
* results in a motor reset. This function may be invoked at any time; like
* motor_reset(), any operation in progress at the time this occurs, will be
* terminated.
*
* @param[in] max Variable that holds the new operating limits.
* @returns @c 0, if the limits where acceptable; non-zero, otherwise.
*/
int8_t motor_set_max(Position* max);

/**
* @brief Move the device to the given position.
*
* Calling this function will result in operating the motors and will fail should
* the motors already be operated upon or @p target lies outside the available
* device space (#GRID_X_LEN, #GRID_Y_LEN, #GRID_Z_LEN). Also, homing should be
* performed (by calling #motor_reset()); otherwise, the behaviour is unknown.
*
* This function is non-blocking (see #motor_update()).
*
* @param[in] target The new device position.
* @returns @c 0, if @p target position is valid and the apparatus will be
*   configured to reach it; @c -1, otherwise.
*/
int8_t motor_set(Position target);

/**
* @brief Announces the current position of the device.
*
* This function will fail (return @c -1) if the motors are currently resetting
* or otherwise operated upon.
*
* @param[out] pos The value of #cur_pos.
* @returns @c 0, if @p pos was set; @c -1, otherwise.
*/
int8_t motor_get(Position *pos);

/**
* @brief Activates the appropriate motors in order to reach #new_pos.
*
* It determines which motors should be operated and for how many steps. Motors X
* and Y may be set-up to operate concurrently but only for the same distance
* (irrespective of direction); once the common part has been completed, a new
* invocation of this function will configure the other motor to perform the
* remainder of steps. Note that #cur_pos should be updated before this new
* invocation occurs. Motor Z takes precedence over motors X and Y when its new
* position (@link #new_pos new_pos.z@endlink) is greater than its current
* (@link #cur_pos cur_pos.z@endlink) (that is the
* sensor head will first be retracted and then moved on X-Y plane). The reverse
* holds true when lowering the head.
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

/**
* @brief Backtrack a motor that has engaged a limit switch.
*
* In order for this to operate correctly, @c OCR1A/B and #MTR_IS_Z, if needed,
* must be populated with an appropriate value beforehand. This means that this
* function cannot be used to backtrack on an axis the limit switch of which has
* been engaged *before* the initiation of motion (such as before device
* power-on).
*
* Note that the position of the backtracked motor in #cur_pos is not updated.
* This function should be followed by another operation to update the position
* (eg, reset).
*
* @returns The axis that has been backtracked; @c 0 in the event that no limit
*   switch signal read as logic low.
*/
static MotorAxis motor_backtrack();

#endif /* MOTOR_H_INCL */
/** @} */
