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
    /** @brief X-coordinate (surface abscissa). */
    uint8_t x;

    /** @brief Y-coordinate (surface ordinate). */
    uint8_t y;

    /** @brief Z-coordinate (head elevation). */
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
* @brief A date in BCD format.
*/
typedef struct {
    /** @brief Year: @f$0x00-0x99@f$. */
    uint8_t year;

    /** @brief Month: @f$0x01-0x12@f$. */
    uint8_t mon;

    /** @brief Date: @f$0x01-0x31@f$. */
    uint8_t date;

    /** @brief Hours: @f$0x00-0x23@f$. */
    uint8_t hour;

    /** @brief Minutes: @f$0x00-0x59@f$. */
    uint8_t min;

    /** @brief Seconds: @f$0x00-0x59@f$. */
    uint8_t sec;
} BCDDate;

/**
* @brief Default device IP address.
*/
#define FACTORY_IADDR       192, 168, 1, 73

/**
* @brief Default device gateway address.
*/
#define FACTORY_GATEWAY     192, 168, 1, 1

/**
* @brief Default device subnet mask.
*/
#define FACTORY_SUBNET      255, 255, 255, 0

/**
* @brief Default device hardware address.
*/
#define FACTORY_HADDR       0xBE, 0xEB, 0xEE, 0xBE, 0xEB, 0xEE

/**
* @brief User-data RTC memory address.
*
* The RTC (DS1307) provides 56 Bytes of user-defined battery-backed RAM. This is
* the address of the first available byte.
*
* Also, see sys_set().
*/
#define RTC_BASE        0x08

/**
* @brief Backup memory address of IP address.
*/
#define SYS_IADDR      (RTC_BASE        + 0x00)

/**
* @brief Backup memory address of Gateway address.
*/
#define SYS_GATEWAY    (SYS_IADDR       + 0x04)

/**
* @brief Backup memory address of Subnet mask.
*/
#define SYS_SUBNET     (SYS_GATEWAY     + 0x04)

/**
* @brief Backup memory address of Hardware address.
*/
#define SYS_HADDR      (SYS_SUBNET      + 0x04)

/**
* @brief Backup memory address of operating range.
*/
#define SYS_MTR_MAX    (SYS_HADDR       + 0x06)

/**
* @brief Backup memory address of axis X maximum value.
*/
#define SYS_MTR_MAX_X  (SYS_HADDR       + 0x06)

/**
* @brief Backup memory address of axis Y maximum value.
*/
#define SYS_MTR_MAX_Y  (SYS_MTR_MAX_X   + 0x01)

/**
* @brief Backup memory address of axis Z maximum value.
*/
#define SYS_MTR_MAX_Z  (SYS_MTR_MAX_Y   + 0x01)

/**
* @brief Set device configuration settings.
*
* The purpose of this function is two-fold; it provides an easy way to pass
* settings to various modules without the need to use their respective API
* (which may be too verbose at times); it stores a copy of those settings to a
* backup memory.
*
* For a list of available settings, see SYS_* macros. It should be noted that
* this is the *preferred* way of updating the settings for which such a macro is
* provided.
*
* The backup memory allows settings persistence even if the device is completely
* disconnected from the mains. Currently, the backup memory is assigned to the
* battery-backed RTC RAM which allows resetting to factory default settings, if
* the battery is temporarily removed.
*
* @param[in] setting The system setting to update.
* @param[in] value The new value of @p setting.
* @returns @c 0 on success; @c -1, otherwise. Failure designates the setting
*   could not be written due to a communication error with the backup memory
*   and *not* due to an erroneous value; if the underlying module rejects the
*   supplied value, no indication is given by this function. The callee should
*   only supply valid values.
*/
int8_t sys_set(uint8_t setting, void* value);

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
* @brief Socket of W5100 that corresponds to the HTTP server.
*/
#define HTTP_SOCKET     0

/**
* @brief Available output buffer size in the network module (chip).
*
* This is just a convenience. The actual setting is done in code (main()).
*/
#define HTTP_BUF_SIZE   2048
/**
* @brief Size of the buffer used in parsing query parameters.
*
* This much space is allocated upon calling srvr_call() and should be large
* enough to contain all the acceptable parameters (token + value) for any given
* resource.
*
* It is safe to assume that the server will use this amount of memory only once
* for a single HTTP request.
*
* It is possible to run the server without allocating buffer for such a task
* (simply by setting this to @c 0).
*/
#define QUERY_BUF_LEN       105

