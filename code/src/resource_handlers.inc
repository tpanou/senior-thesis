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

#include <avr/pgmspace.h>
#include <inttypes.h>

/**
* @brief Token: x
*
* This is used by rsrc_handler_coordinates() to read and return device
* coordinates.
*/
static uint8_t prm_x[] PROGMEM = "x";

/**
* @brief Token: y
*
* This is used by rsrc_handler_coordinates() to read and return device
* coordinates.
*/
static uint8_t prm_y[] PROGMEM = "y";

/**
* @brief Token: z
*
* This is used by rsrc_handler_coordinates() to read and return device
* coordinates.
*/
static uint8_t prm_z[] PROGMEM = "z";

void rsrc_handle_server(HTTPRequest* req) {
}

void rsrc_handle_root(HTTPRequest* req) {
}

void rsrc_handle_client_js(HTTPRequest* req) {
}

void rsrc_handle_configuration(HTTPRequest* req) {
}

/**
* @ingroup resource
* @brief Manage device positioning.
*
* Method GET:
*   - 200 OK; the body contains the current coordinates of the device. Format:
*       @verbatim {
    "x" : number , 
    "y" : number , 
    "z" : number
 } @endverbatim
*   - 503 Service Unavailable; the device is currently positioning itself. The
*       request should be reattempted later, as specified in the `Retry-After'
*       header. Currently, the estimated time is hard-coded but could change in
*       the future.
*
* Method PUT:
*   - 200 OK; the device is already at the specified position -- no action will
*       be taken. The body contains the current coordinates. The format is the
*       same as above.
*   - 202 Accepted; the specified coordinates were valid and the device's
*       repositioning has been initiated. Header `Retry-After' designates the
*       estimated time until completion.
*   - 400 Bad request; the specified coordinates lay outside the allowable
*       device-space or an invalid parameter and/or value has been specified.
*       The body contains the maximum acceptable values. The format is the same
*       as above.
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
        if(!json_parse(tokens, params, 3)) {

            /* Already there. */
            if(pos.x == npos.x && pos.y == npos.y && pos.z == pos.z) {
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
                      TXF_CONTENT_LENGTH, TXF_HS, TXFx_FW_UINT, 45,
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
                      TXF_CONTENT_LENGTH, TXF_HS, TXFx_FW_UINT, 45,
                      TXF_lnln);

            /* Return maximum values. */
            npos.x = GRID_X_LEN;
            npos.y = GRID_Y_LEN;
            npos.z = GRID_Z_LEN;

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

