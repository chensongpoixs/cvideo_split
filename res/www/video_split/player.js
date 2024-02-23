
function Player() {
    this.frameBuffer = [];
    this.webglPlayer = null;
    this.playing = false;
    this.initDecodeWorker();    
}



Player.prototype.initDecodeWorker = function () {
    this.decodeWorker = new Worker("decoder.js");
    var player = this;
    this.decodeWorker.onmessage = function (evt) {
        var objData = evt.data;
        switch (objData.t) {
            case kVideoFrame:
                player.frameBuffer.push(objData);
                break;
        }
    }
};

Player.prototype.play = function (domain, url, canvas) {
    console.log("Play " + url + ".");

    this.playing = true;
    var ret = {
        e: 0,
        m: "Success"
    };

    do {

        if (!url) {
            ret = {
                e: -1,
                m: "Invalid url"
            };
            success = false;
            console.log("[ER] playVideo error, url empty.");
            break;
        }

        if (!canvas) {
            ret = {
                e: -2,
                m: "Canvas not set"
            };
            success = false;
            console.log("[ER] playVideo error, canvas empty.");
            break;
        }


        if (!this.decodeWorker) {
            ret = {
                e: -4,
                m: "Decoder not initialized"
            };
            success = false;
            console.log("[ER] Decoder not initialized.");
            break
        }


        this.canvas = canvas;
        this.displayLoop();

        //var playCanvasContext = playCanvas.getContext("2d"); //If get 2d, webgl will be disabled.
        this.webglPlayer = new WebGLPlayer(this.canvas, {
            preserveDrawingBuffer: false
        });


        var reqCDec = {
            t: kCreateDec,
            u: url,
            d: domain
        };
        this.decodeWorker.postMessage(reqCDec);

        decWork = this.decodeWorker;
        this.registerVisibilityEvent(function (visible) {

            var VisiableEvent = {
                t: kVisiableEvent,
                v: visible
            };
            decWork.postMessage(VisiableEvent);
            if (visible) {
                console.log("visible");

            } else {
                console.log("not visible");
            }

        });

    } while (false);

    return ret;
};


Player.prototype.stop = function () {
    console.log("Closing decoder.");
    if(this.playing)
    {
        this.decodeWorker.postMessage({
            t: kCloseDecoderReq
        });
        this.playing = false;
    }

};


Player.prototype.fullscreen = function () {
    if (this.webglPlayer) {
        this.webglPlayer.fullscreen();
    }
};




Player.prototype.displayVideoFrame = function (frame) {

    this.yLength = frame.w * frame.h;
    this.uvLength = (frame.w / 2) * (frame.h / 2);
    this.webglPlayer.renderFrame(frame.d, frame.w, frame.h, this.yLength, this.uvLength);

    return true;
};


Player.prototype.displayLoop = function () {
    //if (this.playerState !== playerStateIdle) {
    requestAnimationFrame(this.displayLoop.bind(this));
    //}


    if (this.frameBuffer.length == 0) {
        return;
    }

    for (i = 0; i < 2; ++i) {
        var frame = this.frameBuffer[0];
        switch (frame.t) {
            case kVideoFrame:
                if (this.displayVideoFrame(frame)) {
                    this.frameBuffer.shift();
                }
                break;
            default:
                return;
        }

        if (this.frameBuffer.length == 0) {
            break;
        }
    }
};




Player.prototype.registerVisibilityEvent = function (cb) {
    var hidden = "hidden";

    // Standards:
    if (hidden in document) {
        document.addEventListener("visibilitychange", onchange);
    } else if ((hidden = "mozHidden") in document) {
        document.addEventListener("mozvisibilitychange", onchange);
    } else if ((hidden = "webkitHidden") in document) {
        document.addEventListener("webkitvisibilitychange", onchange);
    } else if ((hidden = "msHidden") in document) {
        document.addEventListener("msvisibilitychange", onchange);
    } else if ("onfocusin" in document) {
        // IE 9 and lower.
        document.onfocusin = document.onfocusout = onchange;
    } else {
        // All others.
        window.onpageshow = window.onpagehide = window.onfocus = window.onblur = onchange;
    }

    function onchange(evt) {
        var v = true;
        var h = false;
        var evtMap = {
            focus: v,
            focusin: v,
            pageshow: v,
            blur: h,
            focusout: h,
            pagehide: h
        };

        evt = evt || window.event;
        var visible = v;
        if (evt.type in evtMap) {
            visible = evtMap[evt.type];
        } else {
            visible = this[hidden] ? h : v;
        }
        cb(visible);
    }

    if (document[hidden] !== undefined) {
        onchange({ type: document[hidden] ? "blur" : "focus" });
    }
}