/**
* @brief The maximum number of acceptable parameters for any one resource.
*
* This should be equal to the maximum number of query string parameters that are
* expected by any *one* of the available resource handlers.
*/
#define QUERY_PARAM_LEN     6

/**
* @brief The amount of total records to store in the EEPROM Log.
*
* The available host EEPROM is 1KB, whereas each log record requires 11 bytes
* (see #LogRecord). It is decided to keep tract of the last 90 measurements, so
* the circular buffer requires a total of 990 bytes. From the remainder bytes,
* two more are used; one for #log_index and one for #log_count.
*/
#define LOG_LEN             90

/**
* @brief The EEPROM address to start storing log records.
*
* The circular buffer is stored at the last 990 bytes of the EEPROM. Thus, its
* lowest byte is at this address.
*/
#define LOG_BASE_ADDR       34

/**
* @brief Value of @c SPSR. This should only affect bit @c SPI2X.
*
* Currently, \f$ clk_{IO} \f$ is 4MHz. The 25LC1024 supports transfer rates up
* to 20MHz. The closest that can be attained with the current configuration is
* 2MHz. This requires setting bit @c SPI2X but none of the @c SPR1:0 of @c SPCR.
*/
#define FLS_SPSR        _BV(SPI2X)

/**
* @brief Value of @c SPCR. This should only affect bits @c SPCR1:0.
*
* Currently, \f$ clk_{IO} \f$ is 4MHz. The 25LC1024 supports transfer rates up
* to 20MHz. The closest that can be attained with the current configuration is
* 2MHz. This requires setting bit @c SPI2X but none of the @c SPR1:0 of @c SPCR.
*/
#define FLS_SPCR        0

/**
* @brief First flash page address of index.
*
* The file starts at this page and extends for #FILE_SIZE_INDEX bytes.
*/
#define FILE_PAGE_INDEX         32*0

/**
* @brief First flash page address of style.css.
*
* The file starts at this page and extends for #FILE_SIZE_STYLE_CSS bytes.
*/
#define FILE_PAGE_STYLE_CSS     32*1

/**
* @brief First flash page address of logo.png.
*
* The file starts at this page and extends for #FILE_SIZE_LOGO_PNG bytes.
*/
#define FILE_PAGE_LOGO_PNG     (32*1 + 16)

/**
* @brief First flash page address of client.js.
*
* The file starts at this page and extends for #FILE_SIZE_CLIENT_JS bytes.
*/
#define FILE_PAGE_CLIENT_JS     32*2

/**
* @brief Size of the index file.
*/
#define FILE_SIZE_INDEX         1818

/**
* @brief Size of the style.css file.
*/
#define FILE_SIZE_STYLE_CSS     1161

/**
* @brief Size of the logo.png file.
*/
#define FILE_SIZE_LOGO_PNG      4288

/**
* @brief Size of the client.js file.
*/
#define FILE_SIZE_CLIENT_JS     6306

/**
* @brief Pulls Flash @c nCS low.
*/
#define FLS_ENABLE()    MUX_S1_PORT |=  _BV(MUX_S1);\
                        MUX_S0_PORT |=  _BV(MUX_S0);\
                        MUX_ENABLE()

/**
* @brief Pulls Flash @c nCS high.
*/
#define FLS_DISABLE()   MUX_DISABLE()

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
* @brief The pin port the AutoLock MOSFET's Gate connects to.
*
* Also, see #MTR_nLOCK_PORT and #MTR_nLOCK.
*/
#define MTR_nLOCK_PIN   PIND

