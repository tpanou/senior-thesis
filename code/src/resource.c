
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

/**
* @ingroup resource
* @brief The resources to be exposed by the HTTP server.
*
* Each string (which is an absolute path) in @c tokens corresponds to a
* ResourceHandler in @c rsrc_handlers.
*/
static ServerResources rsrc_resources = {
    .tokens     = rsrc_tokens,
    .handlers   = rsrc_handlers,
    .len        = RSRC_LEN
};

void rsrc_init() {
    /* TODO */
    #error rsrc_init not implemented.
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
