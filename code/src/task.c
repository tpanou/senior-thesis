#include "task.h"

#include "motor.h"
#include "sensor.h"

#include <avr/io.h>
#include <util/delay.h>

static uint8_t pending_samples;

void task_init() {

    motor_set_callback(&task_handle_motor);
}

static void task_handle_motor(Position pos, uint8_t evt) {
    printf("Motor handler: %d @ [%d,%d,%d]\n", evt, pos.x, pos.y, pos.z);

    if(pending_samples) {
        switch(evt) {
            case MTR_EVT_OK:
                /* Take a sample, if the sensor head is submerged. */
                if(pos.z == 0) {
                    _delay_ms(3000);
                    uint16_t t = sens_read_t();

                    /* TODO: Log the result. */
                    printf("Sample: %d.%cC\n", t>>4, '0' + 5*((t&0xF)>>3));

                    /* Since the measurement is complete, the head should be
                    * retracted. */
                    pos.z = GRID_Z_LEN;

                    /* Check whether there are pending samples to take and
                    * request a pair of X-Y coordinates. Note that retracting
                    * the sensor head (ie, increasing translation along axis Z)
                    * takes precedence over motion along axes X and Y. Thus, it
                    * is not necessary to set Z and X-Y motor coordinates
                    * separately (see motor_update()). */
                    if(--pending_samples) {
                        /* TODO: Request a new random X-Y position for the
                        * head. */
                    }


                /* The head has reached a new position and there are pending
                * samples. Request the head be submerged to take a sample later
                * on. */
                } else {
                    pos.z = 0;
                }

                motor_set(pos);
            break;

        }
    }
}
