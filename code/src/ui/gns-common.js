;(function(ns) {

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

}(window.gNS = window.gNS || {}));
