/**
@file
*/

#include <stdio.h>   /* e.g. FILE, stdin, fdev_setup_stream() */

/**
* @brief Handler for sending a single @c char over the USART.
*
* @param c The character to send
* @param stream Pointer to the output stream, as required by the underlying API.
* @returns Always returns 0 (success).
*/
int usart_putchar(char c, FILE* stream);

/**
* @brief Handler for receiving a single @c char over the USART.
*
* @param stream Pointer to the input stream, as required by the underlying API.
* @returns The character read.
*/
int usart_getchar(FILE* stream);

/**
* @brief Initialise the various modules.
*
* It should be noted that this function sets the motor operating range
* (motor_set_max()). As a result, upon completion, the motors will be resetting.
*/
static void init();

/**
* @brief Modifies the system clock to the desired frequency.
*/
void init_clock();

/**
* @brief Sets up the USART registers.
*/
void init_usart();
