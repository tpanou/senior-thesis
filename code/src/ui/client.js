(function() {

    /* An object of key-value pairs of URL fragments and their corresponding
    * handler function that updates the page as required. Note that the hash
    * sign must be provided as part of all keys. An exception to this is the
    * empty string which is matched against no hash at all and hash with no
    * succeeding text. */
    /* TODO: Verify that this last bit is the default behaviour of browsers
    * other than FF. */
    var STATE_HANDLERS = {
        ""              : handle_index,
        "#content"      : handle_show_log
    }

    /* Add listeners to update the page content when the URL fragment is
    * altered. Reloading the state `onhashchange' responds to hash changes due
    * to history traversal (eg, pressing back or forward), whereas `onload'
    * deals with direct visiting (eg, a bookmarked link or page refresh). */
    sfAddEventListener(window, "hashchange", loadState);
    sfAddEventListener(window, "load", loadState);

    /**
    * @brief Updates the main content of the page.
    *
    * The state of the page, and thus the content to load, is determined by the
    * URL fragment (or hash) which is interpreted as a state indicator. This
    * function simply calls the handler specified for the corresponding hash
    * in #STATE_HANDLERS (see handle* functions). */
    function loadState() {
        var hash        =  window.location.hash,
            handler     =  STATE_HANDLERS[hash];

        /* Any unknown hashes will not affect the state of the page. */
        if(handler) {
            handler();
        }
    }

    function handleIndex() {
        document.getElementById("content").innerHTML = "";
    }

    function handleShowLog() {
        document.getElementById("content").innerHTML = "Hello world";
    }

    /*
    * Safe functions.
    *
    * Functions that attempt to provide some compatibility layer with older
    * browsers. `sf' prefix stands for 'safe'.
    */

    /**
    * @brief Register a callback function for a particular element.
    *
    * Currently, a single callback is supported in fallback mode.
    *
    * @param[in] el The element to be attached a listener.
    * @param[in] event String of the event name (non-inclusive of "on").
    * @param[in] fn The callback function / event handler.
    */
    function sfAddEventListener(el, event, fn) {
        if(el.addEventListener) {
            el.addEventListener(event, fn);

        /* This should suffice for versions prior to IE9. */
        } else if(el.attachEvent) {
            el.attachEvent("on" + event, fn);

        } else {
            el["on" + event] = fn;
        }
    }

    /**
    * @brief Return an object that can be used to make HTTP request objects.
    *
    * If instantiation of XMLHttpRequest is not possible, Msxml2.XMLHTTP and
    * Microsoft.XMLHTTP will be attempted (in that order). If none of these
    * works, @c NULL is returned.
    *
    * @returns An XML HTTP object or @c NULL.
    */
    function sfCreateRequest() {
        var request;

        if(window.XMLHttpRequest) {
          request       =  new XMLHttpRequest();

        } else if(window.ActiveXObject) {
          try {
            request     =  new ActiveXObject("Msxml2.XMLHTTP");

          } catch(e) {
            try {
              request   =  new ActiveXObject("Microsoft.XMLHTTP");

            } catch(e) {
            }
          }
        }
        return request;
    }
})();
