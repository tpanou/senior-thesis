#include "task.h"
#include "defs.h"
#include "log.h"
#include "util.h"
#include "motor.h"
#include "sensor.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/**
* @brief Convert BCD hours @p h and minutes @m to an interval since midnight.
*
* Note that each interval unit equals 6 minutes.
*/
#define BCD8_TO_INTERVAL(h, m)     (FROM_BCD8(h) * (60/6) + FROM_BCD8(m) / 6)

/**
* @brief Pending samples.
*
* This is set by task_log_sample() and task_log_samples() and managed by
* task_handle_motor().
*/
static uint8_t pending_samples;

/**
* @brief Return value of task_pending().
*/
static uint8_t task_is_pending;

/**
* @brief Time-stamp of the most recent measurement.
*
* This value is calculated at start-up (when task_init() is invoked) and updated
* after each new log record is appended (in task_handle_motor()).
*
* Each unit equals 6 minutes. Values range in @c 0--@c 240 (just like
* Task#interval).
*/
static uint8_t task_recent;

/**
* @brief Time-stamp of the current task operation.
*
* Note that this refers to the *seconds* elapsed since the beginning of this (or
* the previous) hour at which time #task_estimate was set.
*/
static uint16_t task_start;

/**
* @brief The estimated time to complete the current task (in seconds).
*/
static uint16_t task_estimate;

/**
* @brief Current settings of automated samplings.
*/
static Task task;

void task_init() {
    /* Default date limits. */
    BCDDate since   = {.year = 0x00, .mon = 0x01, .date = 0x01,
                       .hour = 0x00, .min = 0x00, .sec  = 0x00};
    BCDDate until   = {.year = 0x99, .mon = 0x12, .date = 0x31,
                       .hour = 0x23, .min = 0x59, .sec  = 0x59};
    LogRecordSet    set;
    LogRecord       rec;
    uint8_t         count;

    /* Identify the time-stamp of the most recent sampling. */
    count   =  log_get_set(&set, &since, &until);
    if(count) {
        /* Fetch the last record. */
        log_skip(&set, count - 1);
        log_get_next(&rec, &set);

        task_recent =  BCD8_TO_INTERVAL(rec.date.hour, rec.date.min);
    } else {
        task_recent =  0;
    }

    motor_set_callback(&task_handle_motor);
}

int8_t task_set(Task* t) {
    if(t->interval > 240) {
        t->interval =  240;
        t->samples  =  255;
        return -1;
    }

    task.interval   =  t->interval;
    task.samples    =  t->samples;
    return 0;
}

void task_get(Task* t) {
    t->interval = task.interval;
    t->samples  = task.samples;
}

void task_log_samples(uint8_t count) {
    if(count) {
        Position pos = {0, 0, 0};

        /* Request the first random position and begin sampling. */
        make_target(&pos.x, &pos.y);

        pending_samples     =  count;
        task_is_pending     =  1;

        motor_set(pos);
    }
}

uint8_t task_log_sample(Position* pos) {

    /* Request the sensor head be submerged. */
    pos->z  =  0;

    /* Schedule a single sampling. It is important this be done before calling
    * motor_set() because, this way, a correct estimate can be calculated by
    * update_motor_eta(). */
    pending_samples =  1;

    /* Stop, if the position is not valid. */
    if(motor_set(*pos)) {
        pending_samples =  0;
        return -1;
    }

    task_is_pending =  1;

    return 0;
}

uint8_t task_pending() {
    return task_is_pending;
}

