;(function(ns) {

    /**
    * @brief Responsible for rendering the page of Log results.
    *
    * Functions of immediate interest are init(), reload(), submit() and
    * reset(). init() should be called first, before attempting to operate this.
    */
    ns.PageLog = ns.PageLog || (function() {

        /* These must *all* be supplied via init(). */
        var hash,           // Used to construct URI fragments with createURI()
            fSize,
            fSince,
            fUntil,
            pages,          // Paginator variable
            elTable,        // Section to put data `dt's
            elSection,      // This element is hidden when first reloading
            clsHidden;

        var request;        // XMLHTTPRequest variable
        var objParams;      // An object with valid parameters extracted from
                            // the URI with parseParams().

        request =  ns.createRequest();
        if(!request) throw "PageLog: Could not instantiate XMLHTTPRequest.";

        /**
        * @brief Initialise this instance.
        *
        * @param[in] s Object with initialisation settings. It should contain
        *   the following keys, each with a value as follows:@verbatim {
        * hash                  URI fragment of this Page. All links generated
        *                       will be prefixed with this value (so it should
        *                       contain #)
        * form.idSize           id of page-size input field
        *     .idSince          id prefix (see FieldDateGroup()) for date-since
        *     .idUntil          id prefix (as above) for date-until
        *     .clsError         class of `ul's when displaying field errors
        * data.elTotal          See first 4 parameters of Paginator()
        *     .elPages          Same as above
        *     .clsCurrent       Same as above
        *     .clsOther         Same as above
        *     .clsHidden        Class string to hide element
        *     .idSection        id of element to apply clsHidden whenever
        *                       reloading the default (empty) page.
        *     .idTable          id of the element to accept `td's of log data
        *}@endverbatim
        */
        var init = function(s) {
            var cls =  s.form.clsError;

            hash    =  s.hash;
            fSize   =  (new ns.FieldInt(s.form.idSize, 0, 255))
                              .setErrorClass(cls),
            fSince  =  (new ns.FieldDateGroup(s.form.idSince))
                              .setErrorClass(cls),
            fUntil  =  (new ns.FieldDateGroup(s.form.idUntil))
                              .setErrorClass(cls);

            pages   =  new ns.Paginator(s.data.elTotal,
                                        s.data.elPages,
                                        s.data.clsCurrent,
                                        s.data.clsOther,
                                        createURI);

            elSection = document.getElementById(s.data.idSection);
            clsHidden = s.data.clsHidden;

            elTable =  document.getElementById(s.data.idTable);

        };

        /**
        * @brief Update the view of the Page.
        *
        * The default behaviour is to maintain the state of the page and not
        * clearing everything out to pre-set values.
        *
        * Note that, unlike other Page functions, this one accepts an object
        * of search parameters and not an Event source.
        *
        * @brief[in] params Object containing the parameters specified within
        *   the URI fragment.
        */
        var reload = function(params) {

            var p,              // URI parameters parsed and validated
                query;          // Query string to submit


            /* Do not query the server unless there are parameters set. */
            if(ns.isEmpty(params)) {

                /* In case the page is loaded without a valid table state (as
                * indicated by not having any request parameters set), hide the
                * result section. */
                if(objParams === undefined) elSection.className = clsHidden;
                return;

            } else {
                p   =  parseParams(params);
            }

            /* Clear any previously set field messages. */
            resetMsg();

            /* Build the query string. Note that in the UI, @c index starts at 1
            * whereas, in the back-end, @c page-index is zero-based. */
            query   =  "?page-index=" + (p["index"] ? p["index"] - 1 : 0);
            if(p.size || p.size === 0)   query += "&page-size=" + p.size;
            p.since && (query += "&date-since=" + p.since);
            p.until && (query += "&date-until=" + p.until);

            /* Keep a copy of the search values in a private member for
            * createURI() to use. */
            objParams =  p;

            /* Submit the request. */
            request.open("GET", "measurement.php" + query);
            request.onreadystatechange = async;
            request.send();
        };

        /**
        * @brief Validate log search fields and create the new URI fragment.
        *
        * This function alters the `href' attribute of the triggering element
        * (@c this) so that it contains the values of the search fields (granted
        * they are valid). The format of the `href' value is the following:
        *@verbatim
        *#log,index:val-1,size:val-2,since:val-3,until:val-4@endverbatim
        * @c index is always provided and set to @c 1. The remaining parameters
        * are only provided, if they where set by the user.
        *
        * @param[in] evt The event that triggered this function. This function
        *   uses Event.preventDefault() in case of an erroneous field value.
        *   Otherwise, the `href' attribute of the source is modified to a
        *   fragment that contains the specified search parameters, right before
        *   the browser loads the link.
        */
        var submit = function(evt) {
            var errors  = {},
                since,              // Field value
                until,              // Field parameter value
                size;               // Field value

            /* Clear any previously set field messages. */
            resetMsg();

            /* Ignore and reset date fields, if year has been not been set. */
            fSince.elYear.value ? since = fSince.get(errors) : fSince.reset();
            fUntil.elYear.value ? until = fUntil.get(errors) : fUntil.reset();
            /* fSize should accept @c 0 */
            fSize.el.value !== "" && (size = fSize.get(errors));

            /* Display error messages and do not allow the page to update, if
            * there have been errors. */
            if(!ns.isEmpty(errors)) {
                evt.preventDefault();

            } else {

                /* Change the `href' of this anchor to contain the new search
                * parameters. Note, this function is called as a result of
                * activating an anchor, yet its link has not yet been
                * resolved. */
                this.href   =  hash  +   ",index:1";
                if(size || size === 0) this.href   +=  ",size:" + size;
                since && (this.href  +=  ",since:" + since.toJSON());
                until && (this.href  +=  ",until:" + until.toJSON());

                /* The rest will be taken care of by `loadState()' which will be
                * triggered now that the URI has been altered. */
            }
        };

        /**
        * @brief Reset all input Fields (including error messages).
        *
        * @param[in] evt If instance of Event, .preventDefault() will be
        *   invoked. Optional.
        */
        var reset = function(evt) {
            evt instanceof Event && evt.preventDefault();

            resetMsg();
            fSize.reset();
            fSince.reset();
            fUntil.reset();
        }

        /**
        * @brief Create a URI fragment with the supplied @p index and the
        * current request parameters.
        *
        * @param[in] index The index value to set to the constructed URI
        *   fragment.
        * @returns A string that can be used to request the @c index page of
        *   the current request parameters.
        */
        var createURI = function(index) {
            var href = hash + ",index:" + index;
            objParams.size === undefined || (href +=
                                                ",size:"  + objParams.size);

            objParams.since === undefined || (href +=
                                                ",since:" + objParams.since);

            objParams.until === undefined || (href +=
                                                ",until:" + objParams.until);
            return href;
        };

        /**
        * @brief Parse the record set into the result table.
        *
        * @param[in] set A valid response containing a record set.
        */
        var parseResults = function(set) {
            var recTotal    =  set["total"],
                pageSize    =  set["page-size"],
                pageIndex   =  set["page-index"],
                pageTotal,
                inc         =  pageSize * pageIndex + 1,
                rec,                        // A record
                dt,                         // The date of a record
                i;

            console.log("parsing", set);

            /* Calculate total pages (links) and current result page index. */
            pageTotal   =  pageSize === 0 ? 0 : Math.ceil(recTotal/pageSize);
            pageIndex  >=  pageTotal && (pageIndex = pageTotal);
            pages.show(recTotal, pageTotal, pageIndex + 1);

            elTable.innerHTML   =  "";

            for(i = 0 ; i < set["log"].length ; ++i) {
                rec     =  set["log"][i];

                if(!rec) continue;
                dt      =  new Date(rec["date"]);

                elTable.innerHTML +=  "<tr>"
                                  +   "<td>" + inc++            + "</td>"
                                  +   "<td>" + dt.toUTCString() + "</td>"
                                  +   "<td>" + rec["x"]         + "</td>"
                                  +   "<td>" + rec["y"]         + "</td>"
                                  +   "<td>" + rec["t"]         + "</td>"
                                  +   "<td>" + "&ndash;"        + "</td>"
                                  +   "<td>" + "&ndash;"        + "</td>"
                                  +   "</tr>";
            }
            elSection.className = "";
        };

        /**
        * @brief Called via onreadystatechange. Handles asynchronous responses.
        */
        var async = function() {

            if(this.readyState !== 4) return;

            if(this.status === 200) parseResults(JSON.parse(this.responseText));
        };

        /**
        * @brief Load the supplied object of values into the form Fields.
        *
        * Checks if any of the expected parameters have been set and whether
        * their value is acceptable by its intended field. Otherwise, it resets
        * the contents of that field and removes (with @c delete) that parameter
        * from @p.
        *
        * @param[in,out] p An object of parameter-value pairs to update the
        *   Fields with. @p p.index is always set.
        * @returns The modified @p p.
        */
        var parseParams = function(p) {
            var value;      // Any parsed value

            /* Parameter @c index is an exceptional case; it is wrapped by a
            * Field(). It suffices, though, if it is a positive number the
            * server can parse, something that, more or less, applies to @c
            * size. So, borrow the constraints of that Field to validate the
            * supplied value of index, if any. The loose equality with @c null
            * ensures that a value of @c 0 is ignored. */
            if((value = p.index) === undefined
            || (value = fSize.validate(value)) == null) {
                p.index  =  1;
            } else {
                p.index =  value;
            }

            if((value = p.size) === undefined
            || (value = fSize.set(value)) === null) {
                fSize.reset();
                delete p.size;
            } else {
                p.size  =  value;
            }

            if((value = p.since) === undefined
            || (value = fSince.set(new Date(value))) === null) {
                fSince.reset();
                delete p.since;
            } else {
                p.since =  value.toJSON();
            }

            if((value = p.until) === undefined
            || (value = fUntil.set(new Date(value))) === null) {
                fUntil.reset();
                delete p.until;
            } else {
                p.until =  value.toJSON();
            }

            return p;
        };

        /**
        * @brief Request that all Fields remove their error messages.
        */
        var resetMsg = function() {
            fSize.resetMsg();
            fSince.resetMsg();
            fUntil.resetMsg();
        };

        /* Advertise privileged functions. */
        return {"init"      : init,
                "reload"    : reload,
                "submit"    : submit,
                "reset"     : reset};
    })();
})(window.gNS = window.gNS || {});
