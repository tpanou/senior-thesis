/*
* This module (resource) is divided into two parts; resource.c, containing
* common base functions, and this file, containing the definition of each
* resource handler. It is assumed that #parser and #serialiser have already been
* defined as part of resource.c and are currently available.
*/

#include "http_server.h"
#include "defs.h"
#include "param.h"
#include "util.h"
#include "motor.h"
#include "rtc.h"
#include "log.h"
#include "w5100/w5100.h"

#include <avr/pgmspace.h>
#include <inttypes.h>

/**
* @ingroup resource
* @brief Index of parameter "date" (in resource /configuration).
*/
#define PRM_SRVR_DATE       0

/**
* @ingroup resource
* @brief Index of parameter "day" (in resource /configuration).
*/
#define PRM_SRVR_DAY        1

/**
* @ingroup resource
* @brief Index of parameter "gateway" (in resource /configuration).
*/
#define PRM_SRVR_GATEWAY    2

/**
* @ingroup resource
* @brief Index of parameter "iaddr" (in resource /configuration).
*/
#define PRM_SRVR_IADDR      3

/**
* @ingroup resource
* @brief Index of parameter "subnet" (in resource /configuration).
*/
#define PRM_SRVR_SUBNET     4

/**
* @ingroup resource
* @brief Index of parameter "x" (in resource /configuration).
*/
#define PRM_SRVR_X          5

/**
* @ingroup resource
* @brief Index of parameter "y" (in resource /configuration).
*/
#define PRM_SRVR_Y          6

/**
* @ingroup resource
* @brief Index of parameter "z" (in resource /configuration).
*/
#define PRM_SRVR_Z          7

/**
* @ingroup resource
* @brief Size of a date string (ISO8601 format) (inclusive of null-byte).
*/
#define PRM_DATE_LEN        25

/**
* @ingroup resource
* @brief Size of any IPv4 address string (inclusive of null-byte).
*/
#define PRM_INET_LEN        16

/**
* @ingroup resource
* @brief Size of any temperature reading (inclusive of null-byte).
*/
#define PRM_TEMP_LEN        6

/**
* @ingroup resource
* @brief Index of parameter "date-since" (in resource /measurement).
*/
#define PRM_MSR_DATE_SINCE 0

/**
* @ingroup resource
* @brief Index of parameter "date-until" (in resource /measurement).
*/
#define PRM_MSR_DATE_UNTIL 1

/**
* @ingroup resource
* @brief Index of parameter "page-index" (in resource /measurement).
*/
#define PRM_MSR_PAGE_INDEX 2

/**
* @ingroup resource
* @brief Index of parameter "page-size" (in resource /measurement).
*/
#define PRM_MSR_PAGE_SIZE  3

#ifndef BV
/**
* @brief Set a single bit.
*
* @param[in] x The bit to set.
*/
#define BV(x)              (1 << x)
#endif

/*
* @brief Token: x
*
* This is used by rsrc_handle_coordinates() to read and return device
* coordinates.
*/
static uint8_t prm_x[] PROGMEM = "x";

/* Doxygen does not handle attributes (like PROGMEM) very well. */
/*
* @brief Token: y
*
* This is used by rsrc_handle_coordinates() to read and return device
* coordinates.
*/
static uint8_t prm_y[] PROGMEM = "y";

/*
* @brief Token: z
*
* This is used by rsrc_handle_coordinates() to read and return device
* coordinates.
*/
static uint8_t prm_z[] PROGMEM = "z";

/*
* @brief Token: date
*
* This is used by rsrc_handle_configuration() to parse a new or return the
* current server date and time.
*/
static uint8_t prm_date[] PROGMEM = "date";

/*
* @brief Token: day
*
* This is used by rsrc_handle_configuration() to parse a new or return the
* current server day (eg, @c 1 for Sunday).
*/
static uint8_t prm_day[] PROGMEM = "day";

/*
* @brief Token: gateway
*
* This is used by rsrc_handle_configuration() to parse a new or return the
* current server default gateway.
*/
static uint8_t prm_gateway[] PROGMEM = "gateway";

/*
* @brief Token: ip
*
* This is used by rsrc_handle_configuration() to parse a new or return the
* current server IP address.
*/
static uint8_t prm_iaddr[] PROGMEM = "iaddr";

/*
* @brief Token: subnet
*
* This is used by rsrc_handle_configuration() to parse a new or return the
* current server subnet mask.
*/
static uint8_t prm_subnet[] PROGMEM = "subnet";

