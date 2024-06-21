self.Module = {
    onRuntimeInitialized: function () {
        if (self.decoder) {
            self.decoder.onWasmLoaded();
        } else {
            console.log("[ER] No decoder!");
        }
    }
};

self.importScripts("./common.js");
self.importScripts("./libffmpeg.js");
self.importScripts("./js/config.js");

function Decoder() {
    this.coreLogLevel = 1;
    this.wasmLoaded = false;
    this.cacheBuffer = null;
    this.tmpReqQue = [];
    this.videoCallback = null;
    this.audioCallback = null;
    this.requestCallback = null;
    this.ws == null
    this.hDec = 0;
    this.width = 0;
    this.height = 0;
    this.url = null;
}





///////////////////////////////////////// lkpadd
Decoder.prototype.onWsMsg = function (evt) 
{
	// console.log('onWsMsg ---->>>> evt.data.byteLength =' + evt.data.byteLength);
    if (evt.data.byteLength == 16 && this.hDec == 0) {
        // success
        var param = new Int16Array(evt.data);
        //var codecId=174;//265
        //var codecId=28;//264
        var codecId = param[0];//
        this.hDec = Module._CreateVideoDec(codecId, this.videoCallback);
        this.cacheBuffer = Module._malloc(1024 * 1024);
        console.log("CreateVideoDec " + ".");
        if (this.hDec == 0) {
            console.log("err CreateVideoDec failed");
        }
    }
    else if (evt.data.byteLength === 32 && this.hDec == 0)
    {
        // decodef failed 
        console.log("chenosng");
    }
    else if (this.hDec != 0) {
        var stream = new Uint8Array(evt.data);
        this.DecVideo(stream, stream.byteLength);
    }


}

Decoder.prototype.width = function()
{
    return this.width;
}
Decoder.prototype.height = function()
{
    return this.height;
}

Decoder.prototype.CreateVideoDec = function (url, domain) {

    var dec = this;
    if (this.ws == null) {
        //var wsurl = "ws://192.168.0.191:9600";
		// var wsurl = "ws://192.168.2.91:9600";
        this.ws = new WebSocket(decoder_url_address);
        this.ws.binaryType = 'arraybuffer';

        this.ws.onopen = function (evt) {
            console.log("Ws connected. =  " + decoder_url_address);
            // {"msg_id":202,"url":"udp://@224.1.1.3:20000"}
			let play_data  = JSON.stringify({
                                               msg_id: 202,
                                               url: url
                                           });
                               console.log(`paly -> SS: offer:\n${play_data}`);
            this.url = url;
            dec.ws.send(play_data);//lkpmd
        };

        this.ws.onerror = function (evt) {
            console.log("Ws connect error " + evt.data);
        }

        this.ws.onmessage = this.onWsMsg.bind(this);
    }

};

Decoder.prototype.DecVideo = function (streamBuff, dataSize) {

    Module.HEAPU8.set(streamBuff, this.cacheBuffer);
    var rt = Module._DecVideo(this.hDec, this.cacheBuffer, dataSize);
    if (rt != 0) {
        console.log("err DecVideo failed");
    }
    return rt;
};
Decoder.prototype.ReleaseVideoDecode = function () {
    console.log("release decoder");
    //  var stream = new Uint8Array(16060);
    // this.DecVideo(stream, stream.byteLength);
     if (this.ws != null)
	 {
		  this.ws.close();
	 }
    this.ws = null;
    if (this.cacheBuffer != null) {
        Module._free(this.cacheBuffer);
        this.cacheBuffer = null;
    }
    Module._ReleaseVideoDecode(this.hDec);
    this.hDec = 0;
};

Decoder.prototype.cacheReq = function (req) {
    if (req) {
        this.tmpReqQue.push(req);
    }
};
Decoder.prototype.processReq = function (req) {
    switch (req.t) {
        case kCreateDec:
            this.CreateVideoDec(req.u, req.d);
            break;
        case kCloseDecoderReq:
            this.ReleaseVideoDecode();
            break;
        case kVisiableEvent:
            if (this.ws && this.ws.readyState == WebSocket.OPEN) 
            {
                let play_data  = JSON.stringify({
                                               msg_id: 202,
                                               url: this.url
                                           });
                               console.log(`paly -> SS: offer:\n${play_data}: req: ${req}`);
                               this.ws.send(play_data);
                // if (req.v)
                //     this.ws.send("onshow");
                // else
                //     this.ws.send("onhide");
            }
            break;

        default:
            console.log("Unsupport messsage " + req.t);
    }
};


Decoder.prototype.onWasmLoaded = function () {
    console.log("Wasm loaded.");
    this.wasmLoaded = true;

    // this.videoCallback = Module.addFunction(function (buff, width, height, timestamp) {
    //     var outArray = Module.HEAPU8.subarray(buff, buff + width * height * 3 / 2);
    //     var data = new Uint8Array(outArray);
    //     var objData = {
    //         t: kVideoFrame,
    //         s: timestamp,
    //         d: data,
    //         w: width,
    //         h: height
    //     };

    this.videoCallback = Module.addFunction(function (buff, size, timestamp) {
		
        var width = Math.floor(size / 10000);
        var height = size % 10000;
        var outArray = Module.HEAPU8.subarray(buff, buff + width * height * 3 / 2);
        var data = new Uint8Array(outArray);
        var objData = {
            t: kVideoFrame,
            s: timestamp,
            d: data,
            w: width,
            h: height
        };
		//console.log('t = '+ kVideoFrame + ' s = ' + timestamp + ', w = ' + width + ', height = ' + height);
        this.width = width;
        this.height = height;
        this.postMessage(objData, [objData.d.buffer]);
    }, 'viid');

    this.audioCallback = Module.addFunction(function (buff, size, timestamp) {
        var outArray = Module.HEAPU8.subarray(buff, buff + size);
        var data = new Uint8Array(outArray);
        var objData = {
            t: kAudioFrame,
            s: timestamp,
            d: data
        };
        this.postMessage(objData, [objData.d.buffer]);
    }, 'viid');



    while (this.tmpReqQue.length > 0) {
        var req = this.tmpReqQue.shift();
        this.processReq(req);
    }

};

self.decoder = new Decoder;

self.onmessage = function (evt) {
    if (!self.decoder) {
        console.log("[ER] Decoder not initialized!");
        return;
    }

    var req = evt.data;
    if (!self.decoder.wasmLoaded) {
        self.decoder.cacheReq(req);
        console.log("Temp cache req " + req.t + ".");
        return;
    }


    self.decoder.processReq(req);
};

