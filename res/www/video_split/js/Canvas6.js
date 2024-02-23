let originWidth6; // 图片原始宽度
let originHeigh6; // 图片原始高度
let container6 = document.getElementById('container6');
let imgDiv6 = document.getElementById('imgDiv6');    // 存放mycanvas
//let btnDiv = document.getElementById('btnDiv1');
let clipImgDiv6 = document.getElementById('clipImgDiv6');    // 显示裁剪所获的图片
var oRelDiv6 = document.createElement("div"); // 截图框
var scaleX6 = 1;// 图片宽度缩放比例（当前实际/原始）
var scaleY6 = 1;  // 图片高度缩放比例（当前实际/原始）
var bgwidth6 = 417;
var bgheight6 = 220;

var img6 = new Image();
//拖拽与拉伸方法
//拖拽拉伸所需参数
let params6 = {
    left: 0,
    top: 0,
    width: 0,
    height: 0,
    currentX: 0,
    currentY: 0,
    flag: false,
    kind: "drag"
};
// CSS样式修改
container6.style.display = 'flex';
container6.style.flexDirection = 'column';
imgDiv6.style.marginBottom = '20px';

/**********************************************************************/
$(function () {
    ExecuteCanvas6();
})


function ExecuteCanvas6() {
    // 创建canvas，用于显示被裁剪图片
    var myCanvas6 = document.createElement('canvas');
    myCanvas6.setAttribute('id', 'myCanvas6');
    myCanvas6.style.display = 'block';
    /*myCanvas.style.position = 'absolute';*/
    myCanvas6.width = bgwidth6;
    myCanvas6.height = bgheight6;
    myCanvas6.style.border = "1px solid #d3d3d3";
    myCanvas6.innerText = '您的浏览器不支持 HTML5 canvas 标签。';
    myCanvas6.style.zIndex = 'auto';

    var ctx6 = myCanvas6.getContext('2d');

    // 被裁剪图片

    img6.src = image6;
    img6.setAttribute('id', 'img');
    img6.width = bgwidth6;
    img6.height = bgheight6;
    img6.onload = function () {
        console.log('onload()执行...');
        ctx6.drawImage(img6, 0, 0, bgwidth6, bgheight6);
        originWidth6 = img6.naturalWidth;
        originHeight6 = img6.naturalHeight;
        console.log('图片原始宽度=', originWidth6);
        console.log('图片原始高度=', originHeight6);

        cutImg6();
    };
    /**********************************************************************/

    imgDiv6.appendChild(myCanvas6);
}

// 获取指定元素DOM
const ID6 = function (id) {
    return document.getElementById(id);
};

//获取相关CSS属性方法
let getCss6 = function (o, key) {
    return o.currentStyle ? o.currentStyle[key] : document.defaultView.getComputedStyle(o, false)[key];
};


// 裁剪得到的图片
let clipImg6 = new Image();
clipImg6.src = '';
clipImg6.style.height = '100px';
clipImg6.style.width = '100px';
clipImg6.alt = '裁剪获得图片...';

