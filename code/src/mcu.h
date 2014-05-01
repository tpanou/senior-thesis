/**
@file
*/

#include<stdio.h>   /* e.g. FILE, stdin, fdev_setup_stream() */

/**
@brief Handler for sending a single @c char over the USART.
@param c The character to send
@param stream Pointer to the output stream, as required by the underlying API.
@returns Always returns 0 (success).
*/
int usart_putchar(char c, FILE* stream);

/**
@brief Handler for receiving a single @c char over the USART.
@param stream Pointer to the input stream, as required by the underlying API.
@returns The character read.
*/
int usart_getchar(FILE* stream);

/**
@brief Modifies the system clock to the desired frequency.
*/
void init_clock();

/**
@brief Sets up the USART registers.
*/
void init_usart();

/**
@brief Output stream over the USART.

This stream is configured as the default output and error streams (stdout,
stderr).
*/
FILE usart_output = FDEV_SETUP_STREAM(usart_putchar, NULL, _FDEV_SETUP_WRITE);

/**
@brief Input stream over the USART.

This stream is configured as the default input stream (stdin).
*/
FILE usart_input  = FDEV_SETUP_STREAM(NULL, usart_getchar, _FDEV_SETUP_READ);
