;(function (ns) {

    /**
    * @brief Manage Page state.
    *
    * The state of the page (or, alternatively, the page name), and thus the
    * content to load, is determined by the URI fragment (or hash) part of which
    * is interpreted as the state indicator. During initialisation (see init()),
    * the set of acceptable states are specified along with their respective
    * handler functions.
    *
    * The URI fragment may, optionally, contain comma-separated strings, each a
    * parameter followed by a colon and a value. When calling update() to prompt
    * PageMonitor for a page renewal, the current fragment is split into an
    * array of such strings. The first string is the state indicator. The rest,
    * are converted into an object (see makeParams()) and passed into the
    * corresponding handler.
    */
    ns.PageMonitor = ns.PageMonitor || (function () {
        var states,
            pages,
            menus,
            defaultName;

        /**
        * @brief Initialise the page monitor.
        *
        * When specifying page names and their respective handler, care should
        * be taken to *not* include the hash sign as part of the name. Also,
        * static-content pages need not specify a handler; the @p page and @p
        * name will be switchedTo(), but no additional code will be executed.
        * Names that are not specified, are ignored. As a special case, @p name
        * determines the default page/menu when there is no URI fragment at all.
        *
        * @param[in] blob Either an object of state-handler pairs, or an array
        *   of states (names) in which case @p arrFn must contain the
        *   corresponding callback functions.
        * @param[in] arrFn Array of functions to call for each item in @p blob.
        *   It is ignored, if @blob is an object.
        * @param[in] page Typically, object of OneOfMany. switchTo() is used to
        *   select the new page by passing the page name from the window's hash.
        * @param[in] menu Typically, object of OneOfMany. switchTo() is used to
        *   select the new menu by passing the page name from the window's hash.
        * @param[in] name The page/menu to select, if the hash contains no name
        *   (ie, there is only a hash sign or no URI fragment at all).
        */
        var init = function (blob, arrFn, page, menu, name) {
            var i;

            if(blob instanceof Array) {
                /* Merge state names and handlers into one. */
                states  =  {};
                for(i = 0 ; i < blob.length ; ++i) {
                    states[blob[i]] =  arrFn[i];
                }
            } else {
                states  =  blob;
            }

            pages       =  page;
            menus       =  menu;
            defaultName =  name;
        };

        /**
        * @brief Display a new page and menu, if necessary.
        *
        * It expects to find the name of the page to load at the beginning of
        * the window's hash, optionally terminated by a comma (more on this
        * later). If the string extracted this way matches any of the specified
        * names, switchTo() is invoked on @c pages and @c menu. Also, if a
        * handler has been specified for that name, it is invoked.
        *
        * Each handler, upon invocation, receives an object of parameters
        * specified along with the URI fragment. As one would expect, each
        * member is the named after a parameter and contains its value. Note
        * that the format of these parameters in *not* the same as with a query
        * string; this decision was made to support more readable/user-friendly
        * URIs. Instead, parameters are separated by a comma whereas
        * parameter-value pairs are connected with a colon. For details on
        * this implementation's limitations, see makeParams().
        *
        *
        * Typically, this should be called every time the window fragment is
        * altered.
        */
        var update = function () {
            var hash        =  window.location.hash.split(","),
                handler;

            /* If there is no query fragment, use the default menu (which has
            * `href' set to @c idHome. In any case, the hash sign is removed. */
            if(hash[0] === "") {
                hash[0]     =  defaultName;
            } else {
                if(hash[0].indexOf("#") == 0) {
                    hash[0] =  hash[0].substr(1);
                }
            }

            /* Any unknown hashes will not affect the state of the page. */
            handler         =  states[hash[0]];

            if(handler !== undefined) {
                pages.switchTo(hash[0]);
                menus.switchTo(hash[0]);
                gNS.Logger.update();

                /* @p p is typically an array of strings. Form: ["key1:value1", …].
                * Convert that into an object and pass it to the handler. */
                if(handler !== null) {
                    hash.shift();
                    handler(makeParams(hash));
                }
            }
        };

        /**
        * @brief Split each item of @p arrStr into key and value.
        *
        * The array is traversed from the beginning to the end. Each item is
        * split into two parts; one up to the first occurrence of a colon (:);
        * the other, after the colon up to the end. If an item starts with a
        * colon, it is omitted. If an item does not contain a colon and
        * value-part, it is included as a key, but it is set to @c null. Any
        * item with a colon but no value is set to an empty string (@c "").
        *
        * The parsing is not performed by JSON.parse() and, thus, the key-part
        * of each item need not (or, better, *must not*) be enclosed in double
        * quotes. The value of the last occurrence of each key is preserved.
        *
        * Currently, only single-level parsing is performed (ie, arrays and
        * objects are passed as string values to their respective key).
        *
        * @param[in] arrStr An array of `key:value' strings.
        * @returns An object with key-value pairs extracted from the array
        * items.
        */
        var makeParams = function (arrStr) {
            var object  = {},   // The object to return
                colon,          // Index of the first colon in each @p arrStr
                key,            // Key to add
                i;

            for(i = 0; i < arrStr.length ; ++i) {
                colon   =  arrStr[i].indexOf(":");

                if(colon > 0) {
                    /* Write to a key equal to each string up to the first colon and
                    * use the rest of the string (after the colon) as its value. */
                    object[arrStr[i].substring(0, colon)]
                        =  arrStr[i].substring(colon + 1);

                } else if(colon < 0) {

                    /* If no colon was found, append the key without a value. */
                    object[arrStr[i]]   =  null;
                }
            }
            return object;
        };

        return {"init"      : init,
                "update"    : update};
    })();
})(window.gNS = window.gNS || {});
