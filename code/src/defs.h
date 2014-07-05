/**
@file
*/
#ifndef DEFS_H_INCL
#define DEFS_H_INCL

#include <inttypes.h>

/**
* @brief A coordinate in device space.
*/
typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t z;
} Position;

/**
* @brief Discerns among the available axes.
*
* These constants are not to be OR-ed together.
*/
typedef enum {
    AXIS_X,
    AXIS_Y,
    AXIS_Z
} MotorAxis;

/**
@brief The frequency of CPU clock, required by <avr/delay.h> convenience
functions.
*/
#define F_CPU (4000000UL)

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

/**
* Value of @c TOP (@c OC1A) that produces pulses at 50Hz taking #MTR_PRESCALER
* into consideration.
*/
#define MTR_TOP (int)(F_CPU/2/8/50)

/**
* @brief Motor PWM prescaler bits.
*
* The prescaler to use for motor PWM generation. Granted \f$ clk_{IO} \f$
* is 4MHz (see #F_CPU), a prescaler of \f$ clk_{IO}/8 \f$ to produce a 50Hz
* signal, provides 5000 discrete steps of resolution. *Atmel pp.133--135.*
*
* The value should be the bitwise-OR operation of the appropriate bits of @c
* TCCR1B.
*/
#define MTR_PRESCALER   (_BV(CS11))

/**
* @brief Data Direction Register of pin X and Z motors connect to.
*
* Also, see #MTR_XZ_PORT and #MTR_XZ.
*/
#define MTR_XZ_DDR      DDRB

/**
* @brief The pin port X and Z motors connect to.
*
* Also, see #MTR_XZ_DDR and #MTR_XZ.
*/
#define MTR_XZ_PORT     PORTB

/**
* @brief Pin of #MTR_XZ_PORT where X and Z motors connect to.
*
* This pin is used for the transmission of the PWM signal. Note that in this
* implementation, both X and Z motors receive signal from the same pin; it is
* the value of pins #MUX_S0 and #MUX_S1 that determine which one the signal
* actually reaches to.
*
* Also, see #MTR_XZ_DDR and #MTR_XZ_PORT.
*/
#define MTR_XZ          PORTB2

/**
* @brief Data Direction Register of pin Y motor connects to.
*
* Also, see #MTR_Y_PORT and #MTR_Y.
*/
#define MTR_Y_DDR       DDRB

/**
* @brief The pin port Y motor connects to.
*
* Also, see #MTR_Y_DDR and #MTR_Y.
*/
#define MTR_Y_PORT      PORTB

/**
* @brief Pin of #MTR_Y_PORT where Y motor connects to.
*
* This pin is used for the transmission of the PWM signal.
*
* Also, see #MTR_Y_DDR and #MTR_Y_PORT.
*/
#define MTR_Y           PORTB1

/**
* @brief Data Direction Register of pin the AutoLock MOSFET's Gate connects to.
*
* Also, see #MTR_nLOCK_PORT and #MTR_nLOCK.
*/
#define MTR_nLOCK_DDR   DDRD

/**
* @brief The pin port the AutoLock MOSFET's Gate connects to.
*
* Also, see #MTR_nLOCK_PORT and #MTR_nLOCK.
*/
#define MTR_nLOCK_PORT  PORTD

/**
* @brief Pin of #MTR_nLOCK_PORT the AutoLock MOSFET's Gate connects to.
*
* This pin enables and disables the Lock and, in order for the AutoLock
* mechanism to operate, it must coincide with pin @OC0A. It is active low.
*
* Also, see #MTR_nLOCK_PORT and #MTR_nLOCK.
*/
#define MTR_nLOCK       PORTD6

/**
* @brief Data Direction Register of pin MUX @c nCS connects to.
*
* Also, see #MUX_nCS_PORT and #MUX_nCS.
*/
#define MUX_nCS_DDR     DDRD

/**
* @brief The pin port MUX @c nCS connects to.
*
* Also, see #MUX_nCS_DDR and #MUX_nCS.
*/
#define MUX_nCS_PORT    PORTD

/**
* @brief Pin of #MUX_nCS_PORT MUX @c nCS connects to.
*
* This pin enables and disables the multiplexer. It is active low.
*
* Also, see #MTR_nLOCK_PORT and #MTR_nLOCK.
*/
#define MUX_nCS         PORTD5

