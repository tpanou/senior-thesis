
#include "resource.h"
/* See #include "resource_handlers.inc" further below. */
#include "param.h"
#include "http_server.h"

#include <stdio.h>
#include <inttypes.h>

/**
* @ingroup resource
* @brief The number of token-handler pairs in #rsrc_handlers.
*/
#define RSRC_LEN    8

/**
* @brief Index of /measurement in #rsrc_handlers.
*
* Such macros are only defined for resources that accept query parameters.
*/
#define RSRC_MEASUREMENT    6

/**
* @ingroup resource
* @brief Function pointer to a parser conforming to the param.h module.
*
* This is set using rsrc_set_parser() and it should be set before any resource
* handlers are invoked.
*/
static int8_t (*parser)(uint8_t**, ParamValue*, uint8_t);

/**
* @ingroup resource
* @brief Serialising function pointer conforming to the param.h module.
*
* It is set using rsrc_set_serial() and it should be set before any resource
* handlers are invoked.
*/
static void (*serialiser)(uint8_t**, ParamValue*, uint8_t, uint8_t);

/*
* This module is divided into two parts; this file, containing common base
* functions, and resource_handlers.inc containing the definition of each
* resource handler. The latter is included after #parser and #serialiser have
* been declared because they are needed in the included source code.
* resource_handlers.inc should not be compiled separately.
*/
#include "resource_handlers.inc"

/**
* @ingroup resource
* @brief Absolute path tokens of resources exposed by the HTTP server.
*/
static uint8_t* rsrc_tokens[RSRC_LEN] = {
    "*",
    "/",
    "/client.js",
    "/configuration",
    "/coordinates",
    "/index",
    "/measurement",
    "/style.css"
};

/**
* @ingroup resource
* @brief Available methods and corresponding handlers for each resource.
*
* This array and #rsrc_tokens are correlated through srvr_set_resources().
*/
static ResourceHandler rsrc_handlers[RSRC_LEN]; /* Definition is in the end. */

void rsrc_init() {
    srvr_set_resources(rsrc_tokens, rsrc_handlers, RSRC_LEN);
}

void rsrc_set_parser(int8_t
                    (*new_parser)(uint8_t**, ParamValue*, uint8_t len)) {
    parser  = new_parser;
}

void rsrc_set_serial(void (*new_serialiser)(uint8_t**,
                                           ParamValue*,
                                           uint8_t len,
                                           uint8_t ctr)) {
    serialiser  = new_serialiser;
}

void rsrc_set_handler(uint8_t uri,
                      uint8_t methods,
                      void (*handler)(HTTPRequest*)) {

    if(uri < RSRC_LEN) {
        rsrc_handlers[uri].methods = methods;
        rsrc_handlers[uri].call    = handler;
    }
}

void rsrc_inform(struct HTTPRequest* req) {
    rsrc_get_qparam(req);
}

static inline void rsrc_get_qparam(struct HTTPRequest* req) {
    uint16_t offset;    /* QueryString.buf_i. */
    uint8_t i;          /* Number of permissible parameters. */

    if(req->uri == RSRC_MEASUREMENT && req->method == METHOD_GET) {
        i = 4;
        offset = pgm_read_str_array(req->query.tokens,
                                    req->query.buf,
        /* These string addresses are defined in resource_handlers.inc. */
                                    prm_date_since,
                                    prm_date_until,
                                    prm_page_index,
                                    prm_page_size,
                                    NULL);

    } else {
        i       =  0;
        offset  =  QUERY_BUF_LEN;
    }

    req->query.buf_len  =  QUERY_BUF_LEN;
    req->query.buf_i    =  offset;

    req->query.count = i;

    /* Reset .values to a known value to indicate they have not yet been set.
    * This is used in place of memset(). */
    while(i) {
        --i;
        req->query.values[i] = NULL;
    }
}

static ResourceHandler rsrc_handlers[RSRC_LEN] = {
    {.methods = HTTP_OPTIONS,   .call = &rsrc_handle_server},
    {.methods = HTTP_GET,       .call = &rsrc_handle_root},
    {.methods = HTTP_GET,       .call = &rsrc_handle_client_js},
    {.methods = HTTP_GET
              | HTTP_PUT,       .call = &rsrc_handle_configuration},
    {.methods = HTTP_GET
              | HTTP_PUT,       .call = &rsrc_handle_coordinates},
    {.methods = HTTP_GET,       .call = &rsrc_handle_index},
    {.methods = HTTP_GET
              | HTTP_POST,      .call = &rsrc_handle_measurement},
    {.methods = HTTP_GET,       .call = &rsrc_handle_style_css}
};