/*
* @brief Token: page-index
*
* It specifies the page of search results returned by
* rsrc_handle_measurement().
*/
static uint8_t prm_page_index[] PROGMEM = "page-index";

/*
* @brief Token: page-size
*
* It specifies the number of search results returned by
* rsrc_handle_measurement().
*/
static uint8_t prm_page_size[] PROGMEM = "page-size";

/*
* @brief Token: date-since
*
* It specifies the starting date of search results returned by
* rsrc_handle_measurement().
*/
static uint8_t prm_date_since[] PROGMEM = "date-since";

/*
* @brief Token: date-until
*
* It specifies the end date of search results returned by
* rsrc_handle_measurement().
*/
static uint8_t prm_date_until[] PROGMEM = "date-until";

/*
* @brief Token: log
*
* It contains the records returned by rsrc_handle_measurement().
*/
static uint8_t prm_log[] PROGMEM = "log";

/*
* @brief Token: total
*
* It specifies the total amount of available records to be returned by
* rsrc_handle_measurement().
*/
static uint8_t prm_total[] PROGMEM = "total";

/*
* @brief Token: ph
*
* The pH value of a record, as returned by rsrc_handle_measurement().
*/
static uint8_t prm_ph[] PROGMEM = "ph";

/*
* @brief Token: rh
*
* The Relative Humidity value of a record, as returned by
* rsrc_handle_measurement().
*/
static uint8_t prm_rh[] PROGMEM = "rh";

/*
* @brief Token: t
*
* The Temperature value of a record, as returned by rsrc_handle_measurement().
*/
static uint8_t prm_t[] PROGMEM = "t";

void rsrc_handle_server(HTTPRequest* req) {
}

void rsrc_handle_root(HTTPRequest* req) {
}

void rsrc_handle_client_js(HTTPRequest* req) {
}

