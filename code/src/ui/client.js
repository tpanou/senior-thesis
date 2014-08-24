(function() {

    /**************************************************************************\
    *
    * SECTION Globals
    *
    * Global variables that are used through this implementation (possibly
    * excluding any modules contained herein, such as `MenuBar').
    *
    \**************************************************************************/

    /* An object of key-value pairs of URI fragments (non-inclusive of the hash
    * sigh)
    * and their corresponding
    * handler function that updates the page as required. Note that the hash
    * sign must not be provided as part of the keys.
    */
    var STATE_HANDLERS = {
        "home"              : handlePageHome,
        "log"               : handlePageLog,
        "config"            : handlePageConfig,
        "operate"           : handlePageOperate,
        "help"              : handlePageHelp
    };

    /*
    * Common messages.
    */
    var MSG = {
        error   : {
            "iaddr4"        : "Χρειάζονται 4 αριθμοί (0 έως 255) " +
                              "διαχωρισμένοι με τελεία.",
            "date-invalid"  : "Μη έγκυρη ημερομηνία/ώρα.",
            "xyz-positive"  : "Απαιτούνται θετικοί ακέραιοι.",
            "not-int"       : "Απαιτείται ακέραιος.",
        }
    }

    /*
    * Current log search parameters.
    *
    * This is only used as convenience when constructing the `href' value of
    * pagination links.
    */
    var LOG_QUERY = {"page-index" : null,
                     "page-size"  : null,
                     "date-since" : null,
                     "date-until" : null};

    /**
    * @brief A simplistic menu bar wrapper.
    *
    * @param[in] elNav An element that contains the menus.
    * @param[in] selMenu A selector that returns the elements each one of which
    * constitutes a menu within @p elNav. These elements are applied class
    *   @p clsActive to change their appearance as active.
    * @param[in] clsActive The class (with no dot prefixed) that defines a menu
    *   element as active/selected.
    */
    var MAIN_MENU = (function MenuBar(elNav, selMenu, clsActive) {
        /* Pairs of menu names and the element that contains an anchor to that
        * name (see init()). */
        var menuItems;

        /**
        * @brief Initialise references to a menu bar.
        *
        * Populates an internal structure with references to menu elements (as
        * selected with @p selMenu). Each such element is correlated to a menu
        * name which is required when selecting a new menu (see `selectMenu()').
        *
        * Note that the name of each menu is extracted from the `href' attribute
        * of the *first* descendant anchor element. The initial hash sign (if
        * one is present in `href', which should) is omitted. Also note that the
        * names must be unique for each menu within a particular @p elNav
        * because, for each menu name, only one element reference is maintained.
        */
        var init = function() {
            var menus,              // All menus in elNav
                menu,               // A single menu from @c menus
                i,                  // Iterator over @c menus
                hash;               // The `href' of the anchor within @c menu

            menuItems   =  {};      // Remove any previously set menus.
            menus       =  elNav.querySelectorAll(selMenu);

            /* For all menu elements within elNav, fetch their first descendant
            * anchor in order to store the URI fragment of that menu in
            * @c menuItems. The hash sign is omitted. */
            for(i = 0 ; i < menus.length ; ++i) {
                menu            =  menus.item(i);
                hash            =  menu.querySelector("a").getAttribute("href");

                if(hash.indexOf("#") == 0) {
                    hash        =  hash.substr(1);
                }

                menuItems[hash] =  menu;
            }
        };

        /**
        * @brief Apply @p clsActive to the menu that corresponds to @p menu.
        *
        * Any previously selected menu is deselected, even if @p menu is not a
        * registered menu name.
        *
        * @param[in] menu The menu to activate/select. It should not contain the
        * initial hash sigh.
        * @returns @c 1, if a new menu was selected; @c 0 otherwise.
        */
        var selectMenu = function(menu) {
            var curMenu;        // Currently selected menu.

            /* Deselect the current menu, even if the new one is not valid. */
            curMenu =  elNav.querySelector("." + clsActive);
            if(curMenu) {
                /* Remove class @p clsActive. Spaces around the class name are
                * added to avoid replacing a partial match.  */
                curMenu.className = (" " + curMenu.className + " ")
                                    .replace(" " + clsActive + " ", "")
                                    .replace(/^\s+|\s+$/g, "");
            }

            /* Select the new menu, if one with such a name exists. */
            if(menuItems[menu]) {
                menuItems[menu].className += " " + clsActive;
                return 1;
            }
            return 0;
        };

        /* Initialise the menu bar. */
        init(elNav, selMenu, clsActive);

        /* Return access to the public functions of this object. */
        return { "init"         : init,
                 "selectMenu"   : selectMenu
        };

    })(document.getElementById("nav"), "li", "current");

    /**************************************************************************\
    *
    * SECTION Initialisation
    *
    * Initialisation of elements and event listeners.
    *
    \**************************************************************************/

    /* Add listeners to update the page content when the URI fragment is
    * altered. Reloading the state `onhashchange' responds to hash changes due
    * to history traversal (eg, pressing back or forward), whereas `onload'
    * deals with direct visiting (eg, a bookmarked link or page refresh). */
    sfAddEventListener(window, "hashchange", loadState);
    sfAddEventListener(window, "load", loadState);
    sfAddEventListener(document.getElementById("config-save"),
                       "click",
                       handleConfigSave);

    sfAddEventListener(document.getElementById("log-search"),
                       "click",
                       handleLogURI);

    /**************************************************************************\
    *
    * SECTION General functions
    *
    \**************************************************************************/

    /**
    * @brief Updates the main content of the page.
    *
    * The state of the page, and thus the content to load, is determined by the
    * URI fragment (or hash) which is interpreted as a state indicator. This
    * function simply calls the handler specified by the corresponding hash
    * in #STATE_HANDLERS (see handlePage* functions).
    *
    * The URI fragment may contain comma-separated options, each followed by a
    * colon and a value. The current fragment is split into an array of string
    * at every comma. The whole array (including the first string which
    * identifies the 
    */
    function loadState() {
        var hash        =  window.location.hash.split(","),
            handler;

        /* If there is no query fragment, use the default menu (which has `href'
        * set to #home. In any case, the hash sign is removed. */
        if(hash[0] === "") {
            hash[0]     =  "home";
        } else {
            if(hash[0].indexOf("#") == 0) {
                hash[0] =  hash[0].substr(1);
            }
        }

        /* Any unknown hashes will not affect the state of the page. */
        handler         =  STATE_HANDLERS[hash[0]];
        if(handler) {
            MAIN_MENU.selectMenu(hash[0]);
            handler(hash);
        }
    };

    /**
    * @brief Switch visibility between current page and @p idPage.
    *
    * Apply classes "page hidden" to the first element within content (ie, the
    * element with ID set to "content") that possesses classes "page visible".
    * Then, apply class "page visible" to the element with its ID set to
    * @p idPage.
    *
    * @param[in] idPage The id attribute of the element to display. It will be
    *   applied the classes "page visible". Any other classes are removed.
    */
    function switchToPage(idPage) {
        var curPage,
            newPage;
        curPage =  document.getElementById("content")
                  .querySelector(".page.visible");

        if(curPage) {
            curPage.className    = "page hidden";
        }

        newPage =  document.getElementById(idPage);
        if(newPage) {
            newPage.className    = "page visible";
        }
    };

    /**
    * @brief Split each item of @p arrStr into key and value.
    *
    * The array is traversed from the beginning to the end. Each item is split
    * into two parts; one up to the first occurrence of a colon (:); the other,
    * after the colon up to the end. If an item starts with a colon, it is
    * omitted. If an item does not contain a colon, it is included as a key, but
    * is set to @c null.
    *
    * The key-part of each item need not be enclosed in double quotes. The value
    * of the last occurrence of each key is preserved.
    *
    * Currently, only single-level parsing is performed (ie, arrays and objects
    * are passed as string values to their respective key).
    *
    * @param[in] arrStr An array of `key:value' strings.
    * @returns An object with key-value pairs extracted from the array items.
    */
    function objectifyStrings(arrStr) {
        var object  = {},       // The object to return
            colon,              // Index of the first colon in each @p arrStr
            key,                // Key to add
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

    /**
    * @brief Append a message list, sibling to each specified element.
    *
    * Each key in @p objMsg is taken as the id attribute of a document element.
    * For each such element, a new `ul' with its class attribute set to @p cls
    * is created. The @c ul is then appended to the element's direct parent.
    *
    * @param[in] objMsg Object with field ids paired to array of messages.
    */
    function fieldShowMsg(objMsg, cls) {
        var id,         // Id of each field with a message to display
            el,         // The element with its id equal to @c id
            ul;         // The generated list

        for(id in objMsg) {

            /* Create the message container. */
            ul      =  document.createElement("ul");
            ul.innerHTML    =  "<li>" + objMsg[id].join("</li><li>") + "</li>";
            ul.className    =  cls;

            /* Append the message. */
            el      =  document.getElementById(id);
            el.parentNode.appendChild(ul);
        }
    };

    /**
    * @brief Empty field value attribute.
    *
    * This actually sets the value attribute of each specified element to "".
    *
    * @param[in] arrId An array of ids of input to reset.
    */
    function fieldReset(arrId) {
        var i;

        for(i = 0 ; i < arrId.length ; ++i) {
            document.getElementById(arrId[i]).value =  "";
        }
    }

    /**
    * @brief Validate coordinate fields.
    *
    * Field values are updated to match the one parsed.
    *
    * @param[out] error Contains error strings. If any errors occurred during
    *   parsing, they will be inserted into `error[id]' as an array of
    *   strings, where @c id is the first non-null supplied id.
    * @param[in] idX The id attribute of the field containing the X-coordinate.
    *   Optional.
    * @param[in] idY The id attribute of the field containing the Y-coordinate.
    *   Optional.
    * @param[in] idZ The id attribute of the field containing the Z-coordinate.
    *   Optional.
    * @returns An object with @p idX, @p idY and @p idZ as keys (those that are
    *   non-null), each with its corresponding value.
    */
    function fieldsCoordinates(error, idX, idY, idZ) {
        var values  =  {},
            args,           // @p idX, @p idY and @p idZ
            el,             // The field element
            value   =  0,   // Parsed value
            i;

        /* Not using arguments so done manually. */
        args        =  [idX, idY, idZ];

        for(i = 0 ; i < args.length ; ++i) {
            /* Not all coordinates may be required. */
            if(args[i]) {
                el          =  document.getElementById(args[i]);
                number      =  parseInt(el.value, 10);

                if(sfIsNaN(number) || number < 0) {
                    error[args[i]]  =  [MSG.error["xyz-positive"]];
                    break;
                }
                el.value    =  number;
                values[args[i]] =  number;
            }
        }
        return values;
    };

    /**************************************************************************\
    *
    * SECTION Page handlers
    *
    * Callback functions for STATE_HANDLERS and, consequently, `loadState()'.
    * These functions are responsible for responding to user requests that alter
    * the UI (such as clicking on an anchor that switches to different page).
    *
    \**************************************************************************/

    /**
    * @brief Responsible for displaying the home page.
    */
    function handlePageHome() {
        switchToPage("home-page");
    };

    /**
    * @brief Responsible for displaying the configuration page.
    */
    function handlePageConfig() {
        switchToPage("config-page");
    }

    /**
    * @brief Responsible for displaying the manual operation page.
    */
    function handlePageOperate() {
        switchToPage("operate-page");
    }

    /**
    * @brief Responsible for displaying the help page.
    */
    function handlePageHelp() {
        switchToPage("help-page");
    }

    /**************************************************************************\
    *
    * SECTION Other handlers
    *
    * Callbacks that perform general tasks such as form handling.
    *
    \**************************************************************************/

    /**
    * @brief Validate and update the device configuration.
    */
    function handleConfigSave() {
        var errors  = {},
            req     = {},
            date,           // Special-case value to separate day-of-week
            coords;         // Parse coordinates as one field; then, separate.

        /* Clear any previously set field messages. */
        sfRemoveNodes(document.getElementById("config-page"), ".field-msg");

        req["iaddr"]    =  fieldIAddr(errors, "config-iaddr");
        req["subnet"]   =  fieldIAddr(errors, "config-subnet");
        req["gateway"]  =  fieldIAddr(errors, "config-gateway");

        coords          =  fieldsCoordinates(errors, "config-x",
                                                     "config-y",
                                                     "config-z");

        date            =  fieldsDate(errors, "config-date-year",
                                              "config-date-mon",
                                              "config-date-date",
                                              "config-date-hour",
                                              "config-date-min",
                                              "config-date-sec");

        /* Display error messages, if there has been errors. */
        if(!sfIsEmpty(errors)) {
            fieldShowMsg(errors, "field-msg error");

        } else {

            req["x"]    =  coords["config-x"];
            req["y"]    =  coords["config-y"];
            req["z"]    =  coords["config-z"];
            req["date"] =  date.toJSON();
            req["day"]  =  date.getDay();

        }
    };

    /**
    * @brief Validate log search fields and create the new URI fragment.
    *
    * This function alters the `href' attribute of the triggering element (@c
    * this) so that it contains the values of the search fields (granted they
    * are valid). The format of the `href' value is the following: @verbatim
    *#log,index:val-1,size:val-2,since:val-3,until:val-4@endverbatim
    * @c index is always provided and set to @c 1. The remaining parameters are
    * only provided, if they where set by the user.
    *
    * @param[in] evt The event that triggered this function. This function uses
    *   Event.preventDefault() in case of an erroneous field value.
    */
    function handleLogURI(evt) {
        var errors  = {},
            since,              // `date-since' parameter value
            until,              // `date-until' parameter value
            size;               // `page-size' parameter value

        /* Clear any previously set field messages. */
        sfRemoveNodes(document.getElementById("log-page"), ".field-msg");

        /* Ignore and reset date fields, if neither month nor year has been
        * set. */
        if(document.getElementById("log-since-year").value
        && document.getElementById("log-since-mon").value) {

            since   =  fieldsDate(errors, "log-since-year",
                                          "log-since-mon",
                                          "log-since-date",
                                          "log-since-hour",
                                          "log-since-min",
                                          "log-since-sec");
        } else {
            /* Reset fields. */
            fieldReset(["log-since-year", "log-since-mon", "log-since-date",
                        "log-since-hour", "log-since-min", "log-since-sec"]);
        }

        if(document.getElementById("log-until-year").value
        && document.getElementById("log-until-mon").value) {

            until   =  fieldsDate(errors, "log-until-year",
                                          "log-until-mon",
                                          "log-until-date",
                                          "log-until-hour",
                                          "log-until-min",
                                          "log-until-sec");
        } else {
            /* Reset fields. */
            fieldReset(["log-until-year", "log-until-mon", "log-until-date",
                        "log-until-hour", "log-until-min", "log-until-sec"]);
        }

        /* Not imposing a page size will return all the available records. */
        if(document.getElementById("log-page-size").value) {
            size        =  fieldInt(errors, "log-page-size", 0, 255);
        }

        /* Display error messages and do not allow the page to update, if there
        * have been errors. */
        if(!sfIsEmpty(errors)) {
            fieldShowMsg(errors, "field-msg error");
            evt.preventDefault();

        } else {
            /* Change the `href' of this anchor to contain the new search
            * parameters. Note, this function is called as a result of
            * activating an anchor, yet its link has not yet been resolved. */
            this.href   =  "#log,index:1";
            if(size)    this.href  +=  ",size:" + size;
            if(since)   this.href  +=  ",since:" + since.toJSON();
            if(until)   this.href  +=  ",until:" + since.toJSON();

            /* The rest will be taken care of by `loadState()' which will be
            * triggered now that the URI has been altered. */
        }
    };

    /**************************************************************************\
    *
    * SECTION Safe functions
    *
    * Functions that attempt to provide some compatibility layer with older
    * browsers. `sf' prefix stands for 'safe'.
    *
    \**************************************************************************/

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
    };

    /**
    * @brief Return an object that can be used to make HTTP request objects.
    *
    * If instantiation of XMLHttpRequest is not possible, Msxml2.XMLHTTP and
    * Microsoft.XMLHTTP will be attempted (in that order). If none of these
    * works, @c null is returned.
    *
    * @returns An XML HTTP object or @c null.
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
    };

    /**
    * @brief Check whether @p value is NaN.
    *
    * This deals with the issue of isNaN() returning @c false even for
    * non-number values.
    *
    * @param[in] value The value to check.
    * @returns @c true if @p value is NaN; @p false, otherwise.
    */
    function sfIsNaN(value) {
        if(typeof value !== 'number') return false;
        return isNaN(value);
    };

    /**
    * @brief Check whether @p collection is empty.
    *
    * @param[in] collection The collection to check.
    * @returns @c true if @p collection is empty or @c null; @p false,
    *   otherwise.
    */
    function sfIsEmpty(collection) {
        var i   =  0,
            val;        // A key.

        for(val in collection) {
            ++i;
            break;
        }
        return i === 0;
    };

    /**
    * @brief Convert a number into a string of fixed amount of digits.
    *
    * @param[in] num The number to prefix with zeros.
    * @param[in] digits The number of total digits to produce.
    * @returns A string number with that many @p digits.
    */
    function sfFixInt(num, digits) {
        var value   =  num.toString(10),
            times   =  digits - value.length,
            i;

        for(i = 0 ; i < times ; ++i) {
            value = "0" + value;
        }
        return value;
    };

    /**
    * @brief Remove the specified @p selNodes from @p elParent.
    *
    * @param[in] elParent An element which will be queried for matching
    *   children. The nodes to remove need not be immediate descendants of this
    *   element.
    * @param[in] selNodes A selector to match the nodes to be removed.
    */
    function sfRemoveNodes(elParent, selNodes) {
        var nodes,      // All nodes returned with @p selNodes
            el,         // A single element from @c nodes
            parent,     // Parent of @c el
            i;

        nodes   =  elParent.querySelectorAll(selNodes);
        for(i = 0 ; i < nodes.length ; ++i) {
            el      =  nodes.item(i);

            /* Since `Node.removeChild()' is not yet available, do this
            * manually. */
            parent  =  el.parentNode.removeChild(el);
        }
    };
})();
