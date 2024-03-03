let originWidth11; // 图片原始宽度
let originHeigh11; // 图片原始高度
let container11 = document.getElementById('container11');
let imgDiv11 = document.getElementById('imgDiv11');    // 存放mycanvas
//let btnDiv = document.getElementById('btnDiv1');
let clipImgDiv11 = document.getElementById('clipImgDiv11');    // 显示裁剪所获的图片
var oRelDiv11 = document.createElement("div"); // 截图框
var scaleX11 = 1;// 图片宽度缩放比例（当前实际/原始）
var scaleY11 = 1;  // 图片高度缩放比例（当前实际/原始）
var bgwidth11 = 417;
var bgheight11 = 220;

var img11 = new Image();
//拖拽与拉伸方法
//拖拽拉伸所需参数
let params11 = {
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
container11.style.display = 'flex';
container11.style.flexDirection = 'column';
//btnDiv.style.marginBottom = '20px';
//btnDiv.style.height = '110px';
imgDiv11.style.marginBottom = '20px';

/**********************************************************************/
$(function () {
    ExecuteCanvas11();
})


function ExecuteCanvas11() {
    // 创建canvas，用于显示被裁剪图片
    var myCanvas11 = document.createElement('canvas');
    myCanvas11.setAttribute('id', 'myCanvas11');
    myCanvas11.style.display = 'block';
    /*myCanvas.style.position = 'absolute';*/
    myCanvas11.width = bgwidth11;
    myCanvas11.height = bgheight11;
    myCanvas11.style.border = "1px solid #d3d3d3";
    myCanvas11.innerText = '您的浏览器不支持 HTML5 canvas 标签。';
    myCanvas11.style.zIndex = 'auto';

    var ctx11 = myCanvas11.getContext('2d');

    // 被裁剪图片

    img11.src = image11;
    img11.setAttribute('id', 'img');
    img11.width = bgwidth11;
    img11.height = bgheight11;
    img11.onload = function () {
        console.log('onload()执行...');
        ctx11.drawImage(img11, 0, 0, bgwidth11, bgheight11);
        originWidth11 = img11.naturalWidth;
        originHeight11 = img11.naturalHeight;
        console.log('图片原始宽度=', originWidth11);
        console.log('图片原始高度=', originHeight11);

        cutImg11();
    };
    /**********************************************************************/

    imgDiv11.appendChild(myCanvas11);
}

// 获取指定元素DOM
const ID11 = function (id) {
    return document.getElementById(id);
};

//获取相关CSS属性方法
let getCss11 = function (o, key) {
    return o.currentStyle ? o.currentStyle[key] : document.defaultView.getComputedStyle(o, false)[key];
};


// 裁剪得到的图片
let clipImg11 = new Image();
clipImg11.src = '';
clipImg11.style.height = '100px';
clipImg11.style.width = '100px';
clipImg11.alt = '裁剪获得图片...';

function cutImg11() {
    var clickFlag = false;
    // 获取canvas中图片实际大小
    var iCurWidth = img11.width;
    var iCurHeight = img11.height;
    console.log('图片当前实际宽度=', iCurWidth);
    console.log('图片当前实际高度=', iCurHeight);

    // 可调整截图框
    oRelDiv11.innerHTML = '';
    oRelDiv11.style.position = "absolute";
    oRelDiv11.style.width = iCurWidth + "px";
    oRelDiv11.style.height = iCurHeight + "px";
    oRelDiv11.style.top ="377px";// myCanvas11.offsetTop + 'px';
    console.log('oRelDiv.style.top = ', oRelDiv11.style.top);
    oRelDiv11.id = "cropContainer11";

    var iOrigWidth = originWidth11;
    var iOrigHeight = originHeight11;
    scaleX11 = iCurWidth / iOrigWidth; // 图片宽度缩放比例（当前实际/原始）
    scaleY11 = iCurHeight / iOrigHeight;  // 图片高度缩放比例（当前实际/原始）
    console.log('图片横向（宽度）缩放比=', scaleX11);
    console.log('图片纵向（高度）缩放比=', scaleY11);

    // 将oRelDiv插入到myCanvas前
    myCanvas11.parentNode.insertBefore(oRelDiv11, myCanvas11);

    //初始化坐标与剪裁高宽
    //var cropW = 100; //截图框默认宽度
    //var cropH = 100; //截图框默认高度
    /*console.log('myCanvas.offsetLeft=', myCanvas.offsetLeft);
    console.log('myCanvas.offsetTop=', myCanvas.offsetTop);*/
    //var posX = 0; //myCanvas.width / 2 - cropW / 2;  // 截图框左上角x坐标
    //var posY = 0; //myCanvas.height / 2 - cropH / 2;    // 截图框左上角y坐标
    /*console.log('posX=',posX);
    console.log('posY=',posY);*/
   if($("#up11").val()=="") {$("#up11").val(0);}
   if($("#left11").val()=="") {$("#left11").val(0);}
   if($("#right11").val()=="") {$("#right11").val(100);}
   if($("#down11").val()=="") {$("#down11").val(100);}

    var cropW=parseInt($("#right11").val())-parseInt($("#left11").val());
    var cropH=parseInt($("#down11").val())-parseInt($("#up11").val());
    var posY = parseInt($("#up11").val());
    var posX = parseInt($("#left11").val());
 
    oRelDiv11.innerHTML = '<div id="zxxCropBox11" style="height:' + cropH + 'px; width:' + cropW + 'px; position:absolute; left:' +
        posX + 'px; top:' + posY + 'px; border:1px solid black;">' +
        '<div id="zxxDragBg11" style="height:100%; background:white; opacity:0.3; filter:alpha(opacity=30); cursor:move"></div>' +
        '<div id="dragLeftTop11" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; top:-3px; cursor:nw-resize;"></div>' +
        '<div id="dragLeftBot11" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; bottom:-3px; cursor:sw-resize;"></div>' +
        '<div id="dragRightTop11" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; top:-3px; cursor:ne-resize;"></div>' +
        '<div id="dragRightBot11" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; bottom:-3px; cursor:se-resize;"></div>' +
        '<div id="dragTopCenter11" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; top:-3px; left:50%; margin-left:-3px; cursor:n-resize;"></div>' +
        '<div id="dragBotCenter11" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; bottom:-3px; left:50%; margin-left:-3px; cursor:s-resize;"></div>' +
        '<div id="dragRightCenter11" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; top:50%; margin-top:-3px; cursor:e-resize;"></div> ' +
        '<div id="dragLeftCenter11" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; top:50%; margin-top:-3px; cursor:w-resize;"></div>' +
        '</div>';
    if($("#up11").val()=="")
    $("#up11").val(0);
  if($("#left11").val()=="")
  $("#left11").val(0);
  if($("#down11").val()=="")
  $("#down11").val(100);
  if($("#right11").val()=="")
  $("#right11").val(100);
    ID("cropImageWidth11").value = parseInt(ID("zxxCropBox11").style.width);
    ID("cropImageHeight11").value = parseInt(ID("zxxCropBox11").style.height);

    var startDrag11 = function (point, target, kind) {
        //point是拉伸点，target是被拉伸的目标，其高度及位置会发生改变
        //此处的target与上面拖拽的target是同一目标，故其params.left,params.top可以共用，也必须共用
        //初始化宽高
        params11.width = getCss11(target, "width");
        params11.height = getCss11(target, "height");
        //初始化坐标
        if (getCss11(target, "left") !== "auto") {
            params11.left = getCss11(target, "left");
        }
        if (getCss11(target, "top") !== "auto") {
            params11.top = getCss11(target, "top");
        }
        //target是移动对象
        point.onmousedown = function (event) {
            params11.kind = kind;
            params11.flag = true;
            clickFlag = true;
            if (!event) {
                event = window.event;
            }
            var e = event;
            params11.currentX = e.clientX;  //鼠标按下时坐标x轴
            params11.currentY = e.clientY;  //鼠标按下时坐标y轴
            /*console.log('params.currentX=', params.currentX);
            console.log('params.currentY=', params.currentY);*/
            //防止IE文字选中，有助于拖拽平滑
            point.onselectstart = function () {
                return false;
            };

            document.onmousemove = function (event) {
                let e = event ? event : window.event;
                clickFlag = false;
                if (params11.flag) {
                    var nowX = e.clientX; // 鼠标移动时x坐标
                    var nowY = e.clientY;   // 鼠标移动时y坐标
                    var disX = nowX - params11.currentX;  // 鼠标x方向移动距离
                    var disY = nowY - params11.currentY;  // 鼠标y方向移动距离
                    if (params11.kind === "n") {
                        //上拉伸
                        //高度增加或减小，位置上下移动
                        if (parseInt(params11.top) + disY > 0) {
                            target.style.top = parseInt(params11.top) + disY + "px";
                            target.style.height = parseInt(params11.height) - disY + "px";
                        }
                    } else if (params11.kind === "w") { //左拉伸
                        if (parseInt(params11.left) + disX > 0) {
                            target.style.left = parseInt(params11.left) + disX + "px";
                            target.style.width = parseInt(params11.width) - disX + "px";
                        }
                    } else if (params11.kind === "e") { //右拉伸
                        if (parseInt(params11.width) + disX + parseInt(params11.left) < bgwidth11)
                           target.style.width = parseInt(params11.width) + disX + "px";
                    } else if (params11.kind === "s") { //下拉伸
                        if (parseInt(params11.height) + disY + parseInt(params11.top) < bgheight11)
                            target.style.height = parseInt(params11.height) + disY + "px";
                    } else if (params11.kind === "nw") { //左上拉伸
                        if (parseInt(params11.left) + disX > 0 && parseInt(params11.top) + disY > 0) {
                            target.style.left = parseInt(params11.left) + disX + "px";
                            target.style.width = parseInt(params11.width) - disX + "px";
                            target.style.top = parseInt(params11.top) + disY + "px";
                            target.style.height = parseInt(params11.height) - disY + "px";
                        }
                    } else if (params11.kind === "ne") { //右上拉伸
                        if (parseInt(params11.width) + disX + parseInt(params11.left) < bgwidth11 && parseInt(params11.top) + disY > 0) {
                            target.style.top = parseInt(params11.top) + disY + "px";
                            target.style.height = parseInt(params11.height) - disY + "px";
                            target.style.width = parseInt(params11.width) + disX + "px";
                        }
                    } else if (params11.kind === "sw") { //左下拉伸
                        if (parseInt(params11.left) + disX > 0 && parseInt(params11.height) + disY + parseInt(params11.top) < bgheight11) {
                            target.style.left = parseInt(params11.left) + disX + "px";
                            target.style.width = parseInt(params11.width) - disX + "px";
                            target.style.height = parseInt(params11.height) + disY + "px";
                        }
                    } else if (params11.kind === "se") { //右下拉伸
                        if (parseInt(params11.width) + disX + parseInt(params11.left) < bgwidth11 && parseInt(params11.height) + disY + parseInt(params11.top) < bgheight11) {
                            target.style.width = parseInt(params11.width) + disX + "px";
                            target.style.height = parseInt(params11.height) + disY + "px";
                        }
                    } else { //移动
                        if (parseInt(params11.left) + disX < 0) { target.style.left = 0 + "px"; }//最左侧为0,小于0阻止移动
                        //最右侧，417-截图框宽度为left最大距离，超过阻止移动
                        else if (parseInt(params11.left) + disX + parseInt(ID("zxxCropBox11").style.width) > bgwidth11) { target.style.left = bgwidth11 - parseInt(ID("zxxCropBox11").style.width) + "px"; }
                        else target.style.left = parseInt(params11.left) + disX + "px";//正常范围
                        if (parseInt(params11.top) + disY < 0) { target.style.top = 0 }//高度小于0,阻止向上移动
                        else if (parseInt(params11.top) + disY + parseInt(ID("zxxCropBox11").style.height) > bgheight11)//移动高度+控件高度超过220，阻止移动
                        { target.style.top = bgheight11 - parseInt(ID("zxxCropBox11").style.height) + "px"; }
                        else { target.style.top = parseInt(params11.top) + disY + "px"; }//正常高度范围内
                    }
                }

                document.onmouseup = function () {
                    document.onmousemove = null;
                    document.onmouseup = null;
                    params11.flag = false;
                    if (getCss11(target, "left") !== "auto") {
                        params11.left = getCss11(target, "left");
                    }
                    if (getCss11(target, "top") !== "auto") {
                        params11.top = getCss11(target, "top");
                    }
                    params11.width = getCss11(target, "width");
                    params11.height = getCss11(target, "height");
                    /*console.log('params.width=', params.width);
                    console.log('params.height', params.width);*/

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
                    //赋值
                    //ID("cropPosX").value = posX;
                    //ID("cropPosY").value = posY;
                    //ID("cropImageWidth").value = parseInt(ID("zxxCropBox").style.width);
                    //ID("cropImageHeight").value = parseInt(ID("zxxCropBox").style.height);
                    var right = posX + parseInt(ID("zxxCropBox11").style.width);
                    if (right < 0) right = 0;
                    var down = posY + parseInt(ID("zxxCropBox11").style.height);
                    if (down < 0) down = 0;
                    ID("left11").value = posX;
                    ID("up11").value = posY;
                    ID("right11").value = right;
                    ID("down11").value = down;
                    ID("cropImageWidth11").value = parseInt(ID("zxxCropBox11").style.width);
                    ID("cropImageHeight11").value = parseInt(ID("zxxCropBox11").style.height);
                };
            }
        };
    };

    //绑定拖拽
    startDrag11(ID11("zxxDragBg11"), ID11("zxxCropBox11"), "drag");
    //绑定拉伸
    startDrag11(ID11("dragLeftTop11"), ID11("zxxCropBox11"), "nw");
    startDrag11(ID11("dragLeftBot11"), ID11("zxxCropBox11"), "sw");
    startDrag11(ID11("dragRightTop11"), ID11("zxxCropBox11"), "ne");
    startDrag11(ID11("dragRightBot11"), ID11("zxxCropBox11"), "se");
    startDrag11(ID11("dragTopCenter11"), ID11("zxxCropBox11"), "n");
    startDrag11(ID11("dragBotCenter11"), ID11("zxxCropBox11"), "s");
    startDrag11(ID11("dragRightCenter11"), ID11("zxxCropBox11"), "e");
    startDrag11(ID11("dragLeftCenter11"), ID11("zxxCropBox11"), "w");


    //图片不能被选中，目的在于使拖拽顺滑
    ID11("myCanvas11").onselectstart = function () {
        return false;
    };
    img11.onselectstart = function () {
        return false;
    };
}

//确认裁剪
function RequeryCrop11() {
    var x = document.getElementById("left11").value;
    var y = document.getElementById("up11").value;
    var w = document.getElementById("cropImageWidth11").value;
    var h = document.getElementById("cropImageHeight11").value;
    console.log('cropImage(img,', x, ',', y, ',', parseInt(w), ',', parseInt(h), ')');
}


function cropImage11(img, cropPosX, cropPosY, width, height) {
    /*var cropContainer = ID("cropContainer");
    cropContainer.parentNode.removeChild(cropContainer);*/
    /*ctx.clearRect(0, 0, myCanvas.width, myCanvas.height);*/
    //sx,sy 是相对于图片的坐标。巨坑

    $("#imgDiv11").html('');
    var myCanvas = document.createElement('canvas');
    myCanvas.setAttribute('id', 'myCanvas11');
    myCanvas.style.display = 'block';
    /*myCanvas.style.position = 'absolute';*/
    myCanvas.width = width * scaleX11;
    myCanvas.height = height * scaleY11;
    myCanvas.style.border = "1px solid #d3d3d3";
    myCanvas.innerText = '您的浏览器不支持 HTML5 canvas 标签。';
    myCanvas.style.zIndex = 'auto';

    var newCtx = myCanvas.getContext('2d');
    newCtx.drawImage(img11, cropPosX, cropPosY, width, height, 0, 0, width * scaleX11, height * scaleY11);

    // canvas转化为图片
    var newImage = new Image();
    newImage.src = myCanvas.toDataURL("image/png");
    //newImage.style.marginLeft = '5px';
    newImage.width = bgwidth11;
    newImage.height = bgheight11;
    //clipImgDiv.appendChild(newImage);
    imgDiv11.appendChild(newImage);
    oRelDiv11.innerHTML = '';
}
