;(function (ns) {

    /**
    * @brief Responsible for rendering the configuration of the device.
    *
    * Functions of immediate interest are init(), reload(), submit() and
    * reset(). init() should be called first, before attempting to operate this.
    */
    ns.PageConfig = ns.PageConfig || (function () {

        /* These must *all* be supplied via init(). */
        var fIAddr,
            fGateway,
            fSubnet,
            fDate,
            fCoords,
            elIntervalHrs,
            elIntervalMins,
            fSamples;

        var isInternal;         /* Denotes whether a request has been sent to
                                * the device for the needs of the UI and not
                                * explicitly by the user. */

        /**
        * @brief Initialise this instance.
        *
        * @param[in] s Object with initialisation settings. It should contain
        *   the following keys, each with a value as follows:@verbatim {
        * form.idIAddr          id of iaddr input field
        *     .idGateway        id of gateway input field
        *     .idSubnet         id of subnet input field
        *     .idDate           id prefix (see FieldDateGroup()) of server-date
        *     .idConfigX        id of maximum X input field
        *     .idConfigY        id of maximum Y input field
        *     .idConfigY        id of maximum Z input field
        *     .idIntervalHrs    id of interval hours field
        *     .idIntervalMins   id of interval minutes field
        *     .idSamples        id of automated samplings field
        *     .clsError         class of `ul's when displaying field errors
        *}@endverbatim
        */
        var init = function (s) {
            var cls     =  s.form.clsError,
                x       =  s.form.idConfigX,
                y       =  s.form.idConfigY,
                z       =  s.form.idConfigZ,
                hours   =  s.form.idIntervalHrs,
                mins    =  s.form.idIntervalMins,
                samples =  s.form.idSamples;

            fIAddr      =  (new ns.FieldIAddr(s.form.idIAddr))
                                  .setErrorClass(cls);
            fGateway    =  (new ns.FieldIAddr(s.form.idGateway))
                                  .setErrorClass(cls);
            fSubnet     =  (new ns.FieldIAddr(s.form.idSubnet))
                                  .setErrorClass(cls);
            fDate       =  (new ns.FieldDateGroup(s.form.idDate))
                                  .setErrorClass(cls);
            fCoords     =  (new ns.FieldPoint(x, y, z))
                                  .setErrorClass(cls);
            fSamples    =  (new ns.FieldInt(samples, 0, 100)
                                  .setErrorClass(cls));
            elIntervalHrs
                        =  document.getElementById(hours);
            elIntervalMins
                        =  document.getElementById(mins);

            /* Minimum operational range values. */
            fCoords.fieldX.min  = 1;
            fCoords.fieldY.min  = 1;
            fCoords.fieldZ.min  = 2;
        };

        /**
        * @brief Update the view of the Page.
        *
        * The default behaviour is to load the current configuration from the
        * device, replacing anything that may have been set on a previous visit.
        * This is done to ensure that the displayed configuration is up-to-date.
        *
        * @param[in] evt If instance of Event, .preventDefault() will be
        *   invoked. Optional.
        */
        var reload = function (evt) {
            var req =  ns.createRequest();
            window.document.title   =  "ΓΑΙΟΛικνο - Ρυθμίσεις";

            evt instanceof Event && evt.preventDefault();

            if(!req) return;

            reset();

            /* Send an erroneous value for the operational range. This results
            * in the device sending its physical limits. Those limits will be
            * used as constraints to the input fields. @c isInternal designates
            * this nature of the following response so that the back-end may act
            * accordingly.  */
            isInternal  =  true;

            req.open("PUT", "configuration.php");
            req.onreadystatechange  =  handlePUT;
            req.send("{\"x\":9999}");
        };

        /**
        * @brief Send new configuration to the device.
        *
        * @param[in] evt If instance of Event, .preventDefault() will be
        *   invoked. Optional.
        */
        var submit = function (evt) {
            var errors  =  {},
                payload =  {},      // Request
                value,              // Each parsed value
                req,
                date,               // Field value
                coords;             // Field parameter value

            evt instanceof Event && evt.preventDefault();

            /* Clear any previously set field messages. */
            resetMsg();

            /* Only include Fields that have been set. */

            if(fIAddr.el.value && (value = fIAddr.get(errors)) !== null) {
                payload.iaddr = value;
            }

            if(fGateway.el.value && (value = fGateway.get(errors)) !== null) {
                payload.gateway = value;
            }

            if(fSubnet.el.value && (value = fSubnet.get(errors)) !== null) {
                payload.subnet = value;
            }

            if(fDate.elYear.value && fDate.elMonth.value && fDate.elDate.value
            && fDate.elHours.value && fDate.elMinutes.value) {
                date            =  fDate.get(errors);
            } else {
                fDate.reset();
            }

            (  fCoords.fieldX.el.value
            || fCoords.fieldY.el.value
            || fCoords.fieldZ.el.value)
            && (value = fCoords.get(errors)) === null || (coords = value);

            if(date) {
                payload.date    =  date.toJSON();
                payload.day     =  date.getDay();
            }
            if(coords) {
                payload.x       =  coords.x;
                payload.y       =  coords.y;
                payload.z       =  coords.z;
            }

            /* Each hours contains 10 quanta; each quantum equals 6 minutes. */
            payload.interval    =  elIntervalHrs.selectedIndex * 10
                                +  elIntervalMins.selectedIndex;

            if(fSamples.el.value && (value = fSamples.get(errors) !== null)) {
                payload.samples =  value;
            }

            /* Display error messages and inform the configuration has not been
            * saved. */
            if(!ns.isEmpty(errors)) {

            } else if(!ns.isEmpty(payload)) {

                req     =  ns.createRequest();
                req.open("PUT", "configuration.php");
                req.onreadystatechange = handlePUT;
                req.send(JSON.stringify(payload));
            } else {

                /* All fields were empty. Inform user no operation was
                * performed. */
                ns.log("Συμπληρώστε πρώτα τα επιθυμητά πεδία και μετά πατήσετε "
                     + "&laquo;Αποθήκευση&raquo;.", "critical");
            }
        };

        /**
        * @brief Reset all Fields (including error messages).
        *
        * If it is desired to reset to the current configuration, reload should
        * be used, instead.
        *
        * @param[in] evt If instance of Event, .preventDefault() will be
        *   invoked. Optional.
        */
        var reset = function (evt) {
            evt instanceof Event && evt.preventDefault();

            resetMsg();
            fIAddr.reset();
            fGateway.reset();
            fSubnet.reset();
            fDate.reset();
            fCoords.reset();
            fSamples.reset();
            elIntervalHrs.selectedIndex     =  0;
            elIntervalMins.selectedIndex    =  0;
        };

        /**
        * @brief Request that all Fields remove their error messages.
        */
        var resetMsg = function() {
            fIAddr.resetMsg();
            fGateway.resetMsg();
            fSubnet.resetMsg();
            fDate.resetMsg();
            fCoords.resetMsg();
            fSamples.resetMsg();
        };

        /**
        * @brief Called via onreadystatechange. Handles asynchronous GET.
        *
        * This Page uses method GET only to retrieve the current configuration
        * of the device.
        */
        var handleGET = function () {

            if(this.readyState !== 4) return;

            if(this.status === 200) loadFields(JSON.parse(this.responseText));
        };

        /**
        * @brief Called via onreadystatechange. Handles asynchronous PUT.
        *
        * This Page uses method PUT to update the device configuration. It also
        * uses it, behind the scenes, to retrieve the physical limits of the
        * device and set them as constraints on the Operational Range Field. To
        * do so, it send an erroneous request (which is, of course dropped by
        * the device) but results in the device advertising those limits.
        */
        var handlePUT = function () {
            var response;

            if(this.readyState !== 4) return;

            response    =  JSON.parse(this.responseText);

            if(this.status === 200) {
                /* Inform configuration has been saved. Also, reload the
                * configuration from the device response. */
                ns.log("Η ρυθμίσεις αποθηκεύτηκαν.", "info");
                loadFields(response);

            } else if(this.status === 400) {

                /* If the request was sent to retrieve the absolute physical
                * limits, set them as constraints to the corresponding input
                * fields (@c fConfigX, @c fConfigY and @c fConfigZ). */
                if(isInternal) {
                    /* Set current constraints. */
                    fCoords.fieldX.max  =  response.x;
                    fCoords.fieldY.max  =  response.y;
                    fCoords.fieldZ.max  =  response.z;

                    /* Reuse this request instance, this time, to actually
                    * retrieve the current configuration. */
                    this.open("GET", "configuration.php");
                    this.onreadystatechange  =  handleGET;
                    this.send();
                } else {
                    console.log("request has been dropped: ", response);
                    /* Inform request has been dropped, probably due to
                    * an erroneous value. Also inform what are the physical
                    * limits. */
                    ns.log("Η συσκευή απέρριψε το αίτημα λόγω εσφαλμένης τιμής."
                        + " Το Λειτουργικό εύρος δεν μπορεί να ξεπερνά τις"
                        + " φυσικές διαστάσεις της συσκευής. Οι μέγιστες"
                        + " αποδεκτές τιμές είναι [X, Y, Z] : ["
                        + response.x + ", " + response.y + ", "
                        + response.z + "].", "fatal");
                }
            }

            isInternal  =  false;
        };

        /**
        * @brief Set the fields to the specified values.
        *
        * @param[in] conf An object containing the device configuration, as
        *   returned by the corresponding endpoint.
        */
        var loadFields = function (conf) {
            var errors  =  0,
                interval;

            reset();        // Clear all fields

            fIAddr.set(conf.iaddr)              || ++errors;
            fGateway.set(conf.gateway)          || ++errors;
            fSubnet.set(conf.subnet)            || ++errors;
            fDate.set(new Date(conf.date))      || ++errors;
            fCoords.set(conf.x, conf.y, conf.z) || ++errors;
            fSamples.set(conf.samples)          || ++errors;

            interval    =  conf.interval;
            elIntervalHrs.selectedIndex     =  Math.floor(interval / 10);
            elIntervalMins.selectedIndex    =  Math.floor(interval % 10);

            /* TODO: Show message on the unlikely event of an erroneous value */
        };

        return {"init"      : init,
                "reload"    : reload,
                "submit"    : submit,
                "reset"     : reset};
    })();
})(window.gNS = window.gNS || {});
