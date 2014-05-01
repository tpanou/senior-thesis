/**
@file
*/
#ifndef DEFS_H_INCL
#define DEFS_H_INCL

/**
@brief The frequency of CPU, required by <avr/delay.h> convenience functions.
*/
#define F_CPU 16000000UL

/**
@brief USART baud rate.

For the current CPU clock frequency (4MHz), in Asynchronous normal mode, a
minimal error of 0.2\% is achieved at a baud rate of 19.2kbps. *Atmel
pp.190-193.*
*/
#define USART_BAUD (19200)

/**
@brief Calculates the value for the baud rate register.

For Asynchronous normal mode, the value of UBRR is calculated from the following
formula: \f[ UBRR = \frac{f_{OSC}}{16 BAUD} - 1 \f] where\n
\f$f_{OSC}\f$ is the System Oscillater clock frequency which, in this case is
down-scaled to 4MHz, and\n
\f$BAUD\f$ is the baud rate (in bps).
*/
#define UBRR_VALUE (int)(F_CPU/16/USART_BAUD - 1)

#endif /* DEFS_H_INCL */