/**
* @brief Pin of #MTR_nLOCK_PORT the AutoLock MOSFET's Gate connects to.
*
* This pin enables and disables the Lock and, in order for the AutoLock
* mechanism to operate, it must coincide with pin @c OC0A. It is active low.
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
* @brief Pin of #MUX_2Z_PIN MUX @c 2Z connects to.
*
* This pin gives the output of the rotary encoders (step) of motors Y, Z and X
* on pins @c 2Y0, @c 2Y1 and @c 2Y2 of the multiplexer, respectively. Pin @c 2Y3
* gives access to the 1-wire DQ line.
*
* Also, see #MUX_2Z_PIN and #MUX_2Z_PIN.
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
* @brief Pin Change interrupt vector to enable on a limit switch change.
*
* This particular value is used to enable pin change interrupts on pins
* @c PCINT\[14:8\] (*Atmel pp.74-75*). For simplicity's sake, it is assumed that
* limits on all axes use pins of the same *port*.
*
* Also, see #LMT_PCMSK and LMT_PCMSK_VAL.
*/
#define LMT_PCIE        PCIE1

/**
* @brief PCINT mask register that corresponds to the port defined by #LMT_PCIE.
*
* Also, see #LMT_PCIE and LMT_PCMSK_VAL.
*/
#define LMT_PCMSK       PCMSK1

/**
* @brief Value of #LMT_PCMSK that enables interrupts on the pins of the limit
* switches.
*
* The ones of interest are @c PCINT11 (@c PC3) and @c PCINT10 (@c PC2).
*
* Also, see #LMT_PCIE and #LMT_PCMSK.
*/
#define LMT_PCMSK_VAL   _BV(PCINT11) | _BV(PCINT10);

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
* @brief Pin of #LMT_nY_PIN Y limit strobe connects to.
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
* @brief Pin of #LMT_nXZ_PIN XZ limit strobes connect to.
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
* Also, see #BCK_XZ_PORT and #BCK_XZ.
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
* @brief Pin of #W1_DQ_PORT where 1-wire DQ line connects to.
*
* This pin is used for communicating via the 1-wite protocol.
*
* Also, see #W1_DQ_PORT, #W1_DQ_DDR and #W1_DQ_PIN.
*/
#define W1_DQ           PORTD4

/**
* @brief The pin port 1-wire DQ connects to.
*
* Also, see #W1_DQ, #W1_DQ_DDR and #W1_DQ_PIN.
*/
#define W1_DQ_PORT      PORTD

/**
* @brief Data Direction Register of pin 1-wire DQ connects to.
*
* Also, see #W1_DQ_PORT, #W1_DQ_DDR and #W1_DQ_PIN.
*/
#define W1_DQ_DDR       DDRD

/**
* @brief The pin port 1-wire DQ connects to.
*
* Also, see #W1_DQ, #W1_DQ_PORT and #W1_DQ_DDR.
*/
#define W1_DQ_PIN       PIND

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

/**
* @brief Value of @c SPCR. This should only affect bits @c SPCR1:0.
*
* Currently, \f$ clk_{IO} \f$ is 4MHz. The W5100 supports transfer rates up
* to 14MHz. The closest that can be attained with the current configuration is
* 2MHz. This requires setting bit @c SPI2X but none of the @c SPR1:0 of @c SPCR.
*/
#define NET_SPCR        0

/**
* @brief Value of @c SPCR. This should only affect bits @c SPCR1:0.
*
* Currently, \f$ clk_{IO} \f$ is 4MHz. The W5100 supports transfer rates up
* to 14MHz. The closest that can be attained with the current configuration is
* 2MHz. This requires setting bit @c SPI2X but none of the @c SPR1:0 of @c SPCR.
*/
#define NET_SPSR        _BV(SPI2X)

/**
* @brief Data Direction Register the W5100 nCS pin connects to.
*
* Also, see #NET_nCS_PORT and #NET_nCS.
*/
#define NET_nCS_DDR     DDRD

/**
* @brief The pin port the W5100 nCS pin connects to.
*
* Also, see #NET_nCS_DDR and #NET_nCS.
*/
#define NET_nCS_PORT    PORTD

/**
* @brief Pin of #NET_nCS_PORT the W5100 nCS pin connects to.
*
* Also, see #NET_nCS_DDR and #NET_nCS_PORT.
*/
#define NET_nCS         PORTD7

/**
* @brief Chip-select the W5100.
*/
#define NET_ENABLE()   (NET_nCS_PORT  &= ~_BV(NET_nCS))

/**
* @brief Chip-deselect the W5100.
*/
#define NET_DISABLE()  (NET_nCS_PORT  |=  _BV(NET_nCS))

#endif /* DEFS_H_INCL */