function cutImg6() {
    var clickFlag = false;
    // 获取canvas中图片实际大小
    var iCurWidth = img6.width;
    var iCurHeight = img6.height;
    console.log('图片当前实际宽度=', iCurWidth);
    console.log('图片当前实际高度=', iCurHeight);

    // 可调整截图框
    oRelDiv6.innerHTML = '';
    oRelDiv6.style.position = "absolute";
    oRelDiv6.style.width = iCurWidth + "px";
    oRelDiv6.style.height = iCurHeight + "px";
    oRelDiv6.style.top = "92px";//myCanvas6.offsetTop + 'px';
    console.log('oRelDiv.style.top = ', oRelDiv6.style.top);
    oRelDiv6.id = "cropContainer6";

    var iOrigWidth = originWidth6;
    var iOrigHeight = originHeight6;
    scaleX6 = iCurWidth / iOrigWidth; // 图片宽度缩放比例（当前实际/原始）
    scaleY6 = iCurHeight / iOrigHeight;  // 图片高度缩放比例（当前实际/原始）
    console.log('图片横向（宽度）缩放比=', scaleX6);
    console.log('图片纵向（高度）缩放比=', scaleY6);

    // 将oRelDiv插入到myCanvas前
    myCanvas6.parentNode.insertBefore(oRelDiv6, myCanvas6);

    //初始化坐标与剪裁高宽
   if($("#up6").val()=="") {$("#up6").val(0);}
   if($("#left6").val()=="") {$("#left6").val(0);}
   if($("#right6").val()=="") {$("#right6").val(100);}
   if($("#down6").val()=="") {$("#down6").val(100);}

    var cropW=parseInt($("#right6").val())-parseInt($("#left6").val());
    var cropH=parseInt($("#down6").val())-parseInt($("#up6").val());
    var posY = parseInt($("#up6").val());
    var posX = parseInt($("#left6").val());
 
    oRelDiv6.innerHTML = '<div id="zxxCropBox6" style="height:' + cropH + 'px; width:' + cropW + 'px; position:absolute; left:' +
        posX + 'px; top:' + posY + 'px; border:1px solid black;">' +
        '<div id="zxxDragBg6" style="height:100%; background:white; opacity:0.3; filter:alpha(opacity=30); cursor:move"></div>' +
        '<div id="dragLeftTop6" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; top:-3px; cursor:nw-resize;"></div>' +
        '<div id="dragLeftBot6" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; bottom:-3px; cursor:sw-resize;"></div>' +
        '<div id="dragRightTop6" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; top:-3px; cursor:ne-resize;"></div>' +
        '<div id="dragRightBot6" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; bottom:-3px; cursor:se-resize;"></div>' +
        '<div id="dragTopCenter6" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; top:-3px; left:50%; margin-left:-3px; cursor:n-resize;"></div>' +
        '<div id="dragBotCenter6" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; bottom:-3px; left:50%; margin-left:-3px; cursor:s-resize;"></div>' +
        '<div id="dragRightCenter6" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; top:50%; margin-top:-3px; cursor:e-resize;"></div> ' +
        '<div id="dragLeftCenter6" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; top:50%; margin-top:-3px; cursor:w-resize;"></div>' +
        '</div>';
    if ($("#up6").val() == "")
        $("#up6").val(0);
    if ($("#left6").val() == "")
        $("#left6").val(0);
    if ($("#down6").val() == "")
        $("#down6").val(100);
    if ($("#right6").val() == "")
        $("#right6").val(100);
    ID("cropImageWidth6").value = parseInt(ID("zxxCropBox6").style.width);
    ID("cropImageHeight6").value = parseInt(ID("zxxCropBox6").style.height);

    var startDrag6 = function (point, target, kind) {
        //point是拉伸点，target是被拉伸的目标，其高度及位置会发生改变
        //此处的target与上面拖拽的target是同一目标，故其params.left,params.top可以共用，也必须共用
        //初始化宽高
        params6.width = getCss6(target, "width");
        params6.height = getCss6(target, "height");
        //初始化坐标
        if (getCss6(target, "left") !== "auto") {
            params6.left = getCss6(target, "left");
        }
        if (getCss6(target, "top") !== "auto") {
            params6.top = getCss6(target, "top");
        }
        //target是移动对象
        point.onmousedown = function (event) {
            params6.kind = kind;
            params6.flag = true;
            clickFlag = true;
            if (!event) {
                event = window.event;
            }
            var e = event;
            params6.currentX = e.clientX;  //鼠标按下时坐标x轴
            params6.currentY = e.clientY;  //鼠标按下时坐标y轴
            /*console.log('params.currentX=', params.currentX);
            console.log('params.currentY=', params.currentY);*/
            //防止IE文字选中，有助于拖拽平滑
            point.onselectstart = function () {
                return false;
            };

            document.onmousemove = function (event) {
                let e = event ? event : window.event;
                clickFlag = false;
                if (params6.flag) {
                    var nowX = e.clientX; // 鼠标移动时x坐标
                    var nowY = e.clientY;   // 鼠标移动时y坐标
                    var disX = nowX - params6.currentX;  // 鼠标x方向移动距离
                    var disY = nowY - params6.currentY;  // 鼠标y方向移动距离
                    if (params6.kind === "n") {
                        //上拉伸
                        //高度增加或减小，位置上下移动
                        if (parseInt(params6.top) + disY > 0) {
                            target.style.top = parseInt(params6.top) + disY + "px";
                            target.style.height = parseInt(params6.height) - disY + "px";
                        }
                    } else if (params6.kind === "w") { //左拉伸
                        if (parseInt(params6.left) + disX > 0) {
                            target.style.left = parseInt(params6.left) + disX + "px";
                            target.style.width = parseInt(params6.width) - disX + "px";
                        }
                    } else if (params6.kind === "e") { //右拉伸
                        if (parseInt(params6.width) + disX + parseInt(params6.left) < bgwidth6)
                           target.style.width = parseInt(params6.width) + disX + "px";
                    } else if (params6.kind === "s") { //下拉伸
                        if (parseInt(params6.height) + disY + parseInt(params6.top) < bgheight6)
                            target.style.height = parseInt(params6.height) + disY + "px";
                    } else if (params6.kind === "nw") { //左上拉伸
                        if (parseInt(params6.left) + disX > 0 && parseInt(params6.top) + disY > 0) {
                            target.style.left = parseInt(params6.left) + disX + "px";
                            target.style.width = parseInt(params6.width) - disX + "px";
                            target.style.top = parseInt(params6.top) + disY + "px";
                            target.style.height = parseInt(params6.height) - disY + "px";
                        }
                    } else if (params6.kind === "ne") { //右上拉伸
                        if (parseInt(params6.width) + disX + parseInt(params6.left) < bgwidth6 && parseInt(paparams6rams6.top) + disY > 0) {
                            target.style.top = parseInt(params6.top) + disY + "px";
                            target.style.height = parseInt(params6.height) - disY + "px";
                            target.style.width = parseInt(params6.width) + disX + "px";
                        }
                    } else if (params6.kind === "sw") { //左下拉伸
                        if (parseInt(params6.left) + disX > 0 && parseInt(params6.height) + disY + parseInt(params6.top) < bgheight6) {
                            target.style.left = parseInt(params6.left) + disX + "px";
                            target.style.width = parseInt(params6.width) - disX + "px";
                            target.style.height = parseInt(params6.height) + disY + "px";
                        }
                    } else if (params6.kind === "se") { //右下拉伸
                        if (parseInt(params6.width) + disX + parseInt(params6.left) < bgwidth6 && parseInt(params6.height) + disY + parseInt(params6.top) < bgheight6) {
                            target.style.width = parseInt(params6.width) + disX + "px";
                            target.style.height = parseInt(params6.height) + disY + "px";
                        }
                    } else { //移动
                        if (parseInt(params6.left) + disX < 0) { target.style.left = 0 + "px"; }//最左侧为0,小于0阻止移动
                        //最右侧，417-截图框宽度为left最大距离，超过阻止移动
                        else if (parseInt(params6.left) + disX + parseInt(ID("zxxCropBox6").style.width) > bgwidth6) { target.style.left = bgwidth6 - parseInt(ID("zxxCropBox6").style.width) + "px"; }
                        else target.style.left = parseInt(params6.left) + disX + "px";//正常范围
                        if (parseInt(params6.top) + disY < 0) { target.style.top = 0 }//高度小于0,阻止向上移动
                        else if (parseInt(params6.top) + disY + parseInt(ID("zxxCropBox6").style.height) > bgheight6)//移动高度+控件高度超过220，阻止移动
                        { target.style.top = bgheight6 - parseInt(ID("zxxCropBox6").style.height) + "px"; }
                        else { target.style.top = parseInt(params6.top) + disY + "px"; }//正常高度范围内
                    }
                }

                document.onmouseup = function () {
                    document.onmousemove = null;
                    document.onmouseup = null;
                    params6.flag = false;
                    if (getCss6(target, "left") !== "auto") {
                        params6.left = getCss6(target, "left");
                    }
                    if (getCss6(target, "top") !== "auto") {
                        params6.top = getCss6(target, "top");
                    }
                    params6.width = getCss6(target, "width");
                    params6.height = getCss6(target, "height");

                    //给隐藏文本框赋值
                    posX = parseInt(target.style.left);
                    posY = parseInt(target.style.top);
                    cropW = parseInt(target.style.width);
                    cropH = parseInt(target.style.height);
                    if (posX < 0) {
                        posX = 0;
                    }
                    if (posY < 0) {
                        posY = 0;
                    }
                    if ((posX + cropW) > iCurWidth) {
                        cropW = iCurWidth - posX;
                    }
                    if ((posY + cropH) > iCurHeight) {
                        cropH = iCurHeight - posY;
                    }
                    var right = posX + parseInt(ID("zxxCropBox6").style.width);
                    if (right < 0) right = 0;
                    var down = posY + parseInt(ID("zxxCropBox6").style.height);
                    if (down < 0) down = 0;
                    ID("left6").value = posX;
                    ID("up6").value = posY;
                    ID("right6").value = right;
                    ID("down6").value = down;
                    ID("cropImageWidth6").value = parseInt(ID("zxxCropBox6").style.width);
                    ID("cropImageHeight6").value = parseInt(ID("zxxCropBox6").style.height);
                };
            }
        };
    };

    //绑定拖拽
    startDrag6(ID6("zxxDragBg6"), ID6("zxxCropBox6"), "drag");
    //绑定拉伸
    startDrag6(ID6("dragLeftTop6"), ID6("zxxCropBox6"), "nw");
    startDrag6(ID6("dragLeftBot6"), ID6("zxxCropBox6"), "sw");
    startDrag6(ID6("dragRightTop6"), ID6("zxxCropBox6"), "ne");
    startDrag6(ID6("dragRightBot6"), ID6("zxxCropBox6"), "se");
    startDrag6(ID6("dragTopCenter6"), ID6("zxxCropBox6"), "n");
    startDrag6(ID6("dragBotCenter6"), ID6("zxxCropBox6"), "s");
    startDrag6(ID6("dragRightCenter6"), ID6("zxxCropBox6"), "e");
    startDrag6(ID6("dragLeftCenter6"), ID6("zxxCropBox6"), "w");


    //图片不能被选中，目的在于使拖拽顺滑
    ID6("myCanvas6").onselectstart = function () {
        return false;
    };
    img6.onselectstart = function () {
        return false;
    };
}

