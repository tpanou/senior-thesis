;(function (ns) {

    /**
    * @brief Responsible for rendering the page of Log results.
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
            fCoords;

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
        *     .clsError         class of `ul's when displaying field errors
        *}@endverbatim
        */
        var init = function (s) {
            var cls     =  s.form.clsError,
                x       =  s.form.idConfigX,
                y       =  s.form.idConfigY,
                z       =  s.form.idConfigZ;

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
        };

        /**
        * @brief Update the view of the Page.
        *
        * The default behaviour is to load the current configuration from the
        * device, replacing anything that may have been set on a previous visit.
        * This is done to ensure that the displayed configuration is up-to-date.
        */
        var reload = function () {
            var req =  ns.createRequest();

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
            req.send("{\"x\":\"\"}");
        };

        var submit = function () {
        };

        /**
        * @brief Clear the value of Fields (including error messages).
        *
        * If it is desired to reset to the current configuration, reload should
        * be used, instead.
        */
        var reset = function () {
            resetMsg();
            fIAddr.reset();
            fGateway.reset();
            fSubnet.reset();
            fDate.reset();
            fCoords.reset();
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
            var errors  =  0;

            reset();        // Clear all fields

            fIAddr.set(conf.iaddr)              || ++errors;
            fGateway.set(conf.gateway)          || ++errors;
            fSubnet.set(conf.subnet)            || ++errors;
            fDate.set(new Date(conf.date))      || ++errors;
            fCoords.set(conf.x, conf.y, conf.z) || ++errors;

            /* TODO: Show message on the unlikely event of an erroneous value */
        };

        return {"init"      : init,
                "reload"    : reload,
                "submit"    : submit,
                "reset"     : reset};
    })();
})(window.gNS = window.gNS || {});
