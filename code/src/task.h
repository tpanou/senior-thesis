/**
* @file
* @addtogroup task Task
*/

#ifndef TASK_H_INCL
#define TASK_H_INCL

#include "defs.h"

#include <inttypes.h>

/**
* @brief Settings of automated tasks.
*/
typedef struct {
    /**
    * The time elapsed between successive automated samplings. The range is @c
    * 0--@c 240, with @c 0 being none. Each unit describes 6 minutes. So, a
    * value of @c 60 implies @c load samplings per hour.
    */
    uint8_t interval;

    /** @brief The amount of samples to take after each @c interval. */
    uint8_t load;
} Task;

/**
* @brief Initialise task module dependencies.
*/
void task_init();

/**
* @brief Set the automatic sampling rate.
*
* @param[in,out] t The new settings; on error, the maximum allowable value of
*   each member is set.
* @returns @c 0, if the settings were acceptable (and, thus, stored); @c -1,
*   otherwise, in which case @p task is altered to contain the maximum values.
*/
int8_t task_set(Task* t);

/**
* @brief Get the current configuration for automated sampling.
*
* @param[out] t See corresponding parameter of task_set().
*/
void task_get(Task* t);

/**
* @brief Initiate a chain of samplings.
*
* The positions to be sampled are determined by the internal algorithm.
*
* @param[in] count The total amount of samples to take.
*/
void task_log_samples(uint8_t count);

/**
* @brief Initiate a single sampling at the specified position.
*
* Note that this may fail if axis Z is manipulated externally.
*
* @param[in] pos The position to sample. Member @c z is ignored.
* @returns @c 0, if @p pos is valid; @c -1, otherwise.
*/
uint8_t task_log_sample(Position* pos);

/**
* @brief Returns whether there are registered tasks still in progress.
*
* @returns @c 0, if there are not; non-zero, otherwise.
*/
uint8_t task_pending();

/**
* @brief Create an acceptable random coordinate.
*
* The coordinates returned by this function respect the current operating range
* (see, motor_get_max()). The initial values of @p x and @p y are used as part
* of the seeding.
*
* @param[in, out] x The generated @c x position.
* @param[in, out] y The generated @c y position.
*/
static void make_target(uint8_t* x, uint8_t* y);

/**
* @brief Motor event handler.
*
* It is responsible for performing the requested amount of measurements and
* logging their results.
*
* @param[in] pos The position of the device head, as given by #motor_callback.
* @param[in] evt Status code describing the nature of the event, as given by
*   #motor_callback.
*/
static void task_handle_motor(Position pos, uint8_t evt);

#endif /* TASK_H_INCL */