uint16_t task_get_estimate() {
    BCDDate now;
    uint8_t day;
    uint16_t now_stamp;

    if(!task_estimate) return 0;

    get_date(&now, &day);
    now_stamp      =  FROM_BCD8(now.min) * 60 + FROM_BCD8(now.sec);

    /* If the task was initiated in the previous hour, add an hour (in seconds)
    * to @c now_stamp. Then, subtract then to get the interval. */
    if(now_stamp < task_start) now_stamp += 3600;

    now_stamp  -=  task_start;

    /* Subtract the interval from the estimate. */
    if(now_stamp > task_estimate) return 0;
    return task_estimate - now_stamp;
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

static uint16_t update_motor_eta(Position* new) {
    uint16_t time;
    Position cur;
    int16_t one;
    int16_t two;

    if(!motor_get(&cur)) {
        /* Preserve the offset on either X or Y, whichever is greater (since the
        * common part of the two is run in parallel). */
        one     =  abs(cur.x - new->x);
        two     =  abs(cur.y - new->y);
        if(one < two) one = two;

        /* Add a hypothetical delay to reach each target that is generated by
        * make_target(). */
        if(pending_samples > 1) one += TASK_MEAN_TIME * pending_samples;

        /* Increment overall offset by two times the dimension of Z (ie, to go
        * down and back up) plus the time the head remains submerged for as many
        * times as there are pending tasks. Note that it takes 1s to translate
        * by 1. */
        two     =  (GRID_Z_LEN * 2 + TASK_SAMPLE_TIME) * pending_samples;

        /* If @c z is at @c 0, then the head is already lowered and the sampling
        * has been performed; the latter being certain because this function is
        * called once it is needed to *raise* the head away from @c 0 (and only
        * then is cur.z equal to @c 0). That last sample is, also, already
        * removed from @c pending_samples. Still, the time it takes to fully
        * retract head for that sampling should be taken into account. */
        if(!cur.z) two +=  GRID_Z_LEN;

        time    =  one + two;
    } else {
        time    =  0;
    }
    return time;
}

static void task_handle_motor(Position pos, uint8_t evt) {

    switch(evt) {
        case MTR_EVT_BUSY: {
            BCDDate now;
            uint8_t day;
            get_date(&now, &day);

            task_is_pending =  1;
            task_estimate   =  task_estimate_time(&pos);
            task_start      =  FROM_BCD8(now.min) * 60 + FROM_BCD8(now.sec);

        } break;
        case MTR_EVT_OK:
            if(pending_samples) {
                /* Take a sample, if the sensor head is submerged. */
                if(pos.z == 0) {
                    LogRecord rec;
                    Position max;
                    uint8_t day;
                    uint16_t t;

                    _delay_ms(TASK_SAMPLE_TIME * 1000);
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

                    /* Update #task_recent now because a reset may be issued
                    * before any of the remaining tasks are completed. */
                    task_recent =  BCD8_TO_INTERVAL(rec.date.hour,
                                                    rec.date.min);

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
            } else {
                task_is_pending =  0;
                task_estimate   =  0;
            }
        break;
    }
}

/**
* @brief Periodic automated sampling ISR.
*
* This is the Watchdog Timer ISR, responsible for waking the CPU every 8s (the
* maximum interval for this MCU) to check whether there are tasks that should be
* initiated automatically. They are only initiated granted the following:
*   - No other task is currently in progress.
*   - Task interval and samples have been specified (each, other than @c 0). See
*       task_set().
*   - The RTC is running.
*   - The elapsed quanta since the most recently performed task are equal or
*       greater the the specified interval (as set with task_set()).
*/
ISR(WDT_vect) {
    BCDDate now;
    uint8_t day;
    uint16_t now_stamp;
    uint16_t elapsed;   /* Allow for a two-day interval. */

    /* Do not proceed, if a task is in progress or there are no automation
    * settings. */
    if(task_is_pending || !task.interval || !task.samples) return;

    /* Also, do not proceed if the (RTC) clock is not running. */
    get_date(&now, &day);

    if(bit_is_set(now.sec, RTC_CH)) return;

    /* Calculate the interval between the most recent sampling and the current
    * time-stamp. */
    now_stamp   =  BCD8_TO_INTERVAL(now.hour, now.min);

    /* This is for when #task_recent is before midnight and @c now_stamp is
    * after. */
    if(now_stamp < task_recent) now_stamp  +=  240;

    elapsed =  now_stamp - task_recent;


    if(elapsed >= task.interval) {
        task_log_samples(task.samples);
    }
}
