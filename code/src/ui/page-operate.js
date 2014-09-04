;(function (ns) {

    /**
    * @brief Responsible for operating the device manually.
    */
    ns.PageOperate = ns.PageOperate || (function () {

        /* These must *all* be supplied via init(). */
        var elStatus,
            elRange,
            fCoords;

        /**
        * @brief Initialise this instance.
        *
        * @param[in] s Object with initialisation settings. It should contain
        *   the following keys, each with a value as follows:@verbatim {
        * form.idNewX           id of field for the new X coordinate of device
        *     .idNewY           id of field for the new Y coordinate of device
        *     .clsError         class of `ul's when displaying field errors
        * data.idStatus         id of element to display current device state
        *     .idRange          id of element to display the operational range
        *}@endverbatim
        */
        var init = function (s) {
            var cls =  s.form.clsError,
                x   =  s.form.idNewX,
                y   =  s.form.idNewY;

            elStatus
                    =  document.getElementById(s.data.idStatus);
            elRange =  document.getElementById(s.data.idRange);
            fCoords =  (new ns.FieldPoint(x, y)).setErrorClass(cls);
        };

        /**
        * @brief Update the view of the Page.
        *
        * The default behaviour is to load the current state of the device
        * (position, mostly), while maintaining any previously set coordinates
        * in the input fields.
        *
        * @param[in] evt If instance of Event, .preventDefault() will be
        *   invoked. Optional.
        */
        var reload = function (evt) {
            var req1 =  ns.createRequest(),
                req2 =  ns.createRequest();

            evt instanceof Event && evt.preventDefault();

            if(!req1 || !req2) return;

            reset();

            /* Request current device position. */
            req1.open("GET", "coordinates.php");
            req1.onreadystatechange  =  handleGETCoords;
            req1.send();

            /* Request operational range. */
            req2.open("GET", "configuration.php");
            req2.onreadystatechange  =  handleGETConfig;
            req2.send();
        };

        /**
        * @brief Update device head position.
        *
        * @param[in] evt If instance of Event, .preventDefault() will be
        *   invoked. Optional.
        */
        var move = function (evt) {
            var errors  =  {},
                req,
                coords;             // Field parameter value

            evt instanceof Event && evt.preventDefault();
            /* Clear any previously set field messages. */
            resetMsg();

            req =  ns.createRequest();
            if(!req) return;

            coords  =  fCoords.get(errors);

            /* TODO: Display error messages. */
            if(!ns.isEmpty(errors)) {

            } else {
                req.open("PUT", "coordinates.php");
                req.onreadystatechange = handlePUTCoords;
                req.send(JSON.stringify(coords));
            }
        };

        /**
        * @brief Request the device takes a new measurement.
        *
        * @param[in] evt If instance of Event, .preventDefault() will be
        *   invoked. Optional.
        */
        var sample = function(evt) {
            var req;

            evt instanceof Event && evt.preventDefault();

            /* Clear any previously set field messages. */
            resetMsg();

            req =  ns.createRequest();
            if(!req) return;
            req.open("POST", "measurement.php");
            req.onreadystatechange  =  handlePOSTSample;
            req.setRequestHeader("Content-Length", 0);
            req.send();
        };

        /**
        * @brief Reset all Fields (including error messages).
        *
        * @param[in] evt If instance of Event, .preventDefault() will be
        *   invoked. Optional.
        */
        var reset = function (evt) {
            evt instanceof Event && evt.preventDefault();

            resetMsg();
            fCoords.reset();
        };

        /**
        * @brief Request that all Fields remove their error messages.
        */
        var resetMsg = function() {
            fCoords.resetMsg();
        };

        /**
        * @brief Update operational range display and constraints.
        *
        * @param[in] conf Object containing @c X and @c Y maximum values.
        */
        var updateRange = function(conf) {
            fCoords.fieldX.max  =  conf.x - 1;  // Zero-based
            fCoords.fieldY.max  =  conf.y - 1;  // Zero-based
            elRange.innerHTML = conf.x + ", " + conf.y;
        };

        /**
        * @brief Called via onreadystatechange. Handles GETting configuration.
        *
        * The configuration is used to determine the operational range of the
        * device, that is, the maximum allowable @c X, @c Y, @cZ values. @c Z
        * cannot be set manually; it is either submerged or not.
        */
        var handleGETConfig = function () {
            var conf;

            if(this.readyState !== 4) return;

            /* Set field constraints and display operational range. */
            if(this.status === 200) {
                conf    =  JSON.parse(this.responseText);
                updateRange(conf);
            }
        };

        /**
        * @brief Called via onreadystatechange. Handles GETting coordinates.
        *
        * The current device coordinates are displayed on the UI for reference.
        * A @c Z value of @c 0 indicates the head is submerged.
        */
        var handleGETCoords = function () {
            var conf;

            if(this.readyState !== 4) return;

            /* Load current coordinates. */
            if(this.status === 200) {
                conf    =  JSON.parse(this.responseText);

                /* By-pass constraints; they may have been altered. */
                fCoords.fieldX.el.value    =  conf.x;
                fCoords.fieldY.el.value    =  conf.y;

                elStatus.innerHTML  =  conf.z === 0
                                    ? " δειγματοληψία"
                                    : " σε ετοιμότητα";

            } else if(this.status === 503) {
                elStatus.innerHTML  =  "εν κινήσει";

                /* Display estimated time of completion. */
                ns.log("Η συσκευή δήλωσε απασχολημένη. Εκτιμώμενος χρόνος"
                    + " ολοκλήρωσης τρέχουσας εργασίας: "
                    + ns.minsec(this.getResponseHeader("Retry-After")),
                      "info");
            }
        };

        /**
        * @brief Called via onreadystatechange. Handles PUTting coordinates.
        *
        * This Page uses method PUT to update the device position.
        */
        var handlePUTCoords = function () {
            var response;

            if(this.readyState !== 4) return;

            switch(this.status) {
                /* The device already is at the requested position. */
                case 200:
                    response    =  JSON.parse(this.responseText);
                    ns.log("Η συσκευή αποκρίθηκε ότι βρίσκεται ήδη στη θέση"
                        + " [X, Y] : [" + response.x + ", " + response.y + "]",
                            "info");
                break;

                /* Header Retry-After designates how long it should take to
                * reach the specified position. */
                case 202:
                    ns.log("Η συσκευή αποδέχθηκε το αίτημα. Εκτιμώμενος χρόνος"
                        + " ολοκλήρωσης: "
                        + ns.minsec(this.getResponseHeader("Retry-After")),
                          "info");
                break;

                /* Bad request. Display operational range; maybe they have been
                * modified in-between requests. */
                case 400:
                    response    =  JSON.parse(this.responseText);
                    ns.log("Η συσκευή απέρριψε το αίτημα λόγω εσφαλμένης τιμής."
                        + " Το τρέχον Λειτουργικό εύρος δηλώθηκε ότι είναι"
                        + " [X, Y] : [" + response.x + ", " + response.y + "]",
                          "fatal");
                    updateRange(response);
                break;

                /* Device is busy. */
                case 503:
                    ns.log("Η συσκευή δήλωσε απασχολημένη. Εκτιμώμενος χρόνος"
                        + " ολοκλήρωσης τρέχουσας εργασίας: "
                        + ns.minsec(this.getResponseHeader("Retry-After")),
                          "info");
                break;
            }
        };

        /**
        * @brief Called via onreadystatechange. Handles POSTing a measurement.
        *
        * This Page uses method POST to request taking a new sample.
        */
        var handlePOSTSample = function () {
            if(this.readyState !== 4) return;

            switch(this.status) {

                /* Header Retry-After designates how long it should take to
                * complete the request. */
                case(202):
                    ns.log("Η συσκευή αποδέχθηκε το αίτημα. Εκτιμώμενος χρόνος"
                        + " ολοκλήρωσης: "
                        + ns.minsec(this.getResponseHeader("Retry-After")),
                          "info");
                break;

                /* The device is busy. */
                case(503):
                    ns.log("Η συσκευή δήλωσε απασχολημένη. Εκτιμώμενος χρόνος"
                        + " ολοκλήρωσης τρέχουσας εργασίας: "
                        + ns.minsec(this.getResponseHeader("Retry-After")),
                          "info");
                break;
            }
        };

        return {"init"      : init,
                "reload"    : reload,
                "move"      : move,
                "sample"    : sample};
    })();
})(window.gNS = window.gNS || {});
