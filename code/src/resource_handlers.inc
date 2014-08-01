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
* @brief Size of #prm_date (inclusive of null-byte).
*/
#define PRM_DATE_LEN        25

/**
* @ingroup resource
* @brief Size of any IPv4 address string (inclusive of null-byte).
*/
#define PRM_INET_LEN        16

#ifndef BV
/**
* @brief Set a single bit.
*
* @param[in] x The bit to set.
*/
#define BV(x)              (1 << x)
#endif

/**
* @ingroup resource
* @brief Token: x
*
* This is used by rsrc_handler_coordinates() to read and return device
* coordinates.
*/
static uint8_t prm_x[] PROGMEM = "x";

/**
* @ingroup resource
* @brief Token: y
*
* This is used by rsrc_handler_coordinates() to read and return device
* coordinates.
*/
static uint8_t prm_y[] PROGMEM = "y";

/**
* @ingroup resource
* @brief Token: z
*
* This is used by rsrc_handler_coordinates() to read and return device
* coordinates.
*/
static uint8_t prm_z[] PROGMEM = "z";

/**
* @ingroup resource
* @brief Token: date
*
* This is used by rsrc_handler_configuration() to parse a new or return the
* current server date and time.
*/
static uint8_t prm_date[] PROGMEM = "date";

/**
* @ingroup resource
* @brief Token: day
*
* This is used by rsrc_handler_configuration() to parse a new or return the
* current server day (eg, @c 1 for Sunday).
*/
static uint8_t prm_day[] PROGMEM = "day";

/**
* @ingroup resource
* @brief Token: gateway
*
* This is used by rsrc_handler_configuration() to parse a new or return the
* current server default gateway.
*/
static uint8_t prm_gateway[] PROGMEM = "gateway";

/**
* @ingroup resource
* @brief Token: ip
*
* This is used by rsrc_handler_configuration() to parse a new or return the
* current server IP address.
*/
static uint8_t prm_iaddr[] PROGMEM = "iaddr";

/**
* @ingroup resource
* @brief Token: subnet
*
* This is used by rsrc_handler_configuration() to parse a new or return the
* current server subnet mask.
*/
static uint8_t prm_subnet[] PROGMEM = "subnet";

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
    "date"      : string,       // Format ISO8601: YYYY-MM-DDTHH:mm:ss[.sss]Z
    "day"       : number,       // Day-of-the-week; 1 denotes Sunday
    "gateway"   : string,       // Server default gateway (dot-notation)
    "ip"        : string,       // Server IP address
    "subnet"    : string,       // Server subnet mask (xxx.xxx.xxx.xxx)
    "x"         : number,       // Custom maximum X dimension
    "y"         : number,       // Custom maximum Y dimension
    "z"         : number        // Custom maximum Z dimension
} @endverbatim
* Additional comments:
*   - Fraction of second (in "date") is always returned as @c 000 and never
*       parsed. Time-zone is UTC (@c Z).
*   - `dot-notation' is: xxx.xxx.xxx.xxx
*   - Custom maximum dimensions allow operating the device in a subset of the
*       maximum allowed device-space (#GRID_X_LEN, #GRID_Y_LEN, #GRID_Z_LEN).
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

   /* TODO: Current maximum position. */
    Position max  =  {GRID_X_LEN, GRID_Y_LEN, GRID_Z_LEN};

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
    RTCMap  dt;             /* Date and time. */
    uint8_t set_params = 0; /* Flags of parameters that have been set (mostly
    * for PRM_SRVR_*. */

    /* Load current server configuration. */
    getGAR(gateway);
    getSIPR(iaddr);
    getSUBR(subnet);
    status = TXF_STATUS_200;

    if(req->method == METHOD_PUT) {
        if(!(*parser)(tokens, params, 8)) {

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

        } else {
            status      =  TXF_STATUS_400;
        }
    }

    switch(status) {
        case TXF_STATUS_200:

            size += inet_to_str(s_gateway, gateway);
            size += inet_to_str(s_iaddr, iaddr);
            size += inet_to_str(s_subnet, subnet);
            rtc_format(s_date, &day);

            srvr_prep(TXF_STATUS_200, TXF_ln,
                      TXF_STANDARD_HEADERS_ln,
                      TXF_CONTENT_LENGTH, TXF_HS, TXFx_FW_UINT, size,
                      TXF_lnln);

            (*serialiser)(tokens, params, 8);

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
            (*serialiser)(tokens, params, 3);

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

            (*serialiser)(tokens, params, 3);
        break;
        case TXF_STATUS_503:
            /* This is implemented as a guard statement in the beginning of the
            * the function. */
        break;
    }
}

void rsrc_handle_index(HTTPRequest* req) {
}

void rsrc_handle_measurement(HTTPRequest* req) {
}

void rsrc_handle_style_css(HTTPRequest* req) {
}

