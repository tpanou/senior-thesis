;(function(ns) {

    /**
    * @brief Allows to select one among a collection of peers.
    *
    * Calling switchTo() with a key found in @p arrKeys, results in applying
    * class @p clsActive to the element @p arrEls in the corresponding index.
    * Any previously selected element is automatically applied @p clsInactive.
    * At most one element may be selected at a time.
    *
    * @param[in] arrKeys Strings to identify each element in @arrEls (on a
    *   one-on-one basis).
    * @param[in] arrEls Elements to be applied classes @p clsActive and @p
    *   clsInactive.
    * @param[in] clsActive
    * @param[in] clsInactive
    * @param[in] keyDefault Key of the element to select upon instantiation.
    * Optional.
    */
    ns.OneOfMany =
    function (arrKeys, arrEls, clsActive, clsInactive, keyDefault) {
        this.one        =  null;
        this.many       =  {};
        this.active     =  clsActive;
        this.inactive   =  clsInactive;

        var i,              // Index of @p arrKeys-arrEls pair
            el;             // An element from @p arrEls

        for(i = 0 ; i < arrKeys.length ; ++i) {
            el                      =  arrEls[i];
            el.className            =  clsInactive;
            this.many[arrKeys[i]]   =  el;
        }

        if(keyDefault !== undefined) {
            this.switchTo(keyDefault);
        }
    };
    ns.OneOfMany.prototype = {

        /**
        * @brief Selects the element that is paired with @p key.
        *
        * The current element (if any) is set to this.inactive regardless of
        * whether @p key corresponds to an available key.
        *
        * @param[in] key The key for the element to select. If @c null, none
        *   will be selected.
        */
        switchTo : function(key) {
            if(this.one !== null) {
                this.one.className  =  this.inactive;
                this.one            =  null;
            }

            if(this.many[key]) {
                this.one            =  this.many[key];
                this.one.className  =  this.active;
            }
        }
    };

    /**
    * @brief Facilitates manaing pagination controls.
    *
    * Every instance keeps track of an element that displays the number results
    * and a second element for page anchors (pagination). Upon invoking show(),
    * the `innerHTML' of those elements is updated accordingly.
    *
    * The link (`href') of each page anchor is generated by an external
    * function,@p fnLinker. @p clsCurrent is a class string to apply to the page
    * anchor dubbed as current (or selected), while @p clsOther is applied to
    * every other.
    *
    * @param[in] elTotal Element which innerHTML will be set to the amount of
    *   search results.
    * @param[in] elPages Element which innerHTML will be populated with a number
    *   of anchors.
    * @param[in] clsCurrent The currently displayed anchor page will be applied
    *   this class.
    * @param[in] clsOther All anchors, except for the selected one, will be
    *   applied this class.
    * @param[in] fnLinker A function that accepts a page number/index (starting
    *   at @c 1) and returns an appropriate value for its corresponding anchor's
    *   `href' attribute.
    */
    ns.Paginator =
    function (elTotal, elPages, clsCurrent, clsOther, fnLinker) {
        this.elTotal    =  elTotal;
        this.elPages    =  elPages;
        this.clsCurrent =  clsCurrent;
        this.clsOther   =  clsOther;
        this.linker     =  fnLinker;
    };
    ns.Paginator.prototype = {

        /**
        * @brief Update the pagination display.
        *
        * @param[in] results The amount of total results.
        * @param[in] anchors The amount of page links to generate.
        * @param[in] current The index of the currently displayed page (starts
        *   at @c 1). This page will not have its `href' set, whereas its class
        *   will be set to clsCurrent unlike the other anchors that acquire
        *   clsOther.
        */
        show : function(results, anchors, current) {
            var i,
                limit,
                pages   =  "";

            this.elTotal.innerHTML  =  results;
            this.elPages.innerHTML  =  "";

            i       =  1;
            limit   =  current;

            while(i <= anchors) {
                for(i ; i < limit ; ++i) {
                    pages  +=  "<a class=\"" + this.clsOther  + "\" "
                           +      "href=\""  + this.linker(i) + "\">"
                           +   i + "</a>";
                }

                if(i === current) {
                    /* Set the class of the current anchor but do not provide
                    * a link. */
                    pages  +=  "<a class=\"" + this.clsCurrent + "\">"
                           +   i + "</a>";
                    ++i;
                    limit   =  anchors + 1;
                }
            }

            if(pages === "") pages +=  " 0";
            this.elPages.innerHTML  =  pages;
        },

        /**
        * @brief Set totals @c 0 and page anchors to a single &ndash;.
        */
        reset : function() {
            this.elTotal.value      =  0;
            this.elPages.innerHTML  =  "&ndash;";
        }
    };

    /**
    * @brief Expand @p child's prototype with @p parent's
    *
    * Prototype members than exist on both @p child and @p parent are ignored,
    * by default, unless @p override evaluates to @c true.
    *
    * @param[in,out] child Contains the prototype to be expanded.
    * @param[in,out] parent Contains the prototype to be copied.
    * @param[in] override Replace common prototype members with those of @p
    *   parent.
    */
    ns.augment = ns.augment ||
    function (child, parent, override) {
        var i;

        for(i in parent.prototype) {
            if(child.prototype[i] !== undefined && !override) continue;
            child.prototype[i] = parent.prototype[i];
        }
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
    ns.addEventListener = ns.addEventListener ||
    function (el, event, fn) {
        if(el.addEventListener) {
            el.addEventListener(event, fn);

        /* This should suffice for IE versions prior to 9. */
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
    * Microsoft.XMLHTTP will be attempted in that order. If none of these
    * works, @c null is returned.
    *
    * @returns An XML HTTP object or @c null.
    */
    ns.createRequest = ns.createRequest ||
    function () {
        var request = null;

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
    ns.isNaN = ns.isNaN ||
    function (value) {
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
    ns.isEmpty = ns.isEmpty ||
    function (collection) {
        var i   =  0,
            val;

        for(val in collection) {
            ++i;
            break;
        }
        return i === 0;
    };

    /**
    * @brief Convert a number into a string of fixed digits.
    *
    * @param[in] num The number to prefix with zeros.
    * @param[in] digits The number of total digits to produce.
    * @returns A string number with that many @p digits.
    */
    ns.fixInt = ns.fixInt ||
    function (num, digits) {
        var value   =  num.toString(10),
            times   =  digits - value.length,
            i;

        for(i = 0 ; i < times ; ++i) {
            value = "0" + value;
        }
        return value;
    };

    /**
    * @brief Facilitate validation and access to input fields.
    *
    * A Field is linked to some input elements. It may then be used to validate
    * and return their value displaying an appropriate error message, if
    * necessary, empty them or validate candidate values without actually
    * setting them.
    *
    * This is not a complete definition and may not be instantiated. It should
    * be augmented to provide specific functionality. Any of the provided
    * members should be replaced to that end.
    */
    ns.Field =
    function () {
        throw "Error: Instantiating Field.";
    };
    ns.Field.prototype = {

        /**
        * @brief The id of the underlying input field(s).
        *
        * This is used to gain a reference to the actual input field(s) as well
        * as an indicator of the error message (see _showErrors()).
        */
        id : null,

        /**
        * @brief A reference to the underlying element(s).
        *
        * Field is defined to support a single input element, though it may be
        * augmented to support multiple fields, as well.
        */
        el : null,

        /**
        * @brief Reference to the generated error element.
        */
        elMsg : null,

        /**
        * @brief Class string to apply to the element containing the error.
        *
        * See _showErrors().
        */
        clsError: null,

        /**
        * @brief Set the value of this Field.
        *
        * The value only updates the underlying input fields, if it passes the
        * validation of validate().
        *
        * @param[in] value The value to set to the Field.
        * @returns The value returned by validate().
        */
        set : function(value) {
            value = this.validate(value);
            if(value !== null) {
                this.el.value   =  value;
            }
            return value;
        },

        /**
        * @brief Parse the current value of the Field.
        *
        * @param[in, out] error Object. Designates that, if validation fails,
        *   an error message should be added to the DOM. This object is added
        *   a new member named after this.id. Its value is an array of error
        *   strings. The generated DOM element (containing the error message)
        *   may be accessed via this.elMsg. Do note that the element is
        *   automatically appended to the DOM (see, _showErrors()). Optional.
        * @returns A valid value parsed form the underlying input fields or @c
        * null.
        */
        get : function(errors) {
            var value   =  this.set(this.el.value);

            if(value === null && typeof errors === "object") {
                errors[this.id] = this._showErrors();
            }

            return value;
        },

        /**
        * @brief Clear the value of the input field(s).
        */
        reset : function() {
            this.el.value   =  "";
            return this;
        },

        /**
        * @brief Validate the supplied value and return it or return @c null.
        *
        * This function accepts a value to test against the constraints of this
        * Field.
        *
        * @return The validated value, which may not be exactly the same as
        *   @p value (for instance, trimmed of leading and trailing spaces).
        */
        validate : null,

        /**
        * @brief Remove the error message from the DOM.
        */
        resetMsg : function() {
            if(this.elMsg) {
                this.elMsg.parentNode.removeChild(this.elMsg);
                delete this.elMsg;
            }
            return this;
        },

        /**
        * @brief Class string applied to the generated error elements.
        */
        setErrorClass : function(clsError) {
            this.clsError   =  clsError;
            return this;
        },

        /**
        * @brief Construct an appropriate error message for this Field.
        *
        * @returns An array of error strings.
        */
        _getErrors : null,

        /**
        * @brief Creates and appends an error element to the DOM.
        *
        * @returns The generated element.
        */
        _showErrors : function() {
            var ul,
                errors = this._getErrors();

            /* Set error message, if validation fails. */
            ul              =  document.createElement("ul");
            ul.innerHTML    =  "<li>"
                            +  errors.join("</li><li>")
                            +  "</li>";

            this.clsError && (ul.className = this.clsError);

            /* Append the message. */
            this.el.parentNode.appendChild(ul);
            this.resetMsg();
            this.elMsg = ul;
            return ul;
        }
    };

    /**
    * @brief Track an integer Field.
    *
    * Since a FieldInt requires a single input field, most of the core
    * functionality remains intact. Only validate() and _getErrors() are
    * implemented.
    *
    * @param[in] idInt id of the input element.
    * @param[in] min The minimum allowable value. The validation and the
    *   generated error message are affected by this value. Optional.
    * @param[in] max The maximum allowable value. The validation and the
    *   generated error message are affected by this value. Optional.
    */
    ns.FieldInt =
    function(idInt, min, max) {
        this.id     =  idInt;
        this.el     =  document.getElementById(idInt);
        this.min    =  min;
        this.max    =  max;
    };
    ns.FieldInt.prototype = {

        /**
        * @brief Parse @p value as an integer within this.min, this.max.
        *
        * If this.min or this.max are @c undefined, they do not affect the
        * validation.
        *
        * @returns A valid integer (within this.min and this.max); @c null, on
        *   error.
        */
        validate : function(value) {
            value   =  parseInt(value, 10);

            if(ns.isNaN(value)
            || (this.min !== undefined && this.min > value)
            || (this.max !== undefined && this.max < value))
                  return null;
            return value;
        },

        /**
        * @brief Constructs an error message for this Field's constraints.
        *
        * `FieldInt' only returns a single string (wrapped in an array) adjusted
        * to the value of this.min and this.max of an instance at the time of
        * its invocation.
        *
        * @returns An array of string errors.
        */
        _getErrors : function() {
            /* Construct the appropriate error message. */
            var msg     =  "";

            /* Construct the tail of the error *without* the period. */
            if(this.min !== undefined && this.max !== undefined) {
                msg = " από " + this.min + " μέχρι " + this.max;

            } else if(this.min !== undefined) {
                msg = " από " + this.min + " και πάνω";

            } else if(this.max !== undefined) {
                msg = " μέχρι " + this.max;
            }

            /* Combine the constant part, the tail (if any) and the period.*/
            msg =  "Απαιτείται ακέραιος" + msg + ".";
            return [msg];
        }
    };
    ns.augment(ns.FieldInt, ns.Field);

}(window.gNS = window.gNS || {}));
