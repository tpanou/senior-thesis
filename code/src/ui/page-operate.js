;(function (ns) {

    /**
    * @brief Responsible for operating the device manually.
    */
    ns.PageOperate = ns.PageOperate || (function () {

        /* These must *all* be supplied via init(). */
        var elStatus,
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
        *}@endverbatim
        */
        var init = function (s) {
            var cls =  s.form.clsError,
                x   =  s.form.idNewX,
                y   =  s.form.idNewY;

            elStatus
                    =  document.getElementById(s.data.idStatus);
            fCoords =  (new ns.FieldPoint(x, y)).setErrorClass(cls);
        };

        /**
        * @brief Update the view of the Page.
        *
        * The default behaviour is to load the current coordinates of the device
        * while maintaining any previously set coordinates in the input fields.
        *
        * @param[in] evt If instance of Event, .preventDefault() will be
        *   invoked. Optional.
        */
        var reload = function (evt) {
            var req =  ns.createRequest();

            evt instanceof Event && evt.preventDefault();

            if(!req) return;

            reset();

            req.open("GET", "coordinates.php");
            req.onreadystatechange  =  handleGET;
        };

        /**
        * @brief Update device head position.
        *
        * @param[in] evt If instance of Event, .preventDefault() will be
        *   invoked. Optional.
        */
        var move = function (evt) {
        };

        var sample = function(evt) {
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
        * @brief Called via onreadystatechange. Handles asynchronous GET.
        */
        var handleGET = function () {
        };

        /**
        * @brief Called via onreadystatechange. Handles asynchronous PUT.
        */
        var handlePUT = function () {
        };

        /**
        * @brief Called via onreadystatechange. Handles asynchronous POST.
        */
        var handlePOST = function () {
        };

        return {"init"      : init,
                "reload"    : reload,
                "move"      : move,
                "sample"    : sample};
    })();
})(window.gNS = window.gNS || {});