/**
* @ingroup resource
* @brief Manage device configuration.
*
* Method GET:
* Returns the current configuration of the device. The response status code is
* always 200. Format: @verbatim {
    "date"      : string,       // Format ISO8601: YYYY-MM-DDTHH:mm:ss[.sss][Z]
    "day"       : number,       // Day-of-the-week; 1 denotes Sunday
    "gateway"   : string,       // Server default gateway (dot-notation)
    "ip"        : string,       // Server IP address
    "subnet"    : string,       // Server subnet mask (xxx.xxx.xxx.xxx)
    "x"         : number,       // Custom maximum X dimension
    "y"         : number,       // Custom maximum Y dimension
    "z"         : number        // Custom maximum Z dimension
} @endverbatim
* Additional notes:
*   - Fraction of second (in @c date) is always returned as @c 000 and never
*       parsed. Time-zone is UTC (@c Z).
*   - `dot-notation' is: xxx.xxx.xxx.xxx
*   - Custom maximum dimensions allow operating the device in a subset of the
*       physical device-space (#GRID_X_LEN, #GRID_Y_LEN, #GRID_Z_LEN).
*
* Method PUT:
* Updates the configuration.
* The message body should include any of the above specified keys, the order of
* which is irrelevant. Only the last instance of each key is preserved. Returns:
*   - 200 OK; the device configuration has been updated. If new device limits
*       have been specified (@c x, @c y and/or @c z), the device head will first
*       be reset.
*   - 400 Bad request; an invalid parameter and/or value has been specified.
*       Currently, no details are given for the exact reasons; the absolute
*       physical limits (for @c x, @c y and @c z) are always returned,
*       regardless.
* Additional notes:
*   - If any supplied parameter fails validation, the whole request is dropped
*       (400 - Bad Request).
*   - Setting @c date requires setting @c day as well, and vice versa.
*       Otherwise, the parameter is ignored.
*   - Only full dates are acceptable (with or without fraction which is
*       ignored).
*/
void rsrc_handle_configuration(HTTPRequest* req) {
    uint8_t  token_buf[38];     /* Key tokens. */
    uint8_t* tokens[8];         /* Pointers to each token in @c token_buf. */

    /* Parameter value buffers. */
    uint8_t  day;
    uint8_t  s_date     [PRM_DATE_LEN];
    uint8_t  s_gateway  [PRM_INET_LEN];
    uint8_t  s_iaddr    [PRM_INET_LEN];
    uint8_t  s_subnet   [PRM_INET_LEN];

   /* Load current maximum position. */
    Position  max;
    motor_get_max(&max);

    /* Set-up acceptable parser parameters. */
    ParamValue params[]     =  {PARAM_STRING(s_date, PRM_DATE_LEN),
                                PARAM_UINT8(day),
                                PARAM_STRING(s_gateway, PRM_INET_LEN),
                                PARAM_STRING(s_iaddr, PRM_INET_LEN),
                                PARAM_STRING(s_subnet, PRM_INET_LEN),
                                PARAM_UINT8(max.x),
                                PARAM_UINT8(max.y),
                                PARAM_UINT8(max.z)};

    /* Load tokens into main memory. */
    pgm_read_str_array(tokens, token_buf, prm_date,
                                          prm_day,
                                          prm_gateway,
                                          prm_iaddr,
                                          prm_subnet,
                                          prm_x,
                                          prm_y,
                                          prm_z, NULL);

    /* --- INITIALISATION end -- */

    uint8_t  status;        /* Status of response. */
    uint16_t size = 130;    /* Response size without Inet address values. */
    uint8_t iaddr[4];       /* Numerical IP address. */
    uint8_t subnet[4];      /* Numerical subnet mask. */
    uint8_t gateway[4];     /* Numerical default gateway address. */
    BCDDate  dt;             /* Date and time. */
    uint8_t set_params = 0; /* Flags of parameters that have been set (mostly
    * for PRM_SRVR_*. */

    /* Load current server configuration. */
    getGAR(gateway);
    getSIPR(iaddr);
    getSUBR(subnet);
    status = TXF_STATUS_200;

    if(req->method == METHOD_PUT) {
        if(!(*parser)(tokens, params, 8)) {

            /* Date and day must both be set, if any one of them is present. */
            if(PARAM_IS_SET(params, PRM_SRVR_DATE)
            && PARAM_IS_SET(params, PRM_SRVR_DAY)) {

                if(str_to_date(&dt, s_date) || day > 7) {
                    status      =  TXF_STATUS_400;

                /* Set date to RTC. */
                } else {
                    set_date(&dt, day);
                }
            }

            /* If any one of PRM_SRVR_GATEWAY, IADDR or SUBNET has been set,
            * it is parsed into numbers but only applied *after* the response
            * has been sent out with the current configuration. */
            if(PARAM_IS_SET(params, PRM_SRVR_GATEWAY)) {
                if(str_to_inet(gateway,
                              (uint8_t*)params[PRM_SRVR_GATEWAY].data_ptr)) {
                    status      =  TXF_STATUS_400;
                } else {
                    set_params |=  BV(PRM_SRVR_GATEWAY);
                }
            }

            if(PARAM_IS_SET(params, PRM_SRVR_IADDR)) {
                if(str_to_inet(iaddr,
                              (uint8_t*)params[PRM_SRVR_IADDR].data_ptr)) {
                    status      =  TXF_STATUS_400;
                } else {
                    set_params |=  BV(PRM_SRVR_IADDR);
                }
            }

            if(PARAM_IS_SET(params, PRM_SRVR_SUBNET)) {
                if(str_to_inet(subnet,
                              (uint8_t*)params[PRM_SRVR_SUBNET].data_ptr)) {
                    status      =  TXF_STATUS_400;
                } else {
                    set_params |=  BV(PRM_SRVR_SUBNET);
                }
            }

            /* New maximum coordinates may be specified at any time; the motors
            * will automatically reset. */
            if(PARAM_IS_SET(params, PRM_SRVR_X)
            || PARAM_IS_SET(params, PRM_SRVR_Y)
            || PARAM_IS_SET(params, PRM_SRVR_Z)) {

                /* If the exceed the physical limits of the device, return the
                * absolute maximum values. */
                if(motor_set_max(&max)) {
                    status      =  TXF_STATUS_400;
                    max.x       =  GRID_X_LEN;
                    max.y       =  GRID_Y_LEN;
                    max.z       =  GRID_Z_LEN;
                }
            }

        } else {
            status      =  TXF_STATUS_400;
        }
    }

    switch(status) {
        case TXF_STATUS_200:

            size += inet_to_str(s_gateway, gateway);
            size += inet_to_str(s_iaddr, iaddr);
            size += inet_to_str(s_subnet, subnet);
            get_date(&dt, &day);
            date_to_str(s_date, &dt);

            srvr_prep(TXF_STATUS_200, TXF_ln,
                      TXF_STANDARD_HEADERS_ln,
                      TXF_CONTENT_LENGTH, TXF_HS, TXFx_FW_UINT, size,
                      TXF_lnln);

            (*serialiser)(tokens, params, 8, SERIAL_DEFAULT);

            /* Apply changes to the address after the response has been sent. */
            if(set_params) {
                if(set_params & BV(PRM_SRVR_GATEWAY)) {
                    setGAR(gateway);
                }
                if(set_params & BV(PRM_SRVR_IADDR)) {
                    setSIPR(iaddr);
                    srvr_set_host_name_ip(iaddr);
                }
                if(set_params & BV(PRM_SRVR_SUBNET)) {
                    setSUBR(subnet);
                    applySUBR();
                }
            }

        break;
        case TXF_STATUS_400:

            srvr_send(TXF_STATUS_400, TXF_ln,
                      TXF_STANDARD_HEADERS_ln,
                      TXF_CONTENT_LENGTH, TXF_HS, TXFx_FW_UINT, 38,
                      TXF_lnln);

            (*serialiser)(&tokens[5], &params[5], 3, SERIAL_DEFAULT);
        break;
    }
}

