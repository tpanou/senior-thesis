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
    add_event_listener(window, "hashchange", load_state);
    add_event_listener(window, "load", load_state);

    /**
    * @brief Updates the main content of the page.
    *
    * The state of the page, and thus the content to load, is determined by the
    * URL fragment (or hash) which is interpreted as a state indicator. This
    * function simply calls the handler specified for the corresponding hash
    * in #STATE_HANDLERS (see handle_* functions). */
    function load_state() {
        var hash        =  window.location.hash,
            handler     =  STATE_HANDLERS[hash];

        /* Any unknown hashes will not affect the state of the page. */
        if(handler) {
            handler();
        }
    }

    function handle_index() {
        document.getElementById("content").innerHTML = "";
    }

    function handle_show_log() {
        document.getElementById("content").innerHTML = "Hello world";
    }

    /**
    * @brief Register a callback function for a particular element.
    *
    * Currently, a single callback is supported in fallback mode.
    *
    * @param[in] el The element to be attached a listener.
    * @param[in] event String of the event name (non-inclusive of "on").
    * @param[in] fn The callback function / event handler.
    */
    function add_event_listener(el, event, fn) {
        if(el.addEventListener) {
            el.addEventListener(event, fn);

        /* This should suffice for versions prior to IE9. */
        } else if(el.attachEvent) {
            el.attachEvent("on" + event, fn);

        } else {
            el["on" + event] = fn;
        }
    }

})();
