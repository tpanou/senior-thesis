(function() {

    /* An object of key-value pairs of URL fragments and their corresponding
    * handler function that updates the page as required. Note that the hash
    * sign must be provided as part of all keys. An exception to this is the
    * empty string which is matched against no hash at all and hash with no
    * succeeding text. */
    /* TODO: Verify that this last bit is the default behaviour of browsers
    * other than FF. */
    var STATE_HANDLERS = {
        ""                  : handleIndex,
        "#log"              : handleShowLog,
        "#configuration"    : handleIndex
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
        /* Pairs of a hash and the element that contains an anchor to that hash
        * (see init()). */
        var menuItems;

        /**
        * @brief Initialise references to a menu bar.
        *
        * Populates an internal structure with references to menu elements (as
        * selected with `selMenu'). Each such element is correlated to a URL
        * fragment which is also required to activate a menu over another (see
        * `selectMenu()').
        *
        * Note that the URL fragment is extracted from the `href' attribute of
        * the *first* anchor element descendant of each menu. Also note that the
        * URL fragments must be unique for each menu within a particular
        * `elNav'.
        */
        var init = function() {
            var menus,              // All menus in elNav
                menu,               // A single menu from `menus'
                i,                  // Iterator over `menus'
                hash;               // The `href' of the anchor within `menu'

            menuItems   =  {};      // Remove any previously set menus.
            menus       =  elNav.querySelectorAll(selMenu);

            /* For all menu elements within elNav, fetch their first descendant
            * anchor in order to store the URL fragment of that menu in
            * `menuItems'. */
            for(i = 0 ; i < menus.length ; ++i) {
                menu            =  menus[i];
                hash            =  menu.querySelector("a").getAttribute("href");
                menuItems[hash] =  menu;
            }
        };

        /**
        * @brief Apply `clsActive' to the menu that corresponds to `hash'.
        *
        * 
        * Any previously selected menu is always deselected, even if `hash' does
        * not identify a particular menu.
        *
        * @param[in] hash The menu to activate/select.
        * @returns @c 1, if a new menu was selected; @c 0 otherwise.
        */
        var selectMenu = function(hash) {
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

            /* Select the new menu, if one with such a hash exists. */
            if(menuItems[hash]) {
                menuItems[hash].className += " " + clsActive;
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
    * URL fragment (or hash) which is interpreted as a state indicator. This
    * function simply calls the handler specified for the corresponding hash
    * in #STATE_HANDLERS (see handle* functions). */
    function loadState() {
        var hash        =  window.location.hash,
            handler     =  STATE_HANDLERS[hash];

        /* If there is no query fragment, use the default menu (which has `href'
        * set to #. */
        if(hash === "") hash = "#";

        /* Any unknown hashes will not affect the state of the page. */
        if(handler) {
            MAIN_MENU.selectMenu(hash);
            handler();
        }
    };

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
