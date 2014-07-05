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
* @brief Initializes all pins and registers used for motor operation.
*
* Motors are operated through #motor_reset(), #motor_set() and #motor_get().
*/
void motor_init();

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
