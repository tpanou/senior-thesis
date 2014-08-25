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


        var reload = function () {
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

        return {"init"      : init,
                "reload"    : reload,
                "submit"    : submit,
                "reset"     : reset};
    })();
})(window.gNS = window.gNS || {});
