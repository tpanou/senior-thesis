#include "task.h"
#include "log.h"
#include "util.h"
#include "motor.h"
#include "sensor.h"

#include <avr/io.h>
#include <util/delay.h>

static uint8_t pending_samples;

void task_init() {

    motor_set_callback(&task_handle_motor);
}

void task_log_samples(uint8_t count) {
    if(count) {
        Position pos = {0, 0, 0};

        /* Request the first random position and begin sampling. */
        make_target(&pos.x, &pos.y);

        motor_set(pos);
        pending_samples     =  count;
    }
}

uint8_t task_log_sample(Position* pos) {

    /* Request the sensor head be submerged. */
    pos->z  =  0;

    /* Stop, if the position is not valid. */
    if(motor_set(*pos)) return -1;

    pending_samples =  1;       /* Schedule a single sampling. */
    return 0;
}

static void make_target(uint8_t* x, uint8_t* y) {
    BCDDate dt;
    uint8_t day;
    Position max;
    uint8_t seed;

    get_date(&dt, &day);
    motor_get_max(&max);
    seed    =  dt.sec + dt.min + *x + *y;
    *x      =  seed        % max.x;
    *y      = (seed + day) % max.y;
}

static void task_handle_motor(Position pos, uint8_t evt) {

    if(pending_samples) {
        switch(evt) {
            case MTR_EVT_OK:
                /* Take a sample, if the sensor head is submerged. */
                if(pos.z == 0) {
                    LogRecord rec;
                    Position max;
                    uint8_t day;
                    uint16_t t;

                    _delay_ms(5000);
                    t  =  sens_read_t();

                    get_date(&rec.date, &day);
                    /* Prepare record. */
                    rec.t       =  t >> 3;
                    rec.x       =  pos.x;
                    rec.y       =  pos.y;
                    rec.rh      =  0xFF;
                    rec.ph      =  0xFF;

                    /* Log the result. */
                    log_append(&rec);

                    printf("Sample: %d.%cC\n", t>>4, '0' + 5*((t&0xF)>>3));

                    /* Since the measurement is complete, the head should be
                    * retracted. */
                    motor_get_max(&max);
                    pos.z = max.z - 1;

                    /* Check whether there are pending samples to take and
                    * request a pair of X-Y coordinates. Note that retracting
                    * the sensor head (ie, increasing translation along axis Z)
                    * takes precedence over motion along axes X and Y. Thus, it
                    * is not necessary to set Z and X-Y motor coordinates
                    * separately (see motor_update()). */
                    if(--pending_samples) {

                        /* Request a new random X-Y position for the head. */
                        make_target(&pos.x, &pos.y);
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
