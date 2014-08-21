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

    /**
    * @brief A simplistic menu bar wrapper.
    *
    * @param[in] elNav An element that contains the menus.
    * @param[in] selMenu A selector that returns the elements each one of which
    * constitutes a menu within `elNav'. These elements are applied class
    *   `clsActive' to change their appearance as active.
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
        * selected with `selMenu'). Each such element is correlated to a menu
        * name which is required when selecting a new menu (see `selectMenu()').
        *
        * Note that the name of each menu is extracted from the `href' attribute
        * of the *first* descendant anchor element. The initial hash sign (if
        * one is present in `href', which should) is omitted. Also note that the
        * names must be unique for each menu within a particular `elNav'
        * because, for each menu name, only one element reference is maintained.
        */
        var init = function() {
            var menus,              // All menus in elNav
                menu,               // A single menu from `menus'
                i,                  // Iterator over `menus'
                hash;               // The `href' of the anchor within `menu'

            menuItems   =  {};      // Remove any previously set menus.
            menus       =  elNav.querySelectorAll(selMenu);

            /* For all menu elements within elNav, fetch their first descendant
            * anchor in order to store the URI fragment of that menu in
            * `menuItems'. The hash sign is omitted. */
            for(i = 0 ; i < menus.length ; ++i) {
                menu            =  menus[i];
                hash            =  menu.querySelector("a").getAttribute("href");

                if(hash.indexOf("#") == 0) {
                    hash        =  hash.substr(1);
                }

                menuItems[hash] =  menu;
            }
        };

        /**
        * @brief Apply `clsActive' to the menu that corresponds to `menu'.
        *
        * Any previously selected menu is deselected, even if `menu' is not a
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
                /* Remove class `clsActive'. Spaces around the class name are
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
    * in #STATE_HANDLERS (see handle* functions).
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
    * @brief Switch visibility between current page and `idPage'
    *
    * Apply classes "page hidden" to the first element within content (ie, the
    * element with ID set to "content") that possesses classes "page visible".
    * Then, apply class "page visible" to the element with its ID set to
    * `idPage'.
    *
    * @param[in] idPage The ID attribute of the element to display. It will be
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
    }

    /**
    * @brief Validate an IP address field.
    *
    * @param[out] error Contains error strings. If any errors occurred during
    *   parsing, they will be inserted into `error[idIAddr]' as an array of
    *   strings.
    * @param[in] idIAddr The id attribute of the input field to parse.
    * @returns A valid IP address string (as it was specified in the input
    *   field, after removing insignificant zeros); @c null, on error.
    */
    function fieldIAddr(error, idIAddr) {
        var el,             // The field element.
            segments,       // An array of IP segments (4 numbers).
            number,         // One of `segments'.
            value   =  0,   // Parsed value.
            i       =  0;

        el          =  document.getElementById(idIAddr);
        segments    =  el.value.split(".");
        if(segments.length !== 4) {
            error[idIAddr] = [MSG.error.iaddr4];

        } else {

            /* Convert each segment into a number to validate it. */
            for(i = 0 ; i < segments.length ; ++i)  {

                number = parseInt(segments[i], 10);
                if(number < 0 || number > 255 || sfIsNaN(number)) {
                    error[idIAddr]  =  [MSG.error.iaddr4];
                    break;
                }

                /* Join the numbers anew to omit any insignificant zeros. */
                value  +=  number;
                if(i < 3) {
                    value += ".";
                }
            }
        }

        if(i === 4) {
            el.value    =  value;
            return         value;
        }
        return null;
    };

    /**
    * @brief Validate a date-time spread over multiple fields.
    *
    * Granted a valid date-time, the fields' value will be updated to the one
    * returned to avoid, for instance, insignificant zeros or date overflow.
    *
    * @param[out] error Contains error strings. If any errors occurred during
    *   parsing, they will be inserted into `error[idYear]'.
    * @param[in] idYear The id attribute of field containing the year.
    * @param[in] idMonth The id attribute of field containing the month.
    * @param[in] idDate The id attribute of field containing the date.
    * @param[in] idHours The id attribute of field containing the hours.
    * @param[in] idMinuntes The id attribute of field containing the minutes.
    * @param[in] idSecondes The id attribute of field containing the seconds.
    * @returns A Date instance, if the values where valid; @c null, on error.
    */
    function fieldsDate(error,
                        idYear,
                        idMonth,
                        idDate,
                        idHours,
                        idMinutes, 
                        idSeconds) {

        var fYear   =  document.getElementById(idYear),
            fMon    =  document.getElementById(idMonth),
            fDate   =  document.getElementById(idDate),
            fHour   =  document.getElementById(idHours),
            fMin    =  document.getElementById(idMinutes),
            fSec    =  document.getElementById(idSeconds),
            month   =  fMon.value,
            dt;

        /* Month is zero-based. */
        if(!sfIsNaN(month) && month > 0) --month;

        /* The device operates in UTC. Assume the values inserted are UTC as
        * well. */
        dt          =  new Date(Date.UTC(fYear.value,
                                         month,
                                         fDate.value,
                                         fHour.value,
                                         fMin.value,
                                         fSec.value));

        /* Check validity of date. For instance, if non-digits where inserted,
        * that would have caused an invalid date. */
        if(sfIsNaN(dt.getTime())) {
            error[idYear]  =  [MSG.error["date-invalid"]];

        } else {
            /* Update field values. Note that the device assumes UTC time. */
            fYear.value =  dt.getUTCFullYear();
            fMon.value  =  sfFixInt(dt.getUTCMonth() + 1, 2);
            fDate.value =  sfFixInt(dt.getUTCDate(), 2);
            fHour.value =  sfFixInt(dt.getUTCHours(),  2);
            fMin.value  =  sfFixInt(dt.getUTCMinutes(), 2);
            fSec.value  =  sfFixInt(dt.getUTCSeconds(), 2);

            return dt;
        }
        return null;
    };

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

    /**
    * @brief Validate an integer field.
    *
    * The field value is updated to match the one parsed.
    *
    * @param[out] error Contains error strings. If any errors occurred during
    *   parsing, they will be inserted into `error[idInt]' as an array of
    *   strings.
    * @param[in] mn The minimum permissive number. Optional.
    * @param[in] mx The maximum permissive number. Optional.
    * @returns A valid integer (in the specified range); @c null, on error.
    */
    function fieldInt(error, idInt, mn, mx) {
        var el,             // The field element
            number,         // The value of the element
            value   =  0;   // Parsed value

        /* Construct the appropriate error message. */
        var msg     =  MSG.error["not-int"];
        var msgTail =  "";

        if(mn && mx) {
            msgTail = " (από " + mn + " μέχρι " + mx + ").";

        } else if(mn) {

            msgTail = " (από " + mn + " και πάνω).";
        } else if(mx) {

            msgTail = " (μέχρι " + mx + ").";
        }
        if(msgTail.length) {

            /* Remove last period (if there is one) and append @c msgTail. */
            if(msg.charAt(msg.length - 1) === '.') {
                msg =  msg.substring(0, msg.length - 1);
            }
            msg    +=  msgTail;
        }

        /* Validate field value. */
        el          =  document.getElementById(idInt);
        number      =  parseInt(el.value, 10);
        if(sfIsNaN(number) || mn > number || mx < number) {
            error[idInt]    =  [msg];
            return null;
        }

        return number;
    };

    /**************************************************************************\
    *
    * SECTION Handlers
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
    * @brief Responsible for displaying the log page.
    *
    * @brief[in] p The parameters specified within the URI fragment.
    */
    function handlePageLog(p) {
        var req     =  sfCreateRequest();

        if(req) {
            switchToPage("log-page");
        }
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
})();