/**
* @brief Data Direction Register of pin MUX @c 2Z connects to.
*
* Also, see #MUX_2Z_PIN and #MUX_2Z.
*/
#define MUX_2Z_DDR      DDRD

/**
* @brief The pin port MUX @c 2Z connects to.
*
* Also, see #MUX_2Z_DDR and #MUX_2Z.
*/
#define MUX_2Z_PIN      PIND

/**
* @brief Pin of #MUX_2Z_PORT MUX @c 2Z connects to.
*
* This pin gives the output of the rotary encoders (step) of motors Y, Z and X
* on pins @c 2Y0, @c 2Y1 and @c 2Y2 of the multiplexer, respectively. Pin @c 2Y3
* gives access to the 1-wire DQ line.
*
* Also, see #MUX_2Z_PIN and #MUX_2Z_PORT.
*/
#define MUX_2Z          PORTD4

/**
* @brief Data Direction Register of pin MUX @c S0 connects to.
*
* Also, see #MUX_S0_PORT and #MUX_S0.
*/
#define MUX_S0_DDR      DDRC

/**
* @brief The pin port MUX S0 connects to.
*
* Also, see #MUX_S0_DDR and #MUX_S0.
*/
#define MUX_S0_PORT     PORTC

/**
* @brief Pin of #MUX_S0_PORT MUX @c S0 connects to.
*
* This pin along with pin #MUX_S1 select which MUX pin, @c 1Y0, @c 1Y1, @c 1Y2
* or @c 1Y3, connects to ground. The first three pins enable a rotary encoder
* (Y, Z or X, respectively) while the last one, the ADC. Apparently, these are
* all active low. They also select one of MUX @c 2Y0, 2Y1, 2Y2 or 2Y3 as input
* on pin #MUX_2Z. The first three correspond to the step (feedback) of a rotary
* encoder (in same order presented above) or, in case of @c 2Y3, the 1-wire DQ
* line.
*
* Motors X and Z receive PWM signal from the same pin, pin #MTR_XZ, while pins
* #MUX_S0 and #MUX_S1 select which one the PWM signal is actually propagated to.
* Because this is implemented by two single switches directly connected to
* #MUX_S0 and #MUX_S1, both motors will be receiving any signal on #MTR_XZ when
* selecting the third MUX channel (@c 1Y3, @c 2Y3). It should be noted though
* that in such a case, no PWM signal should even be present on #MTR_XZ (ie,
* while making a conversion with the ADC or the DS18B20 -- the only 1-wire
* device of this implementation). Also, there is #MTR_nLOCK which could further
* disengage PWM transmission.
*
* Also, see #MUX_S1, #MUX_S0_DDR and #MUX_S0_PORT.
*/
#define MUX_S0          PORTC0

/**
* @brief Data Direction Register of pin MUX @c S1 connects to.
*
* Also, see #MUX_S1_PORT and #MUX_S1.
*/
#define MUX_S1_DDR      DDRC

/**
* @brief The pin port MUX S1 connects to.
*
* Also, see #MUX_S1_DDR and #MUX_S1.
*/
#define MUX_S1_PORT     PORTC

/**
* @brief Pin of #MUX_S1_PORT MUX @c S1 connects to.
*
* This pin along with pin #MUX_S0 are the select bits of the multiplexer.
* For details, refer to #MUX_S0.
*
* Also, see #MUX_S1_DDR and #MUX_S1_PORT.
*/
#define MUX_S1          PORTC1

/**
* @brief Chip-selects the multiplexer.
*
* The multiplexer enables and gives access to the output (step) of the rotary
* encoders on channels 0 through 2. Channel 3 chip-selects the ADC and grants
* access to the 1-wire DQ line through pin #MUX_2Z. Currently, only the DS18B20
* (digital thermometer) is connected on the DQ line.
*/
#define MUX_ENABLE()        MUX_nCS_PORT   &= ~_BV(MUX_nCS)

/**
* @brief Disables the multiplexer, setting all its pins on state Z.
*
* Also, see #MUX_ENABLE().
*/
#define MUX_DISABLE()       MUX_nCS_PORT   |=  _BV(MUX_nCS)