/**
* @ingroup resource
* @brief Manage device positioning.
*
* Method GET:
* Returns the current coordinates of the device head.
*   - 200 OK; the body contains the current coordinates of the head. Format:
*       @verbatim {
    "x"         : number,
    "y"         : number,
    "z"         : number
} @endverbatim
*   - 503 Service Unavailable; the device is currently positioning itself. The
*       request should be reattempted later, as specified in the `Retry-After'
*       header. Currently, the estimated time is hard-coded but could change in
*       the future.
*
* Method PUT:
* Moves the device head to the specified coordinates.
* The message body should include any of the above specified keys, the order of
* which is irrelevant. Only the last instance of each key is preserved. Returns:
*   - 200 OK; the device is already at the specified position -- no action will
*       be taken. The body contains the current coordinates. The format is the
*       same as above.
*   - 202 Accepted; the specified coordinates were valid and the device's
*       repositioning has been initiated. Header `Retry-After' designates the
*       estimated time until completion.
*   - 400 Bad request; the specified coordinates lay outside the allowable
*       device-space or an invalid parameter and/or value has been specified.
*       The body contains the maximum acceptable values for each axis. The
*       format is the same as above.
*   - 503 Service Unavailable; same as with GET.
*/
void rsrc_handle_coordinates(HTTPRequest* req) {
    Position pos;

    /* Position reading and updating may only be performed if the motors are
    * not being operated. In either case, the current position is read and
    * either returned (in case of GET), or updated with a new value (in case of
    * PUT). Return 503 Service Unavailable, otherwise. */
    if(motor_get(&pos)) {
        srvr_send(TXF_STATUS_503, TXF_ln,
                  TXF_STANDARD_HEADERS_ln,
                  TXF_CONTENT_LENGTH_ZERO_ln,
                  TXF_RETRY_AFTER, TXF_HS, TXFx_FW_UINT, 10,
                  TXF_lnln);
        return;
    }

    uint8_t  status;            /* Status of response. */
    uint8_t  token_buf[6];      /* Key tokens. */
    uint8_t* tokens[3];         /* Pointers to each token in @c token_buf. */
    Position npos =  pos;       /* New position. */

    ParamValue params[]     =  {PARAM_UINT8(npos.x),
                                PARAM_UINT8(npos.y),
                                PARAM_UINT8(npos.z)};

    pgm_read_str_array(tokens, token_buf, prm_x, prm_y, prm_z, NULL);

    if(req->method == METHOD_PUT) {

        /* If an acceptable set of parameters have been parsed, attempt to use
        * @c pos to update motor position. It could still fail if, for example,
        * an out-of-range value has been specified. */
        int8_t retval;
        if(!(retval = json_parse(tokens, params, 3))) {

            /* Already there. */
            if(pos.x == npos.x && pos.y == npos.y && pos.z == npos.z) {
                status      =  TXF_STATUS_200;

            /* Invalid coordinate (out of bounds). */
            } else if(motor_set(npos)) {
                status      =  TXF_STATUS_400;

            /* Position is valid and will be processed. Respond with a 202
            * Accepted and a `Retry-Later' header to indicate the estimated
            * completion time. */
            } else {
                status      =  TXF_STATUS_202;
            }

        /* Wrong argument. */
        } else {
            status      =  TXF_STATUS_400;
        }

    } else if(req->method == METHOD_GET) {
        status      =  TXF_STATUS_200;
    }

    switch(status) {
        case TXF_STATUS_200:
            srvr_prep(TXF_STATUS_200, TXF_ln,
                      TXF_STANDARD_HEADERS_ln,
                      TXF_CONTENT_TYPE, TXF_HS,
                          /* This should be using req->accept, after it is set
                          * to specific type (ie, not app/* but app/json). */
                          TXFx_FROMRAM, MIME_MIN + MIME_APP_JSON, TXF_ln,
                      TXF_CONTENT_LENGTH, TXF_HS, TXFx_FW_UINT, 38,
                      TXF_lnln);
            (*serialiser)(tokens, params, 3, SERIAL_DEFAULT);

        break;
        case TXF_STATUS_202:
            /* TODO: Return an *actual* estimate with `Retry-After'. */
            srvr_send(TXF_STATUS_202, TXF_ln,
                      TXF_STANDARD_HEADERS_ln,
                      TXF_CONTENT_LENGTH_ZERO_ln,
                      TXF_RETRY_AFTER, TXF_HS, TXFx_FW_UINT, 10,
                      TXF_lnln);

        break;
        case TXF_STATUS_400:
            srvr_send(TXF_STATUS_400, TXF_ln,
                      TXF_STANDARD_HEADERS_ln,
                      TXF_CONTENT_LENGTH, TXF_HS, TXFx_FW_UINT, 38,
                      TXF_lnln);

            /* Return maximum values. */
            motor_get_max(&npos);

            (*serialiser)(tokens, params, 3, SERIAL_DEFAULT);
        break;
        case TXF_STATUS_503:
            /* This is implemented as a guard statement in the beginning of the
            * the function. */
        break;
    }
}

