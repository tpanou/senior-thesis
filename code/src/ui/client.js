(function() {

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
