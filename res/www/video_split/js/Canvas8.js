let originWidth8; // 图片原始宽度
let originHeigh8; // 图片原始高度
let container8 = document.getElementById('container8');
let imgDiv8 = document.getElementById('imgDiv8');    // 存放mycanvas
//let btnDiv = document.getElementById('btnDiv1');
let clipImgDiv8 = document.getElementById('clipImgDiv8');    // 显示裁剪所获的图片
var oRelDiv8 = document.createElement("div"); // 截图框
var scaleX8 = 1;// 图片宽度缩放比例（当前实际/原始）
var scaleY8 = 1;  // 图片高度缩放比例（当前实际/原始）
var bgwidth8 = 417;
var bgheight8 = 220;

var img8 = new Image();
//拖拽与拉伸方法
//拖拽拉伸所需参数
let params8 = {
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
container8.style.display = 'flex';
container8.style.flexDirection = 'column';
//btnDiv.style.marginBottom = '20px';
//btnDiv.style.height = '80px';
imgDiv8.style.marginBottom = '20px';

/**********************************************************************/
$(function () {
    ExecuteCanvas8();
})


function ExecuteCanvas8() {
    // 创建canvas，用于显示被裁剪图片
    var myCanvas8 = document.createElement('canvas');
    myCanvas8.setAttribute('id', 'myCanvas8');
    myCanvas8.style.display = 'block';
    /*myCanvas.style.position = 'absolute';*/
    myCanvas8.width = bgwidth8;
    myCanvas8.height = bgheight8;
    myCanvas8.style.border = "1px solid #d3d3d3";
    myCanvas8.innerText = '您的浏览器不支持 HTML5 canvas 标签。';
    myCanvas8.style.zIndex = 'auto';

    var ctx8 = myCanvas8.getContext('2d');

    // 被裁剪图片

    img8.src = image8;
    img8.setAttribute('id', 'img');
    img8.width = bgwidth8;
    img8.height = bgheight8;
    img8.onload = function () {
        console.log('onload()执行...');
        ctx8.drawImage(img8, 0, 0, bgwidth8, bgheight8);
        originWidth8 = img8.naturalWidth;
        originHeight8 = img8.naturalHeight;
        console.log('图片原始宽度=', originWidth8);
        console.log('图片原始高度=', originHeight8);

        cutImg8();
    };
    /**********************************************************************/

    imgDiv8.appendChild(myCanvas8);
}

// 获取指定元素DOM
const ID8 = function (id) {
    return document.getElementById(id);
};

//获取相关CSS属性方法
let getCss8 = function (o, key) {
    return o.currentStyle ? o.currentStyle[key] : document.defaultView.getComputedStyle(o, false)[key];
};


// 裁剪得到的图片
let clipImg8 = new Image();
clipImg8.src = '';
clipImg8.style.height = '100px';
clipImg8.style.width = '100px';
clipImg8.alt = '裁剪获得图片...';

function cutImg8() {
    var clickFlag = false;
    // 获取canvas中图片实际大小
    var iCurWidth = img8.width;
    var iCurHeight = img8.height;
    console.log('图片当前实际宽度=', iCurWidth);
    console.log('图片当前实际高度=', iCurHeight);

    // 可调整截图框
    oRelDiv8.innerHTML = '';
    oRelDiv8.style.position = "absolute";
    oRelDiv8.style.width = iCurWidth + "px";
    oRelDiv8.style.height = iCurHeight + "px";
    oRelDiv8.style.top ="377px";// myCanvas8.offsetTop + 'px';
    console.log('oRelDiv.style.top = ', oRelDiv8.style.top);
    oRelDiv8.id = "cropContainer8";

    var iOrigWidth = originWidth8;
    var iOrigHeight = originHeight8;
    scaleX8 = iCurWidth / iOrigWidth; // 图片宽度缩放比例（当前实际/原始）
    scaleY8 = iCurHeight / iOrigHeight;  // 图片高度缩放比例（当前实际/原始）
    console.log('图片横向（宽度）缩放比=', scaleX8);
    console.log('图片纵向（高度）缩放比=', scaleY8);

    // 将oRelDiv插入到myCanvas前
    myCanvas8.parentNode.insertBefore(oRelDiv8, myCanvas8);

    //初始化坐标与剪裁高宽
   if($("#up8").val()=="") {$("#up8").val(0);}
   if($("#left8").val()=="") {$("#left8").val(0);}
   if($("#right8").val()=="") {$("#right8").val(100);}
   if($("#down8").val()=="") {$("#down8").val(100);}

    var cropW=parseInt($("#right8").val())-parseInt($("#left8").val());
    var cropH=parseInt($("#down8").val())-parseInt($("#up8").val());
    var posY = parseInt($("#up8").val());
    var posX = parseInt($("#left8").val());
 
    oRelDiv8.innerHTML = '<div id="zxxCropBox8" style="height:' + cropH + 'px; width:' + cropW + 'px; position:absolute; left:' +
        posX + 'px; top:' + posY + 'px; border:1px solid black;">' +
        '<div id="zxxDragBg8" style="height:100%; background:white; opacity:0.3; filter:alpha(opacity=30); cursor:move"></div>' +
        '<div id="dragLeftTop8" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; top:-3px; cursor:nw-resize;"></div>' +
        '<div id="dragLeftBot8" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; bottom:-3px; cursor:sw-resize;"></div>' +
        '<div id="dragRightTop8" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; top:-3px; cursor:ne-resize;"></div>' +
        '<div id="dragRightBot8" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; bottom:-3px; cursor:se-resize;"></div>' +
        '<div id="dragTopCenter8" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; top:-3px; left:50%; margin-left:-3px; cursor:n-resize;"></div>' +
        '<div id="dragBotCenter8" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; bottom:-3px; left:50%; margin-left:-3px; cursor:s-resize;"></div>' +
        '<div id="dragRightCenter8" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; top:50%; margin-top:-3px; cursor:e-resize;"></div> ' +
        '<div id="dragLeftCenter8" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; top:50%; margin-top:-3px; cursor:w-resize;"></div>' +
        '</div>';
    if($("#up8").val()=="")
    $("#up8").val(0);
  if($("#left8").val()=="")
  $("#left8").val(0);
  if($("#down8").val()=="")
  $("#down8").val(100);
  if($("#right8").val()=="")
  $("#right8").val(100);
    ID("cropImageWidth8").value = parseInt(ID("zxxCropBox8").style.width);
    ID("cropImageHeight8").value = parseInt(ID("zxxCropBox8").style.height);

    var startDrag8 = function (point, target, kind) {
        //point是拉伸点，target是被拉伸的目标，其高度及位置会发生改变
        //此处的target与上面拖拽的target是同一目标，故其params.left,params.top可以共用，也必须共用
        //初始化宽高
        params8.width = getCss8(target, "width");
        params8.height = getCss8(target, "height");
        //初始化坐标
        if (getCss8(target, "left") !== "auto") {
            params8.left = getCss8(target, "left");
        }
        if (getCss8(target, "top") !== "auto") {
            params8.top = getCss8(target, "top");
        }
        //target是移动对象
        point.onmousedown = function (event) {
            params8.kind = kind;
            params8.flag = true;
            clickFlag = true;
            if (!event) {
                event = window.event;
            }
            var e = event;
            params8.currentX = e.clientX;  //鼠标按下时坐标x轴
            params8.currentY = e.clientY;  //鼠标按下时坐标y轴
            /*console.log('params.currentX=', params.currentX);
            console.log('params.currentY=', params.currentY);*/
            //防止IE文字选中，有助于拖拽平滑
            point.onselectstart = function () {
                return false;
            };

            document.onmousemove = function (event) {
                let e = event ? event : window.event;
                clickFlag = false;
                if (params8.flag) {
                    var nowX = e.clientX; // 鼠标移动时x坐标
                    var nowY = e.clientY;   // 鼠标移动时y坐标
                    var disX = nowX - params8.currentX;  // 鼠标x方向移动距离
                    var disY = nowY - params8.currentY;  // 鼠标y方向移动距离
                    if (params8.kind === "n") {
                        //上拉伸
                        //高度增加或减小，位置上下移动
                        if (parseInt(params8.top) + disY > 0) {
                            target.style.top = parseInt(params8.top) + disY + "px";
                            target.style.height = parseInt(params8.height) - disY + "px";
                        }
                    } else if (params8.kind === "w") { //左拉伸
                        if (parseInt(params8.left) + disX > 0) {
                            target.style.left = parseInt(params8.left) + disX + "px";
                            target.style.width = parseInt(params8.width) - disX + "px";
                        }
                    } else if (params8.kind === "e") { //右拉伸
                        if (parseInt(params8.width) + disX + parseInt(params8.left) < bgwidth8)
                           target.style.width = parseInt(params8.width) + disX + "px";
                    } else if (params8.kind === "s") { //下拉伸
                        if (parseInt(params8.height) + disY + parseInt(params8.top) < bgheight8)
                            target.style.height = parseInt(params8.height) + disY + "px";
                    } else if (params8.kind === "nw") { //左上拉伸
                        if (parseInt(params8.left) + disX > 0 && parseInt(params8.top) + disY > 0) {
                            target.style.left = parseInt(params8.left) + disX + "px";
                            target.style.width = parseInt(params8.width) - disX + "px";
                            target.style.top = parseInt(params8.top) + disY + "px";
                            target.style.height = parseInt(params8.height) - disY + "px";
                        }
                    } else if (params8.kind === "ne") { //右上拉伸
                        if (parseInt(params8.width) + disX + parseInt(params8.left) < bgwidth8 && parseInt(params8.top) + disY > 0) {
                            target.style.top = parseInt(params8.top) + disY + "px";
                            target.style.height = parseInt(params8.height) - disY + "px";
                            target.style.width = parseInt(params8.width) + disX + "px";
                        }
                    } else if (params8.kind === "sw") { //左下拉伸
                        if (parseInt(params8.left) + disX > 0 && parseInt(params8.height) + disY + parseInt(params8.top) < bgheight8) {
                            target.style.left = parseInt(params8.left) + disX + "px";
                            target.style.width = parseInt(params8.width) - disX + "px";
                            target.style.height = parseInt(params8.height) + disY + "px";
                        }
                    } else if (params8.kind === "se") { //右下拉伸
                        if (parseInt(params8.width) + disX + parseInt(params8.left) < bgwidth8 && parseInt(params8.height) + disY + parseInt(params8.top) < bgheight8) {
                            target.style.width = parseInt(params8.width) + disX + "px";
                            target.style.height = parseInt(params8.height) + disY + "px";
                        }
                    } else { //移动
                        if (parseInt(params8.left) + disX < 0) { target.style.left = 0 + "px"; }//最左侧为0,小于0阻止移动
                        //最右侧，417-截图框宽度为left最大距离，超过阻止移动
                        else if (parseInt(params8.left) + disX + parseInt(ID("zxxCropBox8").style.width) > bgwidth8) { target.style.left = bgwidth8 - parseInt(ID("zxxCropBox8").style.width) + "px"; }
                        else target.style.left = parseInt(params8.left) + disX + "px";//正常范围
                        if (parseInt(params8.top) + disY < 0) { target.style.top = 0 }//高度小于0,阻止向上移动
                        else if (parseInt(params8.top) + disY + parseInt(ID("zxxCropBox8").style.height) > bgheight8)//移动高度+控件高度超过220，阻止移动
                        { target.style.top = bgheight8 - parseInt(ID("zxxCropBox8").style.height) + "px"; }
                        else { target.style.top = parseInt(params8.top) + disY + "px"; }//正常高度范围内
                    }
                }

                document.onmouseup = function () {
                    document.onmousemove = null;
                    document.onmouseup = null;
                    params8.flag = false;
                    if (getCss8(target, "left") !== "auto") {
                        params8.left = getCss8(target, "left");
                    }
                    if (getCss8(target, "top") !== "auto") {
                        params8.top = getCss8(target, "top");
                    }
                    params8.width = getCss8(target, "width");
                    params8.height = getCss8(target, "height");
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
                    var right = posX + parseInt(ID("zxxCropBox8").style.width);
                    if (right < 0) right = 0;
                    var down = posY + parseInt(ID("zxxCropBox8").style.height);
                    if (down < 0) down = 0;
                    ID("left8").value = posX;
                    ID("up8").value = posY;
                    ID("right8").value = right;
                    ID("down8").value = down;
                    ID("cropImageWidth8").value = parseInt(ID("zxxCropBox8").style.width);
                    ID("cropImageHeight8").value = parseInt(ID("zxxCropBox8").style.height);
                };
            }
        };
    };

    //绑定拖拽
    startDrag8(ID8("zxxDragBg8"), ID8("zxxCropBox8"), "drag");
    //绑定拉伸
    startDrag8(ID8("dragLeftTop8"), ID8("zxxCropBox8"), "nw");
    startDrag8(ID8("dragLeftBot8"), ID8("zxxCropBox8"), "sw");
    startDrag8(ID8("dragRightTop8"), ID8("zxxCropBox8"), "ne");
    startDrag8(ID8("dragRightBot8"), ID8("zxxCropBox8"), "se");
    startDrag8(ID8("dragTopCenter8"), ID8("zxxCropBox8"), "n");
    startDrag8(ID8("dragBotCenter8"), ID8("zxxCropBox8"), "s");
    startDrag8(ID8("dragRightCenter8"), ID8("zxxCropBox8"), "e");
    startDrag8(ID8("dragLeftCenter8"), ID8("zxxCropBox8"), "w");


    //图片不能被选中，目的在于使拖拽顺滑
    ID8("myCanvas8").onselectstart = function () {
        return false;
    };
    img8.onselectstart = function () {
        return false;
    };
}

//确认裁剪
function RequeryCrop8() {
    var x = document.getElementById("left8").value;
    var y = document.getElementById("up8").value;
    var w = document.getElementById("cropImageWidth8").value;
    var h = document.getElementById("cropImageHeight8").value;
    console.log('cropImage(img,', x, ',', y, ',', parseInt(w), ',', parseInt(h), ')');
   // cropImage8(img8, x / scaleX8, y / scaleY8, parseInt(w) / scaleX8, parseInt(h) / scaleY8);
}


function cropImage8(img, cropPosX, cropPosY, width, height) {
    /*var cropContainer = ID("cropContainer");
    cropContainer.parentNode.removeChild(cropContainer);*/
    /*ctx.clearRect(0, 0, myCanvas.width, myCanvas.height);*/
    //sx,sy 是相对于图片的坐标。巨坑

    $("#imgDiv8").html('');
    var myCanvas = document.createElement('canvas');
    myCanvas.setAttribute('id', 'myCanvas8');
    myCanvas.style.display = 'block';
    /*myCanvas.style.position = 'absolute';*/
    myCanvas.width = width * scaleX8;
    myCanvas.height = height * scaleY8;
    myCanvas.style.border = "1px solid #d3d3d3";
    myCanvas.innerText = '您的浏览器不支持 HTML5 canvas 标签。';
    myCanvas.style.zIndex = 'auto';

    var newCtx = myCanvas.getContext('2d');
    newCtx.drawImage(img8, cropPosX, cropPosY, width, height, 0, 0, width * scaleX8, height * scaleY8);

    // canvas转化为图片
    var newImage = new Image();
    newImage.src = myCanvas.toDataURL("image/png");
    //newImage.style.marginLeft = '5px';
    newImage.width = bgwidth8;
    newImage.height = bgheight8;
    //clipImgDiv.appendChild(newImage);
    imgDiv8.appendChild(newImage);
    oRelDiv8.innerHTML = '';
}