/**
* @brief Data Direction Register of pin Y limit strobe connects to.
*
* Also, see #LMT_nY_PIN and #LMT_nY.
*/
#define LMT_nY_DDR      DDRC

/**
* @brief The pin port Y limit strobe connects to.
*
* Also, see #LMT_nY_DDR and #LMT_nY.
*/
#define LMT_nY_PIN      PINC

/**
* @brief Pin of #LMT_nY_PORT Y limit strobe connects to.
*
* The line is externally pulled high. When one of the Y SPDT limit switches are
* reached, the line is pulled low indicating the incident and remains low until
* the switch is released back to its normally-closed (NC) state. Refer to #BCK_Y
* for details on how this is achieved.
*
* Note this pin will go low when *either* limit switch is reached; to determine
* which one, the direction of motion should be considered.
*
* Also, see #LMT_nY_DDR and #LMT_nY_PIN.
*/
#define LMT_nY           PORTC2

/**
* @brief Data Direction Register of pin XZ limit strobes connect to.
*
* Also, see #LMT_nXZ_PIN and #LMT_nXZ.
*/
#define LMT_nXZ_DDR     DDRC

/**
* @brief The pin port XZ limit strobes connect to.
*
* Also, see #LMT_nXZ_DDR and #LMT_nXZ.
*/
#define LMT_nXZ_PIN     PINC

/**
* @brief Pin of #LMT_nXZ_PORT XZ limit strobes connect to.
*
* The line is externally pulled high. When one of the X or Z SPDT limit switches
* are reached, the line is pulled low indicating the incident and remains low
* until the switch is released back to its normally-closed (NC) state. Refer to
* #BCK_XZ for details on how this is achieved.
*
* Note this pin will go low when *any* limit switch is reached; to determine
* which one, the direction of motion as well as which motor, X or Z, was enabled
* at the time the limit was reached should be considered. Also, note that
* reaching a limit on one axis would automatically disable recognition of limit
* on the other, unless NC state is restored in the former, first. Generally,
* this should not be an issue since motion on both X and Z axes at the same time
* should not take place.
*
* Also, see #LMT_nXZ_DDR and #LMT_nXZ_PIN.
*/
#define LMT_nXZ         PORTC3

/**
* @brief Data Direction Register the Backtrack MOSFET's Gate connects to.
*
* Also, see #BCK_DS_PORT and #BCK_XZ.
*/
#define BCK_XZ_DDR      DDRD

/**
* @brief The pin port the Backtrack MOSFET's Gate connects to.
*
* Also, see #BCK_XZ_DDR and #BCK_XZ.
*/
#define BCK_XZ_PORT     PORTD

/**
* @brief Pin of #BCK_XZ_PORT the Backtrack MOSFET's Gate connects to.
*
* This pin controls a secondary connection to ground of motors X and Z. It
* should normally be pulled low and enabled only when an X or Z limit switch has
* been engaged. This, along with a reverse PWM signal should move the apparatus
* away from the switch just enough to disengage it and permit normal operation
* of the motor.
*
* Also, see #BCK_XZ_DDR and #BCK_XZ_PORT.
*/
#define BCK_XZ          PORTD2

/**
* @brief Data Direction Register the Backtrack MOSFET's Gate connects to.
*
* Also, see #BCK_Y_PORT and #BCK_Y.
*/
#define BCK_Y_DDR       DDRB

/**
* @brief The pin port the Backtrack MOSFET's Gate connects to.
*
* Also, see #BCK_Y_DDR and #BCK_Y.
*/
#define BCK_Y_PORT      PORTB

/**
* @brief Pin of #BCK_Y_PORT the Backtrack MOSFET's Gate connects to.
*
* Similar to #BCK_XZ, only this time, it controls a secondary connection to
* ground for motor Y.
*
* Also, see #BCK_Y_DDR and #BCK_Y_PORT.
*/
#define BCK_Y           PORTB0

/**
* @brief MCU pin which the \f$\overline{RESET}\f$ pin of the network controller
* is connected to.
*
* In order to properly reset the W5100 TCP/IP controller, this pin must be held
* low for at least 2us. *WIZnet p.9.*
*/
#define NET_RST PORTD4

/**
* @brief Number of bytes dedicated to W5100 I/O.
*/
#define NET_BUF_LEN (100)

#endif /* DEFS_H_INCL */
