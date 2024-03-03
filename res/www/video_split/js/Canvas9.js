let originWidth9; // 图片原始宽度
let originHeigh9; // 图片原始高度
let container9 = document.getElementById('container9');
let imgDiv9 = document.getElementById('imgDiv9');    // 存放mycanvas
//let btnDiv = document.getElementById('btnDiv1');
let clipImgDiv9 = document.getElementById('clipImgDiv9');    // 显示裁剪所获的图片
var oRelDiv9 = document.createElement("div"); // 截图框
var scaleX9 = 1;// 图片宽度缩放比例（当前实际/原始）
var scaleY9 = 1;  // 图片高度缩放比例（当前实际/原始）
var bgwidth9 = 417;
var bgheight9 = 220;

var img9 = new Image();
//拖拽与拉伸方法
//拖拽拉伸所需参数
let params9 = {
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
container9.style.display = 'flex';
container9.style.flexDirection = 'column';
//btnDiv.style.marginBottom = '20px';
//btnDiv.style.height = '90px';
imgDiv9.style.marginBottom = '20px';

/**********************************************************************/
$(function () {
    ExecuteCanvas9();
})


function ExecuteCanvas9() {
    // 创建canvas，用于显示被裁剪图片
    var myCanvas9 = document.createElement('canvas');
    myCanvas9.setAttribute('id', 'myCanvas9');
    myCanvas9.style.display = 'block';
    /*myCanvas.style.position = 'absolute';*/
    myCanvas9.width = bgwidth9;
    myCanvas9.height = bgheight9;
    myCanvas9.style.border = "1px solid #d3d3d3";
    myCanvas9.innerText = '您的浏览器不支持 HTML5 canvas 标签。';
    myCanvas9.style.zIndex = 'auto';

    var ctx9 = myCanvas9.getContext('2d');

    // 被裁剪图片

    img9.src = image9;
    img9.setAttribute('id', 'img');
    img9.width = bgwidth9;
    img9.height = bgheight9;
    img9.onload = function () {
        console.log('onload()执行...');
        ctx9.drawImage(img9, 0, 0, bgwidth9, bgheight9);
        originWidth9 = img9.naturalWidth;
        originHeight9 = img9.naturalHeight;
        console.log('图片原始宽度=', originWidth9);
        console.log('图片原始高度=', originHeight9);

        cutImg9();
    };
    /**********************************************************************/

    imgDiv9.appendChild(myCanvas9);
}

// 获取指定元素DOM
const ID9 = function (id) {
    return document.getElementById(id);
};

//获取相关CSS属性方法
let getCss9 = function (o, key) {
    return o.currentStyle ? o.currentStyle[key] : document.defaultView.getComputedStyle(o, false)[key];
};


// 裁剪得到的图片
let clipImg9 = new Image();
clipImg9.src = '';
clipImg9.style.height = '100px';
clipImg9.style.width = '100px';
clipImg9.alt = '裁剪获得图片...';

function cutImg9() {
    var clickFlag = false;
    // 获取canvas中图片实际大小
    var iCurWidth = img9.width;
    var iCurHeight = img9.height;
    console.log('图片当前实际宽度=', iCurWidth);
    console.log('图片当前实际高度=', iCurHeight);

    // 可调整截图框
    oRelDiv9.innerHTML = '';
    oRelDiv9.style.position = "absolute";
    oRelDiv9.style.width = iCurWidth + "px";
    oRelDiv9.style.height = iCurHeight + "px";
    oRelDiv9.style.top ="92px";// myCanvas9.offsetTop + 'px';
    console.log('oRelDiv.style.top = ', oRelDiv9.style.top);
    oRelDiv9.id = "cropContainer9";

    var iOrigWidth = originWidth9;
    var iOrigHeight = originHeight9;
    scaleX9 = iCurWidth / iOrigWidth; // 图片宽度缩放比例（当前实际/原始）
    scaleY9 = iCurHeight / iOrigHeight;  // 图片高度缩放比例（当前实际/原始）
    console.log('图片横向（宽度）缩放比=', scaleX9);
    console.log('图片纵向（高度）缩放比=', scaleY9);

    // 将oRelDiv插入到myCanvas前
    myCanvas9.parentNode.insertBefore(oRelDiv9, myCanvas9);

    //初始化坐标与剪裁高宽

   if($("#up9").val()=="") {$("#up9").val(0);}
   if($("#left9").val()=="") {$("#left9").val(0);}
   if($("#right9").val()=="") {$("#right9").val(100);}
   if($("#down9").val()=="") {$("#down9").val(100);}

    var cropW=parseInt($("#right9").val())-parseInt($("#left9").val());
    var cropH=parseInt($("#down9").val())-parseInt($("#up9").val());
    var posY = parseInt($("#up9").val());
    var posX = parseInt($("#left9").val());
 
    oRelDiv9.innerHTML = '<div id="zxxCropBox9" style="height:' + cropH + 'px; width:' + cropW + 'px; position:absolute; left:' +
        posX + 'px; top:' + posY + 'px; border:1px solid black;">' +
        '<div id="zxxDragBg9" style="height:100%; background:white; opacity:0.3; filter:alpha(opacity=30); cursor:move"></div>' +
        '<div id="dragLeftTop9" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; top:-3px; cursor:nw-resize;"></div>' +
        '<div id="dragLeftBot9" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; bottom:-3px; cursor:sw-resize;"></div>' +
        '<div id="dragRightTop9" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; top:-3px; cursor:ne-resize;"></div>' +
        '<div id="dragRightBot9" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; bottom:-3px; cursor:se-resize;"></div>' +
        '<div id="dragTopCenter9" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; top:-3px; left:50%; margin-left:-3px; cursor:n-resize;"></div>' +
        '<div id="dragBotCenter9" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; bottom:-3px; left:50%; margin-left:-3px; cursor:s-resize;"></div>' +
        '<div id="dragRightCenter9" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; top:50%; margin-top:-3px; cursor:e-resize;"></div> ' +
        '<div id="dragLeftCenter9" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; top:50%; margin-top:-3px; cursor:w-resize;"></div>' +
        '</div>';
    if($("#up9").val()=="")
    $("#up9").val(0);
  if($("#left9").val()=="")
  $("#left9").val(0);
  if($("#down9").val()=="")
  $("#down9").val(100);
  if($("#right9").val()=="")
  $("#right9").val(100);
    ID("cropImageWidth9").value = parseInt(ID("zxxCropBox9").style.width);
    ID("cropImageHeight9").value = parseInt(ID("zxxCropBox9").style.height);

    var startDrag9 = function (point, target, kind) {
        //point是拉伸点，target是被拉伸的目标，其高度及位置会发生改变
        //此处的target与上面拖拽的target是同一目标，故其params.left,params.top可以共用，也必须共用
        //初始化宽高
        params9.width = getCss9(target, "width");
        params9.height = getCss9(target, "height");
        //初始化坐标
        if (getCss9(target, "left") !== "auto") {
            params9.left = getCss9(target, "left");
        }
        if (getCss9(target, "top") !== "auto") {
            params9.top = getCss9(target, "top");
        }
        //target是移动对象
        point.onmousedown = function (event) {
            params9.kind = kind;
            params9.flag = true;
            clickFlag = true;
            if (!event) {
                event = window.event;
            }
            var e = event;
            params9.currentX = e.clientX;  //鼠标按下时坐标x轴
            params9.currentY = e.clientY;  //鼠标按下时坐标y轴
            /*console.log('params.currentX=', params.currentX);
            console.log('params.currentY=', params.currentY);*/
            //防止IE文字选中，有助于拖拽平滑
            point.onselectstart = function () {
                return false;
            };

            document.onmousemove = function (event) {
                let e = event ? event : window.event;
                clickFlag = false;
                if (params9.flag) {
                    var nowX = e.clientX; // 鼠标移动时x坐标
                    var nowY = e.clientY;   // 鼠标移动时y坐标
                    var disX = nowX - params9.currentX;  // 鼠标x方向移动距离
                    var disY = nowY - params9.currentY;  // 鼠标y方向移动距离
                    if (params9.kind === "n") {
                        //上拉伸
                        //高度增加或减小，位置上下移动
                        if (parseInt(params9.top) + disY > 0) {
                            target.style.top = parseInt(params9.top) + disY + "px";
                            target.style.height = parseInt(params9.height) - disY + "px";
                        }
                    } else if (params9.kind === "w") { //左拉伸
                        if (parseInt(params9.left) + disX > 0) {
                            target.style.left = parseInt(params9.left) + disX + "px";
                            target.style.width = parseInt(params9.width) - disX + "px";
                        }
                    } else if (params9.kind === "e") { //右拉伸
                        if (parseInt(params9.width) + disX + parseInt(params9.left) < bgwidth9)
                           target.style.width = parseInt(params9.width) + disX + "px";
                    } else if (params9.kind === "s") { //下拉伸
                        if (parseInt(params9.height) + disY + parseInt(params9.top) < bgheight9)
                            target.style.height = parseInt(params9.height) + disY + "px";
                    } else if (params9.kind === "nw") { //左上拉伸
                        if (parseInt(params9.left) + disX > 0 && parseInt(params9.top) + disY > 0) {
                            target.style.left = parseInt(params9.left) + disX + "px";
                            target.style.width = parseInt(params9.width) - disX + "px";
                            target.style.top = parseInt(params9.top) + disY + "px";
                            target.style.height = parseInt(params9.height) - disY + "px";
                        }
                    } else if (params9.kind === "ne") { //右上拉伸
                        if (parseInt(params9.width) + disX + parseInt(params9.left) < bgwidth9 && parseInt(params9.top) + disY > 0) {
                            target.style.top = parseInt(params9.top) + disY + "px";
                            target.style.height = parseInt(params9.height) - disY + "px";
                            target.style.width = parseInt(params9.width) + disX + "px";
                        }
                    } else if (params9.kind === "sw") { //左下拉伸
                        if (parseInt(params9.left) + disX > 0 && parseInt(params9.height) + disY + parseInt(params9.top) < bgheight9) {
                            target.style.left = parseInt(params9.left) + disX + "px";
                            target.style.width = parseInt(params9.width) - disX + "px";
                            target.style.height = parseInt(params9.height) + disY + "px";
                        }
                    } else if (params9.kind === "se") { //右下拉伸
                        if (parseInt(params9.width) + disX + parseInt(params9.left) < bgwidth9 && parseInt(params9.height) + disY + parseInt(params9.top) < bgheight9) {
                            target.style.width = parseInt(params9.width) + disX + "px";
                            target.style.height = parseInt(params9.height) + disY + "px";
                        }
                    } else { //移动
                        if (parseInt(params9.left) + disX < 0) { target.style.left = 0 + "px"; }//最左侧为0,小于0阻止移动
                        //最右侧，417-截图框宽度为left最大距离，超过阻止移动
                        else if (parseInt(params9.left) + disX + parseInt(ID("zxxCropBox9").style.width) > bgwidth9) { target.style.left = bgwidth9 - parseInt(ID("zxxCropBox9").style.width) + "px"; }
                        else target.style.left = parseInt(params9.left) + disX + "px";//正常范围
                        if (parseInt(params9.top) + disY < 0) { target.style.top = 0 }//高度小于0,阻止向上移动
                        else if (parseInt(params9.top) + disY + parseInt(ID("zxxCropBox9").style.height) > bgheight9)//移动高度+控件高度超过220，阻止移动
                        { target.style.top = bgheight9 - parseInt(ID("zxxCropBox9").style.height) + "px"; }
                        else { target.style.top = parseInt(params9.top) + disY + "px"; }//正常高度范围内
                    }
                }

                document.onmouseup = function () {
                    document.onmousemove = null;
                    document.onmouseup = null;
                    params9.flag = false;
                    if (getCss9(target, "left") !== "auto") {
                        params9.left = getCss9(target, "left");
                    }
                    if (getCss9(target, "top") !== "auto") {
                        params9.top = getCss9(target, "top");
                    }
                    params9.width = getCss9(target, "width");
                    params9.height = getCss9(target, "height");
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
                    var right = posX + parseInt(ID("zxxCropBox9").style.width);
                    if (right < 0) right = 0;
                    var down = posY + parseInt(ID("zxxCropBox9").style.height);
                    if (down < 0) down = 0;
                    ID("left9").value = posX;
                    ID("up9").value = posY;
                    ID("right9").value = right;
                    ID("down9").value = down;
                    ID("cropImageWidth9").value = parseInt(ID("zxxCropBox9").style.width);
                    ID("cropImageHeight9").value = parseInt(ID("zxxCropBox9").style.height);
                };
            }
        };
    };

    //绑定拖拽
    startDrag9(ID9("zxxDragBg9"), ID9("zxxCropBox9"), "drag");
    //绑定拉伸
    startDrag9(ID9("dragLeftTop9"), ID9("zxxCropBox9"), "nw");
    startDrag9(ID9("dragLeftBot9"), ID9("zxxCropBox9"), "sw");
    startDrag9(ID9("dragRightTop9"), ID9("zxxCropBox9"), "ne");
    startDrag9(ID9("dragRightBot9"), ID9("zxxCropBox9"), "se");
    startDrag9(ID9("dragTopCenter9"), ID9("zxxCropBox9"), "n");
    startDrag9(ID9("dragBotCenter9"), ID9("zxxCropBox9"), "s");
    startDrag9(ID9("dragRightCenter9"), ID9("zxxCropBox9"), "e");
    startDrag9(ID9("dragLeftCenter9"), ID9("zxxCropBox9"), "w");


    //图片不能被选中，目的在于使拖拽顺滑
    ID9("myCanvas9").onselectstart = function () {
        return false;
    };
    img9.onselectstart = function () {
        return false;
    };
}

//确认裁剪
function RequeryCrop9() {
    var x = document.getElementById("left9").value;
    var y = document.getElementById("up9").value;
    var w = document.getElementById("cropImageWidth9").value;
    var h = document.getElementById("cropImageHeight9").value;
    console.log('cropImage(img,', x, ',', y, ',', parseInt(w), ',', parseInt(h), ')');
   // cropImage9(img9, x / scaleX9, y / scaleY9, parseInt(w) / scaleX9, parseInt(h) / scaleY9);
}


function cropImage9(img, cropPosX, cropPosY, width, height) {
    /*var cropContainer = ID("cropContainer");
    cropContainer.parentNode.removeChild(cropContainer);*/
    /*ctx.clearRect(0, 0, myCanvas.width, myCanvas.height);*/
    //sx,sy 是相对于图片的坐标。巨坑

    $("#imgDiv9").html('');
    var myCanvas = document.createElement('canvas');
    myCanvas.setAttribute('id', 'myCanvas9');
    myCanvas.style.display = 'block';
    /*myCanvas.style.position = 'absolute';*/
    myCanvas.width = width * scaleX9;
    myCanvas.height = height * scaleY9;
    myCanvas.style.border = "1px solid #d3d3d3";
    myCanvas.innerText = '您的浏览器不支持 HTML5 canvas 标签。';
    myCanvas.style.zIndex = 'auto';

    var newCtx = myCanvas.getContext('2d');
    newCtx.drawImage(img9, cropPosX, cropPosY, width, height, 0, 0, width * scaleX9, height * scaleY9);

    // canvas转化为图片
    var newImage = new Image();
    newImage.src = myCanvas.toDataURL("image/png");
    //newImage.style.marginLeft = '5px';
    newImage.width = bgwidth9;
    newImage.height = bgheight9;
    //clipImgDiv.appendChild(newImage);
    imgDiv9.appendChild(newImage);
    oRelDiv9.innerHTML = '';
}
