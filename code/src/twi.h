/**
* @file
*/

#ifndef TWI_H_INCL
#define TWI_H_INCL

#include "defs.h"

/**
* @brief Busy-waits until @c TWINT of @c TWCR register is set. Status code is
* read with #TWI_STATUS().
*/
#define TWI_WAIT()      while(!(TWCR & _BV(TWINT)));

/**
* @brief Clears @c TWINT and sets forth the next action.
*/
#define TWI_DO()        TWCR  =  _BV(TWINT) | _BV(TWEN);

/**
* @brief Clears @c TWINT and sets forth the next action, enabling transmission
* of @c ACK upon completion.
*
* This should be preferred over #TWI_DO() when, in Master Receiver mode, it is
* required to acknowledge the received byte.
*/
#define TWI_DO_ACK()    TWCR  =  _BV(TWINT) | _BV(TWEA) | _BV(TWEN);

/**
* @brief A #TWI_DO() followed by an #TWI_WAIT().
*/
#define TWI_DO_WAIT()   TWI_DO();\
                        TWI_WAIT()

/**
* @brief Gains possession of the bus.
*
* Busy-waits until the operation of performed. #TWI_STATUS() should used to
* identify the outcome.
*/
#define TWI_START()     TWCR  =  _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);\
                        TWI_WAIT()

/**
* @brief Releases the bus.
*
* It sets the appropriate bits and instantly returns.
*/
#define TWI_STOP()      TWCR  =  _BV(TWINT) | _BV(TWSTO) | _BV(TWEN)

/**
* @brief Reads a byte from the bus in @p x.
*
* The LSB of @p x is set to indicate a @c READ operation.
*
* @param[in] x Data byte read from the bus.
*/
#define TWI_SLA_R(x)    TWDR  = x | 1;\
                        TWI_DO_WAIT()

/** @brief Sends data contained in @p x over the TWI bus.
*
* The LSB of @p x is cleared to indicate a @c WRITE operation.
*
* @param[in] x Data byte to send over the bus.
*/
#define TWI_SLA_W(x)    TWDR  = x & 0xFE;\
                        TWI_DO_WAIT()

/**
* @brief Return the TWI bus status code.
*
* The last two bits are the prescaler bits; the bit before them is reserved and
* always reads zero (*Atmel p236*). Regardless, all three (last) bits are masked
* when reading the status code.
*/
#define TWI_STATUS()   (TWSR & 0xF8)

/**
* @brief TWI bus status code; @c START condition has been transmitted.
* *Atmel p222,p225*
*
* In Master Transmitter (MT) mode, other possible status codes include:
* #TWI_RSTART, #TWI_ARB_LOST, #TWI_SLA_W_ACK, #TWI_SLA_W_NACK, #TWI_DATA_W_ACK
* and #TWI_DATA_W_NACK.
*
* In Master Receiver (MT) mode, other possible status codes include:
* #TWI_RSTART, #TWI_ARB_LOST, #TWI_SLA_R_ACK, #TWI_SLA_R_NACK, #TWI_DATA_R_ACK
* and #TWI_DATA_R_NACK.
*/
#define TWI_SSTART      (0x08)

/**
* @brief TWI bus status code; repeated START condition has been transmitted.
* *Atmel p222,p225*
*
* Used both in Master Transmitter (MT) and Master Receiver (MR) mode. For a full
* list of codes per mode, see #TWI_SSTART.
*/
#define TWI_RSTART      (0x10)

/**
* @brief TWI bus status code; arbitration lost in SLA+R mode or NOT ACK bit
* (MR); arbitration lost in SLA+W or data bytes (MT). *Atmel p222,p225*
*
* Used both in Master Transmitter (MT) and Master Receiver (MR) mode. For a full
* list of codes per mode, see #TWI_SSTART.
*/
#define TWI_ARB_LOST    (0x38)

/**
* @brief TWI bus status code; SLA+W has been transmitted and ACK has
* been received. *Atmel p222*
*
* Other status codes in Master Transmitter (MT) mode: #TWI_SSTART, #TWI_RSTART,
* #TWI_ARB_LOST, #TWI_SLA_W_NACK, #TWI_DATA_W_ACK and #TWI_DATA_W_NACK.
*/
#define TWI_SLA_W_ACK   (0x18)

