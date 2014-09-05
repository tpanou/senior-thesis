(function(ns) {

    /* Initialise Logger. */
    ns.Logger.init({    "idLog"         : "infobox",
                        "entries"       : 10,
                        "expires"       : 0.5,
                        "clsEntry"      : "",
                        "clsNew"        : "new",
                        "clsInfo"       : "info",
                        "clsCritical"   : "critical",
                        "clsFatal"      : "fatal"
                        });

    /* Request and display device date in Logger. */
    var req = ns.createRequest();
    if(!req) return;

    req.open("GET", "configuration.php");
    req.onreadystatechange = function() {
        var conf,
            date,
            day,
            el;

        if(this.readyState !== 4) return;

        conf    = JSON.parse(this.responseText);

        date    =  new Date(conf.date);
        date    =  ns.fixInt(date.getUTCDate(),     2) + "-"
                +  ns.fixInt(date.getUTCMonth(),    2) + "-"
                +  ns.fixInt(date.getUTCFullYear(), 2) + " "
                +            date.getUTCHours()        + ":"
                +  ns.fixInt(date.getUTCMinutes(),  2) + ":"
                +  ns.fixInt(date.getUTCSeconds(),  2);

        day     =  conf.day;

        days    = ["Κυρ", "Δευ", "Τρι", "Τετ", "Πεμ", "Παρ", "Σαβ"];

        el      =  document.getElementById("device-time");
        el.innerHTML = days[day - 1] + ", " + date;

        /* Also, reveal the element's parent which is, by default, hidden (so as
        * not to be displayed empty, in case Javascript is disabled). */
        el.parentNode.className = ns.replaceWord(el.parentNode.className,
                                                 "hidden",
                                                 "");

    };
    req.send();

    /**
    * @brief Update the document's title for PageHome.
    */
    function pageStaticHome() {
        window.document.title   =  "ΓΑΙΟΛικνο - Γενικά";
    };

    /**
    * @brief Update the document's title for PageHelp.
    */
    function pageStaticHelp() {
        window.document.title   =  "ΓΑΙΟΛικνο - Βοήθεια";
    };

    /* The area of search results (in page Log). Contains @c .total-count and
    * @c .pagination elements. */
    var lrs = document.getElementById("log-section-result");

    /* Uses for @c pages, @c menu and @c PageMonitor. */
    var pageNames   =  ["home", "log", "config", "operate", "help"];

    /* Handlers for each page. @c null is for simple-text (static) pages. */
    var handlers    =  [pageStaticHome,
                        ns.PageLog.reload,
                        ns.PageConfig.reload,
                        ns.PageOperate.reload,
                        pageStaticHelp];

    var pages   =  new ns.OneOfMany(pageNames,
                                    document.getElementById("content")
                                            .querySelectorAll(".page"),
                                    "page visible",
                                    "page hidden",
                                    "config");
    var menu    =  new ns.OneOfMany(pageNames,
                                    document.getElementById("nav")
                                            .querySelectorAll("li"),
                                    "nav-menu current",
                                    "nav-menu",
                                    "config");

    /* Initialise Pages. */

    ns.PageLog.init({   "hash"      : "#log",
                        "form"      : {
                            "idSize"    : "log-page-size",
                            "idSince"   : "log-since",
                            "idUntil"   : "log-until",
                            "clsError"  : "field-msg error"
                        },
                        "data"      : {
                            "elTotal"   : lrs.querySelector(".total-count"),
                            "elPages"   : lrs.querySelector(".pagination"),
                            "clsCurrent" : "button current",
                            "clsOther"  : "button",
                            "idSection" : "log-section-result",
                            "clsHidden" : "hidden",
                            "idTable"   : "log-table"
                        }});

    ns.PageConfig.init({    "form"  : {
                                "idIAddr"   : "config-iaddr",
                                "idGateway" : "config-gateway",
                                "idSubnet"  : "config-subnet",
                                "idDate"    : "config-date",
                                "idConfigX" : "config-x",
                                "idConfigY" : "config-y",
                                "idConfigZ" : "config-z",
                                "idIntervalHrs"     : "config-interval-hours",
                                "idIntervalMins"    : "config-interval-minutes",
                                "idSamples" : "config-samples",
                                "clsError"  : "field-msg error"
                            }});

    ns.PageOperate.init({   "form"  : {
                                "idNewX"    : "operate-x",
                                "idNewY"    : "operate-y",
                                "clsError"  : "field-msg error"
                            },
                            "data"  : {
                                "idStatus"  : "operate-status",
                                "idRange"   : "operate-range"
                            }});

    /* Initialise PageMonitor (the swapper). */
    ns.PageMonitor.init(pageNames, handlers, pages, menu, "home");

    /* Register event callbacks. */
    ns.addEventListener(document.getElementById("log-submit"),
                        "click",
                        ns.PageLog.submit);

    ns.addEventListener(document.getElementById("log-reset"),
                        "click",
                        ns.PageLog.reset);

    ns.addEventListener(document.getElementById("config-save"),
                        "click",
                        ns.PageConfig.submit);

    ns.addEventListener(document.getElementById("config-reset"),
                        "click",
                        ns.PageConfig.reset);

    ns.addEventListener(document.getElementById("config-reload"),
                        "click",
                        ns.PageConfig.reload);

    ns.addEventListener(document.getElementById("operate-move"),
                        "click",
                        ns.PageOperate.move);

    ns.addEventListener(document.getElementById("operate-sample"),
                        "click",
                        ns.PageOperate.sample);

    /* Add listeners to update the page content when the URI fragment is
    * altered. Reloading the state `onhashchange' responds to hash changes due
    * to history traversal (eg, pressing back or forward), whereas `onload'
    * deals with direct visiting (eg, a bookmarked link or page refresh). */
    ns.addEventListener(window, "hashchange", ns.PageMonitor.update);
    ns.addEventListener(window, "load", ns.PageMonitor.update);

})(window.gNS);
