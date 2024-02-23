let originWidth12; // 图片原始宽度
let originHeigh12; // 图片原始高度
let container12 = document.getElementById('container12');
let imgDiv12 = document.getElementById('imgDiv12');    // 存放mycanvas
//let btnDiv = document.getElementById('btnDiv1');
let clipImgDiv12 = document.getElementById('clipImgDiv12');    // 显示裁剪所获的图片
var oRelDiv12 = document.createElement("div"); // 截图框
var scaleX12 = 1;// 图片宽度缩放比例（当前实际/原始）
var scaleY12 = 1;  // 图片高度缩放比例（当前实际/原始）
var bgwidth12 = 417;
var bgheight12 = 220;

var img12 = new Image();
//拖拽与拉伸方法
//拖拽拉伸所需参数
let params12 = {
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
container12.style.display = 'flex';
container12.style.flexDirection = 'column';
//btnDiv.style.marginBottom = '20px';
//btnDiv.style.height = '120px';
imgDiv12.style.marginBottom = '20px';

/**********************************************************************/
$(function () {
    ExecuteCanvas12();
})


function ExecuteCanvas12() {
    // 创建canvas，用于显示被裁剪图片
    var myCanvas12 = document.createElement('canvas');
    myCanvas12.setAttribute('id', 'myCanvas12');
    myCanvas12.style.display = 'block';
    /*myCanvas.style.position = 'absolute';*/
    myCanvas12.width = bgwidth12;
    myCanvas12.height = bgheight12;
    myCanvas12.style.border = "1px solid #d3d3d3";
    myCanvas12.innerText = '您的浏览器不支持 HTML5 canvas 标签。';
    myCanvas12.style.zIndex = 'auto';

    var ctx12 = myCanvas12.getContext('2d');

    // 被裁剪图片

    img12.src = image12;
    img12.setAttribute('id', 'img');
    img12.width = bgwidth12;
    img12.height = bgheight12;
    img12.onload = function () {
        console.log('onload()执行...');
        ctx12.drawImage(img12, 0, 0, bgwidth12, bgheight12);
        originWidth12 = img12.naturalWidth;
        originHeight12 = img12.naturalHeight;
        console.log('图片原始宽度=', originWidth12);
        console.log('图片原始高度=', originHeight12);

        cutImg12();
    };
    /**********************************************************************/

    imgDiv12.appendChild(myCanvas12);
}

// 获取指定元素DOM
const ID12 = function (id) {
    return document.getElementById(id);
};

//获取相关CSS属性方法
let getCss12 = function (o, key) {
    return o.currentStyle ? o.currentStyle[key] : document.defaultView.getComputedStyle(o, false)[key];
};


// 裁剪得到的图片
let clipImg12 = new Image();
clipImg12.src = '';
clipImg12.style.height = '100px';
clipImg12.style.width = '100px';
clipImg12.alt = '裁剪获得图片...';

function cutImg12() {
    var clickFlag = false;
    // 获取canvas中图片实际大小
    var iCurWidth = img12.width;
    var iCurHeight = img12.height;
    console.log('图片当前实际宽度=', iCurWidth);
    console.log('图片当前实际高度=', iCurHeight);

    // 可调整截图框
    oRelDiv12.innerHTML = '';
    oRelDiv12.style.position = "absolute";
    oRelDiv12.style.width = iCurWidth + "px";
    oRelDiv12.style.height = iCurHeight + "px";
    oRelDiv12.style.top ="377px";// myCanvas12.offsetTop + 'px';
    console.log('oRelDiv.style.top = ', oRelDiv12.style.top);
    oRelDiv12.id = "cropContainer12";

    var iOrigWidth = originWidth12;
    var iOrigHeight = originHeight12;
    scaleX12 = iCurWidth / iOrigWidth; // 图片宽度缩放比例（当前实际/原始）
    scaleY12 = iCurHeight / iOrigHeight;  // 图片高度缩放比例（当前实际/原始）
    console.log('图片横向（宽度）缩放比=', scaleX12);
    console.log('图片纵向（高度）缩放比=', scaleY12);

    // 将oRelDiv插入到myCanvas前
    myCanvas12.parentNode.insertBefore(oRelDiv12, myCanvas12);

    //初始化坐标与剪裁高宽
   if($("#up12").val()=="") {$("#up12").val(0);}
   if($("#left12").val()=="") {$("#left12").val(0);}
   if($("#right12").val()=="") {$("#right12").val(100);}
   if($("#down12").val()=="") {$("#down12").val(100);}

    var cropW=parseInt($("#right12").val())-parseInt($("#left12").val());
    var cropH=parseInt($("#down12").val())-parseInt($("#up12").val());
    var posY = parseInt($("#up12").val());
    var posX = parseInt($("#left12").val());
 
    oRelDiv12.innerHTML = '<div id="zxxCropBox12" style="height:' + cropH + 'px; width:' + cropW + 'px; position:absolute; left:' +
        posX + 'px; top:' + posY + 'px; border:1px solid black;">' +
        '<div id="zxxDragBg12" style="height:100%; background:white; opacity:0.3; filter:alpha(opacity=30); cursor:move"></div>' +
        '<div id="dragLeftTop12" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; top:-3px; cursor:nw-resize;"></div>' +
        '<div id="dragLeftBot12" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; bottom:-3px; cursor:sw-resize;"></div>' +
        '<div id="dragRightTop12" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; top:-3px; cursor:ne-resize;"></div>' +
        '<div id="dragRightBot12" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; bottom:-3px; cursor:se-resize;"></div>' +
        '<div id="dragTopCenter12" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; top:-3px; left:50%; margin-left:-3px; cursor:n-resize;"></div>' +
        '<div id="dragBotCenter12" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; bottom:-3px; left:50%; margin-left:-3px; cursor:s-resize;"></div>' +
        '<div id="dragRightCenter12" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; top:50%; margin-top:-3px; cursor:e-resize;"></div> ' +
        '<div id="dragLeftCenter12" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; top:50%; margin-top:-3px; cursor:w-resize;"></div>' +
        '</div>';
    if($("#up12").val()=="")
    $("#up12").val(0);
  if($("#left12").val()=="")
  $("#left12").val(0);
  if($("#down12").val()=="")
  $("#down12").val(100);
  if($("#right12").val()=="")
  $("#right12").val(100);
    ID("cropImageWidth12").value = parseInt(ID("zxxCropBox12").style.width);
    ID("cropImageHeight12").value = parseInt(ID("zxxCropBox12").style.height);

    var startDrag12 = function (point, target, kind) {
        //point是拉伸点，target是被拉伸的目标，其高度及位置会发生改变
        //此处的target与上面拖拽的target是同一目标，故其params.left,params.top可以共用，也必须共用
        //初始化宽高
        params12.width = getCss12(target, "width");
        params12.height = getCss12(target, "height");
        //初始化坐标
        if (getCss12(target, "left") !== "auto") {
            params12.left = getCss12(target, "left");
        }
        if (getCss12(target, "top") !== "auto") {
            params12.top = getCss12(target, "top");
        }
        //target是移动对象
        point.onmousedown = function (event) {
            params12.kind = kind;
            params12.flag = true;
            clickFlag = true;
            if (!event) {
                event = window.event;
            }
            var e = event;
            params12.currentX = e.clientX;  //鼠标按下时坐标x轴
            params12.currentY = e.clientY;  //鼠标按下时坐标y轴
            /*console.log('params.currentX=', params.currentX);
            console.log('params.currentY=', params.currentY);*/
            //防止IE文字选中，有助于拖拽平滑
            point.onselectstart = function () {
                return false;
            };

            document.onmousemove = function (event) {
                let e = event ? event : window.event;
                clickFlag = false;
                if (params12.flag) {
                    var nowX = e.clientX; // 鼠标移动时x坐标
                    var nowY = e.clientY;   // 鼠标移动时y坐标
                    var disX = nowX - params12.currentX;  // 鼠标x方向移动距离
                    var disY = nowY - params12.currentY;  // 鼠标y方向移动距离
                    if (params12.kind === "n") {
                        //上拉伸
                        //高度增加或减小，位置上下移动
                        if (parseInt(params12.top) + disY > 0) {
                            target.style.top = parseInt(params12.top) + disY + "px";
                            target.style.height = parseInt(params12.height) - disY + "px";
                        }
                    } else if (params12.kind === "w") { //左拉伸
                        if (parseInt(params12.left) + disX > 0) {
                            target.style.left = parseInt(params12.left) + disX + "px";
                            target.style.width = parseInt(params12.width) - disX + "px";
                        }
                    } else if (params12.kind === "e") { //右拉伸
                        if (parseInt(params12.width) + disX + parseInt(params12.left) < bgwidth12)
                           target.style.width = parseInt(params12.width) + disX + "px";
                    } else if (params12.kind === "s") { //下拉伸
                        if (parseInt(params12.height) + disY + parseInt(params12.top) < bgheight12)
                            target.style.height = parseInt(params12.height) + disY + "px";
                    } else if (params12.kind === "nw") { //左上拉伸
                        if (parseInt(params12.left) + disX > 0 && parseInt(params12.top) + disY > 0) {
                            target.style.left = parseInt(params12.left) + disX + "px";
                            target.style.width = parseInt(params12.width) - disX + "px";
                            target.style.top = parseInt(params12.top) + disY + "px";
                            target.style.height = parseInt(params12.height) - disY + "px";
                        }
                    } else if (params12.kind === "ne") { //右上拉伸
                        if (parseInt(params12.width) + disX + parseInt(params12.left) < bgwidth12 && parseInt(params12.top) + disY > 0) {
                            target.style.top = parseInt(params12.top) + disY + "px";
                            target.style.height = parseInt(params12.height) - disY + "px";
                            target.style.width = parseInt(params12.width) + disX + "px";
                        }
                    } else if (params12.kind === "sw") { //左下拉伸
                        if (parseInt(params12.left) + disX > 0 && parseInt(params12.height) + disY + parseInt(params12.top) < bgheight12) {
                            target.style.left = parseInt(params12.left) + disX + "px";
                            target.style.width = parseInt(params12.width) - disX + "px";
                            target.style.height = parseInt(params12.height) + disY + "px";
                        }
                    } else if (params12.kind === "se") { //右下拉伸
                        if (parseInt(params12.width) + disX + parseInt(params12.left) < bgwidth12 && parseInt(params12.height) + disY + parseInt(params12.top) < bgheight12) {
                            target.style.width = parseInt(params12.width) + disX + "px";
                            target.style.height = parseInt(params12.height) + disY + "px";
                        }
                    } else { //移动
                        if (parseInt(params12.left) + disX < 0) { target.style.left = 0 + "px"; }//最左侧为0,小于0阻止移动
                        //最右侧，417-截图框宽度为left最大距离，超过阻止移动
                        else if (parseInt(params12.left) + disX + parseInt(ID("zxxCropBox12").style.width) > bgwidth12) { target.style.left = bgwidth12 - parseInt(ID("zxxCropBox12").style.width) + "px"; }
                        else target.style.left = parseInt(params12.left) + disX + "px";//正常范围
                        if (parseInt(params12.top) + disY < 0) { target.style.top = 0 }//高度小于0,阻止向上移动
                        else if (parseInt(params12.top) + disY + parseInt(ID("zxxCropBox12").style.height) > bgheight12)//移动高度+控件高度超过220，阻止移动
                        { target.style.top = bgheight12 - parseInt(ID("zxxCropBox12").style.height) + "px"; }
                        else { target.style.top = parseInt(params12.top) + disY + "px"; }//正常高度范围内
                    }
                }

                document.onmouseup = function () {
                    document.onmousemove = null;
                    document.onmouseup = null;
                    params12.flag = false;
                    if (getCss12(target, "left") !== "auto") {
                        params12.left = getCss12(target, "left");
                    }
                    if (getCss12(target, "top") !== "auto") {
                        params12.top = getCss12(target, "top");
                    }
                    params12.width = getCss12(target, "width");
                    params12.height = getCss12(target, "height");
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
                    var right = posX + parseInt(ID("zxxCropBox12").style.width);
                    if (right < 0) right = 0;
                    var down = posY + parseInt(ID("zxxCropBox12").style.height);
                    if (down < 0) down = 0;
                    ID("left12").value = posX;
                    ID("up12").value = posY;
                    ID("right12").value = right;
                    ID("down12").value = down;
                    ID("cropImageWidth12").value = parseInt(ID("zxxCropBox12").style.width);
                    ID("cropImageHeight12").value = parseInt(ID("zxxCropBox12").style.height);
                };
            }
        };
    };

    //绑定拖拽
    startDrag12(ID12("zxxDragBg12"), ID12("zxxCropBox12"), "drag");
    //绑定拉伸
    startDrag12(ID12("dragLeftTop12"), ID12("zxxCropBox12"), "nw");
    startDrag12(ID12("dragLeftBot12"), ID12("zxxCropBox12"), "sw");
    startDrag12(ID12("dragRightTop12"), ID12("zxxCropBox12"), "ne");
    startDrag12(ID12("dragRightBot12"), ID12("zxxCropBox12"), "se");
    startDrag12(ID12("dragTopCenter12"), ID12("zxxCropBox12"), "n");
    startDrag12(ID12("dragBotCenter12"), ID12("zxxCropBox12"), "s");
    startDrag12(ID12("dragRightCenter12"), ID12("zxxCropBox12"), "e");
    startDrag12(ID12("dragLeftCenter12"), ID12("zxxCropBox12"), "w");


    //图片不能被选中，目的在于使拖拽顺滑
    ID12("myCanvas12").onselectstart = function () {
        return false;
    };
    img12.onselectstart = function () {
        return false;
    };
}

//确认裁剪
function RequeryCrop12() {
    var x = document.getElementById("left12").value;
    var y = document.getElementById("up12").value;
    var w = document.getElementById("cropImageWidth12").value;
    var h = document.getElementById("cropImageHeight12").value;
    console.log('cropImage(img,', x, ',', y, ',', parseInt(w), ',', parseInt(h), ')');
   // cropImage12(img12, x / scaleX12, y / scaleY12, parseInt(w) / scaleX12, parseInt(h) / scaleY12);
}


function cropImage12(img, cropPosX, cropPosY, width, height) {
    /*var cropContainer = ID("cropContainer");
    cropContainer.parentNode.removeChild(cropContainer);*/
    /*ctx.clearRect(0, 0, myCanvas.width, myCanvas.height);*/
    //sx,sy 是相对于图片的坐标。巨坑

    $("#imgDiv12").html('');
    var myCanvas = document.createElement('canvas');
    myCanvas.setAttribute('id', 'myCanvas12');
    myCanvas.style.display = 'block';
    /*myCanvas.style.position = 'absolute';*/
    myCanvas.width = width * scaleX12;
    myCanvas.height = height * scaleY12;
    myCanvas.style.border = "1px solid #d3d3d3";
    myCanvas.innerText = '您的浏览器不支持 HTML5 canvas 标签。';
    myCanvas.style.zIndex = 'auto';

    var newCtx = myCanvas.getContext('2d');
    newCtx.drawImage(img12, cropPosX, cropPosY, width, height, 0, 0, width * scaleX12, height * scaleY12);

    // canvas转化为图片
    var newImage = new Image();
    newImage.src = myCanvas.toDataURL("image/png");
    //newImage.style.marginLeft = '5px';
    newImage.width = bgwidth12;
    newImage.height = bgheight12;
    //clipImgDiv.appendChild(newImage);
    imgDiv12.appendChild(newImage);
    oRelDiv12.innerHTML = '';
}