//确认裁剪
function RequeryCrop6() {
    var x = document.getElementById("left6").value;
    var y = document.getElementById("up6").value;
    var w = document.getElementById("cropImageWidth6").value;
    var h = document.getElementById("cropImageHeight6").value;
    console.log('cropImage(img,', x, ',', y, ',', parseInt(w), ',', parseInt(h), ')');
}


function cropImage6(img, cropPosX, cropPosY, width, height) {

    $("#imgDiv6").html('');
    var myCanvas = document.createElement('canvas');
    myCanvas.setAttribute('id', 'myCanvas6');
    myCanvas.style.display = 'block';
    /*myCanvas.style.position = 'absolute';*/
    myCanvas.width = width * scaleX6;
    myCanvas.height = height * scaleY6;
    myCanvas.style.border = "1px solid #d3d3d3";
    myCanvas.innerText = '您的浏览器不支持 HTML5 canvas 标签。';
    myCanvas.style.zIndex = 'auto';

    var newCtx = myCanvas.getContext('2d');
    newCtx.drawImage(img6, cropPosX, cropPosY, width, height, 0, 0, width * scaleX6, height * scaleY6);

    // canvas转化为图片
    var newImage = new Image();
    newImage.src = myCanvas.toDataURL("image/png");
    //newImage.style.marginLeft = '5px';
    newImage.width = bgwidth6;
    newImage.height = bgheight6;
    //clipImgDiv.appendChild(newImage);
    imgDiv6.appendChild(newImage);
    oRelDiv6.innerHTML = '';
}
