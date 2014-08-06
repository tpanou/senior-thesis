/**
* @file
* @addtogroup task Task
*/

#ifndef TASK_H_INCL
#define TASK_H_INCL

#include <inttypes.h>

/**
* @brief Initialise task module dependencies.
*/
void task_init();

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
