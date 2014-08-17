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
    /* TODO: Allow for browsers that do not support `addEventListener'. */
    window.addEventListener("hashchange", load_state);
    window.addEventListener("load", load_state);

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
    }

    function handle_show_log() {
    }

})();
