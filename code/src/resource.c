
#include "resource.h"
#include "param.h"
#include "http_server.h"

#include <stdio.h>
#include <inttypes.h>

/**
* @ingroup resource
* @brief The number of token-handler pairs in #resources.
*/
#define RSRC_LEN    5

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
static uint8_t* rsrc_tokens[] = {};

/**
* @ingroup resource
* @brief Available methods and corresponding handlers for each resource.
*
* This array and #rsrc_tokens are correlated through #resources.
*/
static ResourceHandler rsrc_handlers[]; /* See the definition further below. */

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
        rsrc_resources.handlers[uri].methods = methods;
        rsrc_resources.handlers[uri].call    = handler;
    }
}

static ResourceHandler rsrc_handlers[] = {};