void rsrc_handle_index(HTTPRequest* req) {
}

/**
* @brief Serialise statistical information of records to be returned.
*
* The result is similar to this: @verbatim {
    "page-index": number,
    "page-size" : number,
    "total"     : number
@endverbatim
*
* No flushing is performed.
*
* @param[in] page_index The page of results.
* @param[in] page_size The (maximum) number of records within this
*   page/response.
* @param[in] total The amount of available records (regardless of pagination).
*/
static void rsrc_measurement_serial_info(uint8_t page_index,
                                         uint8_t page_size,
                                         uint8_t total) {

    uint8_t  token_buf[31];     /* Key tokens. */
    uint8_t* tokens[4];         /* Pointers to each token in @c token_buf. */

    ParamValue params[] =  {PARAM_UINT8(page_index),
                            PARAM_UINT8(page_size),
                            PARAM_UINT8(total)};

    /* Load tokens into main memory. */
    pgm_read_str_array(tokens, token_buf, prm_page_index,
                                          prm_page_size,
                                          prm_total,
                                          prm_log, NULL);

    (*serialiser)(tokens, params, 3, SERIAL_ATOMIC_S);

    /* An envelope directive takes precedence over the actual parameters so, in
    * this case, needs to be opened independently. */
    (*serialiser)(&tokens[3], NULL, 1, SERIAL_PRECEDED | SERIAL_ENVELOPE_S);

}

