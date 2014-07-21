
#include "resource.h"
#include "param.h"
#include "http_server.h"

#include <stdio.h>
#include <inttypes.h>

/**
* @ingroup resource
* @brief The number of token-handler pairs in #resources.
*/
#define RSRC_LEN    8

/**
* @ingroup resource
* @brief Function pointer to a parser conforming to the param.h module.
*
* This is set using rsrc_set_parser() and it should be done before any resource
* handlers are invoked.
*/
static int8_t (*parser)(uint8_t**, ParamValue*, uint8_t len);

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
* This array and #rsrc_tokens are correlated through #resources.
*/
static ResourceHandler rsrc_handlers[RSRC_LEN]; /* Definition is in the end. */

void rsrc_init() {
    srvr_set_resources(rsrc_tokens, rsrc_handlers, RSRC_LEN);
}

void rsrc_set_parser(int8_t
                    (*new_parser)(uint8_t**, ParamValue*, uint8_t len)) {
    parser  = new_parser;
}

void rsrc_set_handler(uint8_t uri,
                      uint8_t methods,
                      void (*handler)(HTTPRequest*)) {

    if(uri < RSRC_LEN) {
        rsrc_handlers[uri].methods = methods;
        rsrc_handlers[uri].call    = handler;
    }
}

void rsrc_handle_server(HTTPRequest* req) {
}

void rsrc_handle_root(HTTPRequest* req) {
}

void rsrc_handle_client_js(HTTPRequest* req) {
}

void rsrc_handle_configuration(HTTPRequest* req) {
}

void rsrc_handle_coordinates(HTTPRequest* req) {
}

void rsrc_handle_index(HTTPRequest* req) {
}

void rsrc_handle_measurement(HTTPRequest* req) {
}

void rsrc_handle_style_css(HTTPRequest* req) {
}

static ResourceHandler rsrc_handlers[RSRC_LEN] = {
    {.methods = HTTP_OPTIONS,   .call = &rsrc_handle_server},
    {.methods = HTTP_GET,       .call = &rsrc_handle_root},
    {.methods = HTTP_GET,       .call = &rsrc_handle_client_js},
    {.methods = HTTP_GET
              | HTTP_PUT,       .call = &rsrc_handle_configuration},
    {.methods = HTTP_GET
              | HTTP_POST,      .call = &rsrc_handle_coordinates},
    {.methods = HTTP_GET,       .call = &rsrc_handle_index},
    {.methods = HTTP_GET
              | HTTP_POST,      .call = &rsrc_handle_measurement},
    {.methods = HTTP_GET,       .call = &rsrc_handle_style_css}
};
