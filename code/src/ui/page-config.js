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

            req.open("GET", "configuration.php");
            req.onreadystatechange  =  handleGET;
            req.send();
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