/**
* @brief Serialise the records contained within @p set.
*
* The result is similar to this: @verbatim {
    "date"      : "YYYY-MM-DDTHH:mm:ss.000Z",
    "x"         : number,
    "y"         : number,
    "t"         : temperature,      // One-digit fraction; in Celsius.
    "ph"        : pH,               // Not sampled; always 255.
    "rh"        : relative humidity // Not sampled; always 255.
}
@endverbatim
*
* Additional objects may follow.
*
* No flushing is performed.
*
* @param[in,out] set #LogRecordSet from which to extract records. Upon return,
*   @p set will be empty unless @p count records have been serialised, instead.
* @param[in] count Maximum number of records to serialise.
* @returns The amount of serialised records.
*/
static uint8_t rsrc_measurement_serial_log(LogRecordSet* set, uint8_t count) {
    uint8_t  i;                 /* Counts the amount of serialised records. */
    uint8_t  token_buf[47];     /* Key tokens. */
    uint8_t* tokens[6];         /* Pointers to each token in @c token_buf. */

    /* Parameter value buffers. */
    uint8_t  s_date     [PRM_DATE_LEN];     /* String form of rec.date */
    uint8_t  s_temp     [PRM_TEMP_LEN];     /* String form of rec.t */

    LogRecord rec;

    /* Set-up output value references. */
    ParamValue params[]     =  {PARAM_STRING(s_date, PRM_DATE_LEN),
                                PARAM_UINT8(rec.x),
                                PARAM_UINT8(rec.y),
                                PARAM_STRING(s_temp, PRM_TEMP_LEN),
                                PARAM_UINT8(rec.ph),
                                PARAM_UINT8(rec.rh)};

    /* Load tokens into main memory. */
    pgm_read_str_array(tokens, token_buf, prm_date,
                                          prm_x,
                                          prm_y,
                                          prm_t,
                                          prm_ph,
                                          prm_rh, NULL);

    /* The first record is not preceded by another record. */
    uint8_t serial = SERIAL_ATOMIC_S | SERIAL_ATOMIC_E;

    while(!log_get_next(&rec, set) && i < count) {

        date_to_str(s_date, &rec.date);
        temp_to_str(s_temp, PRM_TEMP_LEN, rec.t);

        printf("T %d → %s\n", rec.t, s_temp);

        (*serialiser)(tokens, params, 6, serial);

        serial |= SERIAL_PRECEDED;
        ++i;
    }
    return i;
}

