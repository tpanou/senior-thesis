;(function(ns) {

    /**
    * @brief Responsible for rendering the page of Log results.
    *
    * Functions of immediate interest are init(), reload(), submit() and
    * reset(). init() should be called first, before attempting to operate this.
    */
    ns.PageLog = ns.PageLog || (function() {

        /* These must *all* be supplied via init(). */
        var hash,           // Used to construct 
            fSize,
            fSince,
            fUntil,
            pages,          // Paginator variable
            elTable,        // Section to put data `dt's
            elSection,      // This element is hidden when first reloading
            clsHidden;

        /**
        * @brief Initialise this instance.
        *
        * @param[in] s Object with initialisation settings. It should contain
        *   the following keys, each with a value as follows:@verbatim {
        * hash                  URI fragment of this Page. All links generated
        *                       will be prefixed with this value (so it should
        *                       contain #)
        * form.idSize           id
        *     .idSince          id prefix (see FieldDateGroup())
        *     .idUntil          id prefix (as above)
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
        * @brief Create a URI fragment with the supplied @p index and the
        * current request parameters.
        *
        * @param[in] index The index value to set to the constructed URI
        *   fragment.
        * @returns A string that can be used to request the @c index page of
        *   the current request parameters.
        */
        var createURI = function(index) {
        };
    })();
})(window.gNS = window.gNS || {});