/**
* @brief TWI bus status code; SLA+W has been transmitted and NOT ACK has
* been received. *Atmel p222*
*
* Other status codes in Master Transmitter (MT) mode: #TWI_SSTART, #TWI_RSTART,
* #TWI_ARB_LOST, #TWI_SLA_W_ACK, #TWI_DATA_W_ACK and #TWI_DATA_W_NACK.
*/
#define TWI_SLA_W_NACK  (0x20)

/**
* @brief TWI bus status code; data byte has been transmitted, ACK has been
* received. *Atmel p222*
*
* Other status codes in Master Transmitter (MT) mode: #TWI_SSTART, #TWI_RSTART,
* #TWI_ARB_LOST, #TWI_SLA_W_ACK, #TWI_SLA_W_NACK and #TWI_DATA_W_NACK.
*/
#define TWI_DATA_W_ACK  (0x28)

/**
* @brief TWI bus status code; data byte has been transmitted, NOT ACK has been
* received. *Atmel p222*
*
* Other status codes in Master Transmitter (MT) mode: #TWI_SSTART, #TWI_RSTART,
* #TWI_ARB_LOST, #TWI_SLA_W_ACK, #TWI_SLA_W_NACK and #TWI_DATA_W_ACK.
*/
#define TWI_DATA_W_NACK (0x30)

/**
* @brief TWI bus status code; SLA+R has been transmitted, ACK has been received.
* *Atmel p225*
*
* Other status codes in Master Receiver (MR) mode: #TWI_SSTART, #TWI_RSTART,
* #TWI_ARB_LOST, #TWI_SLA_R_NACK, #TWI_DATA_R_ACK and #TWI_DATA_R_NACK.
*/
#define TWI_SLA_R_ACK   (0x40)

/**
* @brief TWI bus status code; SLA+R has been transmitted, NOT ACK has been
* received. *Atmel p225*
*
* Other status codes in Master Receiver (MR) mode: #TWI_SSTART, #TWI_RSTART,
* #TWI_ARB_LOST, #TWI_SLA_R_ACK, #TWI_DATA_R_ACK and #TWI_DATA_R_NACK.
*/
#define TWI_SLA_R_NACK  (0x48)

/**
* @brief TWI bus status code; data byte has been received, ACK has been
* returned. *Atmel p225*
*
* Other status codes in Master Receiver (MR) mode: #TWI_SSTART, #TWI_RSTART,
* #TWI_ARB_LOST, #TWI_SLA_R_ACK, #TWI_SLA_R_NACK and #TWI_DATA_R_NACK.
*/
#define TWI_DATA_R_ACK  (0x50)

/**
* @brief TWI bus status code; data byte has been received, NOT ACK has been
* returned. *Atmel p225*
*
* Other status codes in Master Receiver (MR) mode: #TWI_SSTART, #TWI_RSTART,
* #TWI_ARB_LOST, #TWI_SLA_R_ACK, #TWI_SLA_R_NACK and #TWI_DATA_R_ACK.
*/
#define TWI_DATA_R_NACK (0x58)

/**
* @brief Executes @c x, returning @c -1 upon failure.
*
* Upon executing @p x, #TWI_STATUS() is used to determine the outcome of the
* operation. Should the status code returned by it be unequal to @c s, the bus
* is released (#TWI_STOP()) and @c -1 is returned. Note that this is a macro
* and, so, the returning function is the 'callee'.
*
* @parm[in] x Arbitrary code to execute. Typically, one of the TWI convenience
*   macros.
* @param[in] s Status code that is expected upon successful completion of @p x.
*/
#define TWI_ATTEMPT(x, s)       x;\
                                if(TWI_STATUS() != s) {\
                                    TWI_STOP();\
                                    return -1;\
                                }

/**
* @brief Initializes the TWI registers.
*
* This macro merely sets the TWI bit-rate value and the prescaler bits.
*/
#define TWI_INIT()              TWBR    =  TWBR_VALUE;\
                                TWSR   |=  TWI_PRESCALER;

#endif /* TWI_H_INCL */