/**
* @ingroup resource
* @brief Manage device measurements.
*
* Method GET:
* Fetch logged measurements. By default, all measurements are returned. The
* following query string parameters may be used to return a subset of results:
* @verbatim
    date-since                  // Format ISO8601: YYYY-MM-DDTHH:mm:ss[.sss][Z]
    date-until                  // Format ISO8601: YYYY-MM-DDTHH:mm:ss[.sss][Z]
    page-index                  // 0 up to 255
    page-size                   // 0 up to 255
@endverbatim
* Additional notes:
*   - Fraction of second and time-zone (in dates) are never parsed and may be
*       omitted. Time-zone is always UTC (@c Z).
*   - Dates since 2000-01-01 up to 2099-12-31 are accepted.
*   - Pagination is applied last, ie, after a subset of results has been
*       selected based on the other parameters, if such parameters were
*       specified.
*   - Any of the above specified parameters may be used, the order of which is
*       irrelevant. If found multiple times, only the first non-empty value of
*       each is preserved.
*   - Unrecognised parameters are ignored. Permissible parameters, although
*       optional, must be valid. Otherwise, 400 is returned.
*   - @c page-index is only relevant, if @c page-size has been specified (to a
*       value greater than @c 0).
*   - To query the amount of records without fetching them, @c page-size may be
*       set to @c 0. @c total will contain the amount of available records (see
*       response format below).
*
* Returns:
*   - 200 OK; the body contains the logged measurements. Format: @verbatim {
    "page-index": number,
    "page-size" : number,
    "total"     : number,
    "log"       : [{
            "date"      : "YYYY-MM-DDTHH:mm:ss.000Z",
            "x"         : number,
            "y"         : number,
            "t"         : temperature,      // One-digit fraction; in Celsius.
            "ph"        : pH,               // Not sampled; always 255.
            "rh"        : relative humidity // Not sampled; always 255.
        },
    …
]} @endverbatim
*   Additional notes:
*       - @c page-index is the requested result page. Defaults to @c 0, if none
*           was specified.
*       - @c page-size is the maximum size of @c log. Defaults to @c  total, if
*           none was specified.
*       - @c total is the available amount of records, if pagination options
*           were not applied.
*       - @c log contains a maximum of @c page-size measurement records. It is
*           always present, even if it is empty.
*   - 400 Bad Request; if a wrong value for any of the permissible parameters
*       has been specified.
*   - 414 Request-URI Too Long; the query string exceeds the allocated buffer
*       size. The request should be reconstructed to contain a smaller query
*       string.
*
* Example: @verbatim
/measurement?page-size=10&page-index=2&date-since=2015-01-01T12:30:00
@endverbatim
*
* Method POST:
* Performs and records a new measurement.
* The message body should be empty. Returns:
*   - 202 Accepted; sampling has been initiated. Header `Retry-After' designates
*       the estimated time until completion.
*   - 503 Service Unavailable; the device is currently busy (repositioning
*       itself or taking a measurement). The request should be reattempted
*       later, as specified in the `Retry-After' header. Currently, the
*       estimated time is hard-coded but could change in the future.
*/
void rsrc_handle_measurement(HTTPRequest* req) {
    uint8_t  status;                /* Status of response. */

    if(req->method == METHOD_POST) {

        Position pos;
        if(motor_get(&pos)) {
            status      =  TXF_STATUS_503;

        } else {
            task_log_sample(&pos);

            status      =  TXF_STATUS_202;
        }

    } else if(req->method == METHOD_GET) {

        /* Default date limits. */
        BCDDate since   = {.year = 0x00, .mon = 0x01, .date = 0x01,
                           .hour = 0x00, .min = 0x00, .sec  = 0x00};
        BCDDate until   = {.year = 0x99, .mon = 0x12, .date = 0x31,
                           .hour = 0x23, .min = 0x59, .sec  = 0x59};

        QueryString* q = &req->query;   /* Access to query parameters. */
        uint8_t page_index  =  0;       /* Requested page index. */
        uint8_t page_size   =  0;       /* Requested page size. */
        uint8_t total;                  /* Total records returned. */
        uint8_t errors      =  0;       /* Parser errors. */
        uint16_t size       = 71;       /* Content-length with 0 records. */
        LogRecordSet set;               /* Results that match current params.*/

        /* Parse string values for the query string. */
        if(q->values[PRM_MSR_DATE_SINCE]) {
            errors     +=  str_to_date(&since, q->values[PRM_MSR_DATE_SINCE]);
        }
        if(q->values[PRM_MSR_DATE_UNTIL]) {
            errors     +=  str_to_date(&until, q->values[PRM_MSR_DATE_UNTIL]);
        }
        if(q->values[PRM_MSR_PAGE_SIZE]) {
            page_size   =  atoi(q->values[PRM_MSR_PAGE_SIZE]);
        }
        if(q->values[PRM_MSR_PAGE_INDEX]) {
            page_index  =  atoi(q->values[PRM_MSR_PAGE_INDEX]);
        }

        /* Execute the request, if there were no errors in the params.*/
        if(!errors) {

            /* Find records within the specified dates. */
            total   =  log_get_set(&set, &since, &until);
            if(total) {
                /* @c 102 includes the object separator (comma), which is
                * excluded for the first record. */
                size   += 102*total - 1;
            }

            /* TODO: Serialise in chunks. */

            srvr_prep(TXF_STATUS_200, TXF_ln,
                      TXF_STANDARD_HEADERS_ln,
                      TXF_CONTENT_LENGTH, TXF_HS, TXFx_FW_UINT, size,
                      TXF_lnln);

            /* Serialise statistical info. */
            rsrc_measurement_serial_info(page_index, page_size, total);

            /* Serialise the records, if any. */
            if(total) {
                rsrc_measurement_serial_log(&set, total);
            }

            /* rsrc_measurement_info() opens an envelope. Finalise it, as well
            * an the whole object and then flush the response. */
            (*serialiser)(NULL, NULL, 1, SERIAL_ENVELOPE_E
                                       | SERIAL_ATOMIC_E
                                       | SERIAL_FLUSH);

        /* Return 400 on erroneous parameter values. */
        } else {
            status      =  TXF_STATUS_400;
        }
    }

    switch(status) {
        case TXF_STATUS_202:
            /* TODO: Return an *actual* estimate with `Retry-After'. */
            srvr_send(TXF_STATUS_202, TXF_ln,
                      TXF_STANDARD_HEADERS_ln,
                      TXF_CONTENT_LENGTH_ZERO_ln,
                      TXF_RETRY_AFTER, TXF_HS, TXFx_FW_UINT, 10,
                      TXF_lnln);

        break;
        case TXF_STATUS_400:
            srvr_send(TXF_STATUS_400, TXF_ln,
                      TXF_STANDARD_HEADERS_ln,
                      TXF_CONTENT_LENGTH_ZERO_ln, TXF_ln);

        break;
        case TXF_STATUS_503:
            srvr_send(TXF_STATUS_503, TXF_ln,
                      TXF_STANDARD_HEADERS_ln,
                      TXF_CONTENT_LENGTH_ZERO_ln,
                      TXF_RETRY_AFTER, TXF_HS, TXFx_FW_UINT, 10,
                      TXF_lnln);
        break;
        /* Case TXF_STATUS_200 is implemented in-line, above. */
    }
}

void rsrc_handle_style_css(HTTPRequest* req) {
}

