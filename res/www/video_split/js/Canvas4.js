let originWidth4; // 图片原始宽度
let originHeigh4; // 图片原始高度
let container4 = document.getElementById('container4');
let imgDiv4 = document.getElementById('imgDiv4');    // 存放mycanvas
//let btnDiv = document.getElementById('btnDiv1');
let clipImgDiv4 = document.getElementById('clipImgDiv4');    // 显示裁剪所获的图片
var oRelDiv4 = document.createElement("div"); // 截图框
var scaleX4 = 1;// 图片宽度缩放比例（当前实际/原始）
var scaleY4 = 1;  // 图片高度缩放比例（当前实际/原始）
var bgwidth4 = 417;
var bgheight4 = 220;

var img4 = new Image();
//拖拽与拉伸方法
//拖拽拉伸所需参数
let params4 = {
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
container4.style.display = 'flex';
container4.style.flexDirection = 'column';
//btnDiv.style.marginBottom = '20px';
//btnDiv.style.height = '40px';
imgDiv4.style.marginBottom = '20px';

/**********************************************************************/
$(function () {
    ExecuteCanvas4();
})

var myCanvas4;
function ExecuteCanvas4() {
    // 创建canvas，用于显示被裁剪图片
 myCanvas4 = document.createElement('canvas');
    myCanvas4.setAttribute('id', 'myCanvas4');
    myCanvas4.style.display = 'block';
    /*myCanvas.style.position = 'absolute';*/
    myCanvas4.width = bgwidth4;
    myCanvas4.height = bgheight4;
    myCanvas4.style.border = "1px solid #d3d3d3";
    myCanvas4.innerText = '您的浏览器不支持 HTML5 canvas 标签。';
    myCanvas4.style.zIndex = 'auto';

    var ctx4 = myCanvas4.getContext('2d');

    // 被裁剪图片

    img4.src = image4;
    img4.setAttribute('id', 'img');
    img4.width = bgwidth4;
    img4.height = bgheight4;
    img4.onload = function () {
        console.log('onload()执行...');
        ctx4.drawImage(img4, 0, 0, bgwidth4, bgheight4);
        originWidth4 = img4.naturalWidth;
        originHeight4 = img4.naturalHeight;
        console.log('图片原始宽度=', originWidth4);
        console.log('图片原始高度=', originHeight4);

        cutImg4();
    };
    /**********************************************************************/

    imgDiv4.appendChild(myCanvas4);
}

// 获取指定元素DOM
const ID4 = function (id) {
    return document.getElementById(id);
};

//获取相关CSS属性方法
let getCss4 = function (o, key) {
    return o.currentStyle ? o.currentStyle[key] : document.defaultView.getComputedStyle(o, false)[key];
};


// 裁剪得到的图片
let clipImg4 = new Image();
clipImg4.src = '';
clipImg4.style.height = '100px';
clipImg4.style.width = '100px';
clipImg4.alt = '裁剪获得图片...';

function cutImg4() {
    var clickFlag = false;
    // 获取canvas中图片实际大小
    var iCurWidth = img4.width;
    var iCurHeight = img4.height;
    console.log('图片当前实际宽度=', iCurWidth);
    console.log('图片当前实际高度=', iCurHeight);

    // 可调整截图框
    oRelDiv4.innerHTML = '';
    oRelDiv4.style.position = "absolute";
    oRelDiv4.style.width = iCurWidth + "px";
    oRelDiv4.style.height = iCurHeight + "px";
    oRelDiv4.style.top = "377px";//myCanvas4.offsetTop + 'px';
    console.log('oRelDiv.style.top = ', oRelDiv4.style.top);
    oRelDiv4.id = "cropContainer4";

    var iOrigWidth = originWidth4;
    var iOrigHeight = originHeight4;
    scaleX4 = iCurWidth / iOrigWidth; // 图片宽度缩放比例（当前实际/原始）
    scaleY4 = iCurHeight / iOrigHeight;  // 图片高度缩放比例（当前实际/原始）
    console.log('图片横向（宽度）缩放比=', scaleX4);
    console.log('图片纵向（高度）缩放比=', scaleY4);

    // 将oRelDiv插入到myCanvas前
    myCanvas4.parentNode.insertBefore(oRelDiv4, myCanvas4);

    //初始化坐标与剪裁高宽
   if($("#up4").val()=="") {$("#up4").val(0);}
   if($("#left4").val()=="") {$("#left4").val(0);}
   if($("#right4").val()=="") {$("#right4").val(100);}
   if($("#down4").val()=="") {$("#down4").val(100);}

    var cropW=parseInt($("#right4").val())-parseInt($("#left4").val());
    var cropH=parseInt($("#down4").val())-parseInt($("#up4").val());
    var posY = parseInt($("#up4").val());
    var posX = parseInt($("#left4").val());
 
    oRelDiv4.innerHTML = '<div id="zxxCropBox4" style="height:' + cropH + 'px; width:' + cropW + 'px; position:absolute; left:' +
        posX + 'px; top:' + posY + 'px; border:1px solid black;">' +
        '<div id="zxxDragBg4" style="height:100%; background:white; opacity:0.3; filter:alpha(opacity=30); cursor:move"></div>' +
        '<div id="dragLeftTop4" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; top:-3px; cursor:nw-resize;"></div>' +
        '<div id="dragLeftBot4" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; bottom:-3px; cursor:sw-resize;"></div>' +
        '<div id="dragRightTop4" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; top:-3px; cursor:ne-resize;"></div>' +
        '<div id="dragRightBot4" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; bottom:-3px; cursor:se-resize;"></div>' +
        '<div id="dragTopCenter4" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; top:-3px; left:50%; margin-left:-3px; cursor:n-resize;"></div>' +
        '<div id="dragBotCenter4" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; bottom:-3px; left:50%; margin-left:-3px; cursor:s-resize;"></div>' +
        '<div id="dragRightCenter4" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; top:50%; margin-top:-3px; cursor:e-resize;"></div> ' +
        '<div id="dragLeftCenter4" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; top:50%; margin-top:-3px; cursor:w-resize;"></div>' +
        '</div>';
    if($("#up4").val()=="")
    $("#up4").val(0);
  if($("#left4").val()=="")
  $("#left4").val(0);
  if($("#down4").val()=="")
  $("#down4").val(100);
  if($("#right4").val()=="")
  $("#right4").val(100);
    ID("cropImageWidth4").value = parseInt(ID("zxxCropBox4").style.width);
    ID("cropImageHeight4").value = parseInt(ID("zxxCropBox4").style.height);

    var startDrag4 = function (point, target, kind) {
        //point是拉伸点，target是被拉伸的目标，其高度及位置会发生改变
        //此处的target与上面拖拽的target是同一目标，故其params.left,params.top可以共用，也必须共用
        //初始化宽高
        params4.width = getCss4(target, "width");
        params4.height = getCss4(target, "height");
        //初始化坐标
        if (getCss4(target, "left") !== "auto") {
            params4.left = getCss4(target, "left");
        }
        if (getCss4(target, "top") !== "auto") {
            params4.top = getCss4(target, "top");
        }
        //target是移动对象
        point.onmousedown = function (event) {
            params4.kind = kind;
            params4.flag = true;
            clickFlag = true;
            if (!event) {
                event = window.event;
            }
            var e = event;
            params4.currentX = e.clientX;  //鼠标按下时坐标x轴
            params4.currentY = e.clientY;  //鼠标按下时坐标y轴
            /*console.log('params.currentX=', params.currentX);
            console.log('params.currentY=', params.currentY);*/
            //防止IE文字选中，有助于拖拽平滑
            point.onselectstart = function () {
                return false;
            };

            document.onmousemove = function (event) {
                let e = event ? event : window.event;
                clickFlag = false;
                if (params4.flag) {
                    var nowX = e.clientX; // 鼠标移动时x坐标
                    var nowY = e.clientY;   // 鼠标移动时y坐标
                    var disX = nowX - params4.currentX;  // 鼠标x方向移动距离
                    var disY = nowY - params4.currentY;  // 鼠标y方向移动距离
                    if (params4.kind === "n") {
                        //上拉伸
                        //高度增加或减小，位置上下移动
                        if (parseInt(params4.top) + disY > 0) {
                            target.style.top = parseInt(params4.top) + disY + "px";
                            target.style.height = parseInt(params4.height) - disY + "px";
                        }
                    } else if (params4.kind === "w") { //左拉伸
                        if (parseInt(params4.left) + disX > 0) {
                            target.style.left = parseInt(params4.left) + disX + "px";
                            target.style.width = parseInt(params4.width) - disX + "px";
                        }
                    } else if (params4.kind === "e") { //右拉伸
                        if (parseInt(params4.width) + disX + parseInt(params4.left) < bgwidth4)
                           target.style.width = parseInt(params4.width) + disX + "px";
                    } else if (params4.kind === "s") { //下拉伸
                        if (parseInt(params4.height) + disY + parseInt(params4.top) < bgheight4)
                            target.style.height = parseInt(params4.height) + disY + "px";
                    } else if (params4.kind === "nw") { //左上拉伸
                        if (parseInt(params4.left) + disX > 0 && parseInt(params4.top) + disY > 0) {
                            target.style.left = parseInt(params4.left) + disX + "px";
                            target.style.width = parseInt(params4.width) - disX + "px";
                            target.style.top = parseInt(params4.top) + disY + "px";
                            target.style.height = parseInt(params4.height) - disY + "px";
                        }
                    } else if (params4.kind === "ne") { //右上拉伸
                        if (parseInt(params4.width) + disX + parseInt(params4.left) < bgwidth4 && parseInt(params4.top) + disY > 0) {
                            target.style.top = parseInt(params4.top) + disY + "px";
                            target.style.height = parseInt(params4.height) - disY + "px";
                            target.style.width = parseInt(params4.width) + disX + "px";
                        }
                    } else if (params4.kind === "sw") { //左下拉伸
                        if (parseInt(params4.left) + disX > 0 && parseInt(params4.height) + disY + parseInt(params4.top) < bgheight4) {
                            target.style.left = parseInt(params4.left) + disX + "px";
                            target.style.width = parseInt(params4.width) - disX + "px";
                            target.style.height = parseInt(params4.height) + disY + "px";
                        }
                    } else if (params4.kind === "se") { //右下拉伸
                        if (parseInt(params4.width) + disX + parseInt(params4.left) < bgwidth4 && parseInt(params4.height) + disY + parseInt(params4.top) < bgheight4) {
                            target.style.width = parseInt(params4.width) + disX + "px";
                            target.style.height = parseInt(params4.height) + disY + "px";
                        }
                    } else { //移动
                        if (parseInt(params4.left) + disX < 0) { target.style.left = 0 + "px"; }//最左侧为0,小于0阻止移动
                        //最右侧，417-截图框宽度为left最大距离，超过阻止移动
                        else if (parseInt(params4.left) + disX + parseInt(ID("zxxCropBox4").style.width) > bgwidth4) { target.style.left = bgwidth4 - parseInt(ID("zxxCropBox4").style.width) + "px"; }
                        else target.style.left = parseInt(params4.left) + disX + "px";//正常范围
                        if (parseInt(params4.top) + disY < 0) { target.style.top = 0 }//高度小于0,阻止向上移动
                        else if (parseInt(params4.top) + disY + parseInt(ID("zxxCropBox4").style.height) > bgheight4)//移动高度+控件高度超过220，阻止移动
                        { target.style.top = bgheight4 - parseInt(ID("zxxCropBox4").style.height) + "px"; }
                        else { target.style.top = parseInt(params4.top) + disY + "px"; }//正常高度范围内
                    }
                }

                document.onmouseup = function () {
                    document.onmousemove = null;
                    document.onmouseup = null;
                    params4.flag = false;
                    if (getCss4(target, "left") !== "auto") {
                        params4.left = getCss4(target, "left");
                    }
                    if (getCss4(target, "top") !== "auto") {
                        params4.top = getCss4(target, "top");
                    }
                    params4.width = getCss4(target, "width");
                    params4.height = getCss4(target, "height");
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
                    var right = posX + parseInt(ID("zxxCropBox4").style.width);
                    if (right < 0) right = 0;
                    var down = posY + parseInt(ID("zxxCropBox4").style.height);
                    if (down < 0) down = 0;
                    ID("left4").value = posX;
                    ID("up4").value = posY;
                    ID("right4").value = right;
                    ID("down4").value = down;
                    ID("cropImageWidth4").value = parseInt(ID("zxxCropBox4").style.width);
                    ID("cropImageHeight4").value = parseInt(ID("zxxCropBox4").style.height);
                };
            }
        };
    };

    //绑定拖拽
    startDrag4(ID4("zxxDragBg4"), ID4("zxxCropBox4"), "drag");
    //绑定拉伸
    startDrag4(ID4("dragLeftTop4"), ID4("zxxCropBox4"), "nw");
    startDrag4(ID4("dragLeftBot4"), ID4("zxxCropBox4"), "sw");
    startDrag4(ID4("dragRightTop4"), ID4("zxxCropBox4"), "ne");
    startDrag4(ID4("dragRightBot4"), ID4("zxxCropBox4"), "se");
    startDrag4(ID4("dragTopCenter4"), ID4("zxxCropBox4"), "n");
    startDrag4(ID4("dragBotCenter4"), ID4("zxxCropBox4"), "s");
    startDrag4(ID4("dragRightCenter4"), ID4("zxxCropBox4"), "e");
    startDrag4(ID4("dragLeftCenter4"), ID4("zxxCropBox4"), "w");


    //图片不能被选中，目的在于使拖拽顺滑
    ID4("myCanvas4").onselectstart = function () {
        return false;
    };
    img4.onselectstart = function () {
        return false;
    };
}

//确认裁剪
function RequeryCrop4() {
    var x = document.getElementById("left4").value;
    var y = document.getElementById("up4").value;
    var w = document.getElementById("cropImageWidth4").value;
    var h = document.getElementById("cropImageHeight4").value;
    console.log('cropImage(img,', x, ',', y, ',', parseInt(w), ',', parseInt(h), ')');
   // cropImage4(img4, x / scaleX4, y / scaleY4, parseInt(w) / scaleX4, parseInt(h) / scaleY4);
}


function cropImage4(img, cropPosX, cropPosY, width, height) {
    /*var cropContainer = ID("cropContainer");
    cropContainer.parentNode.removeChild(cropContainer);*/
    /*ctx.clearRect(0, 0, myCanvas.width, myCanvas.height);*/
    //sx,sy 是相对于图片的坐标。巨坑

    $("#imgDiv4").html('');
    var myCanvas = document.createElement('canvas');
    myCanvas.setAttribute('id', 'myCanvas4');
    myCanvas.style.display = 'block';
    /*myCanvas.style.position = 'absolute';*/
    myCanvas.width = width * scaleX4;
    myCanvas.height = height * scaleY4;
    myCanvas.style.border = "1px solid #d3d3d3";
    myCanvas.innerText = '您的浏览器不支持 HTML5 canvas 标签。';
    myCanvas.style.zIndex = 'auto';

    var newCtx = myCanvas.getContext('2d');
    newCtx.drawImage(img4, cropPosX, cropPosY, width, height, 0, 0, width * scaleX4, height * scaleY4);

    // canvas转化为图片
    var newImage = new Image();
    newImage.src = myCanvas.toDataURL("image/png");
    //newImage.style.marginLeft = '5px';
    newImage.width = bgwidth4;
    newImage.height = bgheight4;
    //clipImgDiv.appendChild(newImage);
    imgDiv4.appendChild(newImage);
    oRelDiv4.innerHTML = '';
}
