let originWidth7; // 图片原始宽度
let originHeigh7; // 图片原始高度
let container7 = document.getElementById('container7');
let imgDiv7 = document.getElementById('imgDiv7');    // 存放mycanvas
//let btnDiv = document.getElementById('btnDiv1');
let clipImgDiv7 = document.getElementById('clipImgDiv7');    // 显示裁剪所获的图片
var oRelDiv7 = document.createElement("div"); // 截图框
var scaleX7 = 1;// 图片宽度缩放比例（当前实际/原始）
var scaleY7 = 1;  // 图片高度缩放比例（当前实际/原始）
var bgwidth7 = 417;
var bgheight7 = 220;

var img7 = new Image();
//拖拽与拉伸方法
//拖拽拉伸所需参数
let params7 = {
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
container7.style.display = 'flex';
container7.style.flexDirection = 'column';
//btnDiv.style.marginBottom = '20px';
//btnDiv.style.height = '70px';
imgDiv7.style.marginBottom = '20px';

/**********************************************************************/
$(function () {
    ExecuteCanvas7();
})


function ExecuteCanvas7() {
    // 创建canvas，用于显示被裁剪图片
    var myCanvas7 = document.createElement('canvas');
    myCanvas7.setAttribute('id', 'myCanvas7');
    myCanvas7.style.display = 'block';
    /*myCanvas.style.position = 'absolute';*/
    myCanvas7.width = bgwidth7;
    myCanvas7.height = bgheight7;
    myCanvas7.style.border = "1px solid #d3d3d3";
    myCanvas7.innerText = '您的浏览器不支持 HTML5 canvas 标签。';
    myCanvas7.style.zIndex = 'auto';

    var ctx7 = myCanvas7.getContext('2d');

    // 被裁剪图片

    img7.src = image7;
    img7.setAttribute('id', 'img');
    img7.width = bgwidth7;
    img7.height = bgheight7;
    img7.onload = function () {
        console.log('onload()执行...');
        ctx7.drawImage(img7, 0, 0, bgwidth7, bgheight7);
        originWidth7 = img7.naturalWidth;
        originHeight7 = img7.naturalHeight;
        console.log('图片原始宽度=', originWidth7);
        console.log('图片原始高度=', originHeight7);

        cutImg7();
    };
    /**********************************************************************/

    imgDiv7.appendChild(myCanvas7);
}

// 获取指定元素DOM
const ID7 = function (id) {
    return document.getElementById(id);
};

//获取相关CSS属性方法
let getCss7 = function (o, key) {
    return o.currentStyle ? o.currentStyle[key] : document.defaultView.getComputedStyle(o, false)[key];
};


// 裁剪得到的图片
let clipImg7 = new Image();
clipImg7.src = '';
clipImg7.style.height = '100px';
clipImg7.style.width = '100px';
clipImg7.alt = '裁剪获得图片...';

function cutImg7() {
    var clickFlag = false;
    // 获取canvas中图片实际大小
    var iCurWidth = img7.width;
    var iCurHeight = img7.height;
    console.log('图片当前实际宽度=', iCurWidth);
    console.log('图片当前实际高度=', iCurHeight);

    // 可调整截图框
    oRelDiv7.innerHTML = '';
    oRelDiv7.style.position = "absolute";
    oRelDiv7.style.width = iCurWidth + "px";
    oRelDiv7.style.height = iCurHeight + "px";
    oRelDiv7.style.top ="377px";// myCanvas7.offsetTop + 'px';
    console.log('oRelDiv.style.top = ', oRelDiv7.style.top);
    oRelDiv7.id = "cropContainer7";

    var iOrigWidth = originWidth7;
    var iOrigHeight = originHeight7;
    scaleX7 = iCurWidth / iOrigWidth; // 图片宽度缩放比例（当前实际/原始）
    scaleY7 = iCurHeight / iOrigHeight;  // 图片高度缩放比例（当前实际/原始）
    console.log('图片横向（宽度）缩放比=', scaleX7);
    console.log('图片纵向（高度）缩放比=', scaleY7);

    // 将oRelDiv插入到myCanvas前
    myCanvas7.parentNode.insertBefore(oRelDiv7, myCanvas7);

    //初始化坐标与剪裁高宽
    //var cropW = 100; //截图框默认宽度
    //var cropH = 100; //截图框默认高度
    /*console.log('myCanvas.offsetLeft=', myCanvas.offsetLeft);
    console.log('myCanvas.offsetTop=', myCanvas.offsetTop);*/
    //var posX = 0; //myCanvas.width / 2 - cropW / 2;  // 截图框左上角x坐标
    //var posY = 0; //myCanvas.height / 2 - cropH / 2;    // 截图框左上角y坐标
    /*console.log('posX=',posX);
    console.log('posY=',posY);*/
   if($("#up7").val()=="") {$("#up7").val(0);}
   if($("#left7").val()=="") {$("#left7").val(0);}
   if($("#right7").val()=="") {$("#right7").val(100);}
   if($("#down7").val()=="") {$("#down7").val(100);}

    var cropW=parseInt($("#right7").val())-parseInt($("#left7").val());
    var cropH=parseInt($("#down7").val())-parseInt($("#up7").val());
    var posY = parseInt($("#up7").val());
    var posX = parseInt($("#left7").val());
 
    oRelDiv7.innerHTML = '<div id="zxxCropBox7" style="height:' + cropH + 'px; width:' + cropW + 'px; position:absolute; left:' +
        posX + 'px; top:' + posY + 'px; border:1px solid black;">' +
        '<div id="zxxDragBg7" style="height:100%; background:white; opacity:0.3; filter:alpha(opacity=30); cursor:move"></div>' +
        '<div id="dragLeftTop7" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; top:-3px; cursor:nw-resize;"></div>' +
        '<div id="dragLeftBot7" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; bottom:-3px; cursor:sw-resize;"></div>' +
        '<div id="dragRightTop7" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; top:-3px; cursor:ne-resize;"></div>' +
        '<div id="dragRightBot7" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; bottom:-3px; cursor:se-resize;"></div>' +
        '<div id="dragTopCenter7" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; top:-3px; left:50%; margin-left:-3px; cursor:n-resize;"></div>' +
        '<div id="dragBotCenter7" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; bottom:-3px; left:50%; margin-left:-3px; cursor:s-resize;"></div>' +
        '<div id="dragRightCenter7" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; top:50%; margin-top:-3px; cursor:e-resize;"></div> ' +
        '<div id="dragLeftCenter7" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; top:50%; margin-top:-3px; cursor:w-resize;"></div>' +
        '</div>';
    if($("#up7").val()=="")
    $("#up7").val(0);
  if($("#left7").val()=="")
  $("#left7").val(0);
  if($("#down7").val()=="")
  $("#down7").val(100);
  if($("#right7").val()=="")
  $("#right7").val(100);
    ID("cropImageWidth7").value = parseInt(ID("zxxCropBox7").style.width);
    ID("cropImageHeight7").value = parseInt(ID("zxxCropBox7").style.height);

    var startDrag7 = function (point, target, kind) {
        //point是拉伸点，target是被拉伸的目标，其高度及位置会发生改变
        //此处的target与上面拖拽的target是同一目标，故其params.left,params.top可以共用，也必须共用
        //初始化宽高
        params7.width = getCss7(target, "width");
        params7.height = getCss7(target, "height");
        //初始化坐标
        if (getCss7(target, "left") !== "auto") {
            params7.left = getCss7(target, "left");
        }
        if (getCss7(target, "top") !== "auto") {
            params7.top = getCss7(target, "top");
        }
        //target是移动对象
        point.onmousedown = function (event) {
            params7.kind = kind;
            params7.flag = true;
            clickFlag = true;
            if (!event) {
                event = window.event;
            }
            var e = event;
            params7.currentX = e.clientX;  //鼠标按下时坐标x轴
            params7.currentY = e.clientY;  //鼠标按下时坐标y轴
            /*console.log('params.currentX=', params.currentX);
            console.log('params.currentY=', params.currentY);*/
            //防止IE文字选中，有助于拖拽平滑
            point.onselectstart = function () {
                return false;
            };

            document.onmousemove = function (event) {
                let e = event ? event : window.event;
                clickFlag = false;
                if (params7.flag) {
                    var nowX = e.clientX; // 鼠标移动时x坐标
                    var nowY = e.clientY;   // 鼠标移动时y坐标
                    var disX = nowX - params7.currentX;  // 鼠标x方向移动距离
                    var disY = nowY - params7.currentY;  // 鼠标y方向移动距离
                    if (params7.kind === "n") {
                        //上拉伸
                        //高度增加或减小，位置上下移动
                        if (parseInt(params7.top) + disY > 0) {
                            target.style.top = parseInt(params7.top) + disY + "px";
                            target.style.height = parseInt(params7.height) - disY + "px";
                        }
                    } else if (params7.kind === "w") { //左拉伸
                        if (parseInt(params7.left) + disX > 0) {
                            target.style.left = parseInt(params7.left) + disX + "px";
                            target.style.width = parseInt(params7.width) - disX + "px";
                        }
                    } else if (params7.kind === "e") { //右拉伸
                        if (parseInt(params7.width) + disX + parseInt(params7.left) < bgwidth7)
                           target.style.width = parseInt(params7.width) + disX + "px";
                    } else if (params7.kind === "s") { //下拉伸
                        if (parseInt(params7.height) + disY + parseInt(params7.top) < bgheight7)
                            target.style.height = parseInt(params7.height) + disY + "px";
                    } else if (params7.kind === "nw") { //左上拉伸
                        if (parseInt(params7.left) + disX > 0 && parseInt(params7.top) + disY > 0) {
                            target.style.left = parseInt(params7.left) + disX + "px";
                            target.style.width = parseInt(params7.width) - disX + "px";
                            target.style.top = parseInt(params7.top) + disY + "px";
                            target.style.height = parseInt(params7.height) - disY + "px";
                        }
                    } else if (params7.kind === "ne") { //右上拉伸
                        if (parseInt(params7.width) + disX + parseInt(params7.left) < bgwidth7 && parseInt(params7.top) + disY > 0) {
                            target.style.top = parseInt(params7.top) + disY + "px";
                            target.style.height = parseInt(params7.height) - disY + "px";
                            target.style.width = parseInt(params7.width) + disX + "px";
                        }
                    } else if (params7.kind === "sw") { //左下拉伸
                        if (parseInt(params7.left) + disX > 0 && parseInt(params7.height) + disY + parseInt(params7.top) < bgheight7) {
                            target.style.left = parseInt(params7.left) + disX + "px";
                            target.style.width = parseInt(params7.width) - disX + "px";
                            target.style.height = parseInt(params7.height) + disY + "px";
                        }
                    } else if (params7.kind === "se") { //右下拉伸
                        if (parseInt(params7.width) + disX + parseInt(params7.left) < bgwidth7 && parseInt(params7.height) + disY + parseInt(params7.top) < bgheight7) {
                            target.style.width = parseInt(params7.width) + disX + "px";
                            target.style.height = parseInt(params7.height) + disY + "px";
                        }
                    } else { //移动
                        if (parseInt(params7.left) + disX < 0) { target.style.left = 0 + "px"; }//最左侧为0,小于0阻止移动
                        //最右侧，417-截图框宽度为left最大距离，超过阻止移动
                        else if (parseInt(params7.left) + disX + parseInt(ID("zxxCropBox7").style.width) > bgwidth7) { target.style.left = bgwidth7 - parseInt(ID("zxxCropBox7").style.width) + "px"; }
                        else target.style.left = parseInt(params7.left) + disX + "px";//正常范围
                        if (parseInt(params7.top) + disY < 0) { target.style.top = 0 }//高度小于0,阻止向上移动
                        else if (parseInt(params7.top) + disY + parseInt(ID("zxxCropBox7").style.height) > bgheight7)//移动高度+控件高度超过220，阻止移动
                        { target.style.top = bgheight7 - parseInt(ID("zxxCropBox7").style.height) + "px"; }
                        else { target.style.top = parseInt(params7.top) + disY + "px"; }//正常高度范围内
                    }
                }

                document.onmouseup = function () {
                    document.onmousemove = null;
                    document.onmouseup = null;
                    params7.flag = false;
                    if (getCss7(target, "left") !== "auto") {
                        params7.left = getCss7(target, "left");
                    }
                    if (getCss7(target, "top") !== "auto") {
                        params7.top = getCss7(target, "top");
                    }
                    params7.width = getCss7(target, "width");
                    params7.height = getCss7(target, "height");
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
                    var right = posX + parseInt(ID("zxxCropBox7").style.width);
                    if (right < 0) right = 0;
                    var down = posY + parseInt(ID("zxxCropBox7").style.height);
                    if (down < 0) down = 0;
                    ID("left7").value = posX;
                    ID("up7").value = posY;
                    ID("right7").value = right;
                    ID("down7").value = down;
                    ID("cropImageWidth7").value = parseInt(ID("zxxCropBox7").style.width);
                    ID("cropImageHeight7").value = parseInt(ID("zxxCropBox7").style.height);
                };
            }
        };
    };

    //绑定拖拽
    startDrag7(ID7("zxxDragBg7"), ID7("zxxCropBox7"), "drag");
    //绑定拉伸
    startDrag7(ID7("dragLeftTop7"), ID7("zxxCropBox7"), "nw");
    startDrag7(ID7("dragLeftBot7"), ID7("zxxCropBox7"), "sw");
    startDrag7(ID7("dragRightTop7"), ID7("zxxCropBox7"), "ne");
    startDrag7(ID7("dragRightBot7"), ID7("zxxCropBox7"), "se");
    startDrag7(ID7("dragTopCenter7"), ID7("zxxCropBox7"), "n");
    startDrag7(ID7("dragBotCenter7"), ID7("zxxCropBox7"), "s");
    startDrag7(ID7("dragRightCenter7"), ID7("zxxCropBox7"), "e");
    startDrag7(ID7("dragLeftCenter7"), ID7("zxxCropBox7"), "w");


    //图片不能被选中，目的在于使拖拽顺滑
    ID7("myCanvas7").onselectstart = function () {
        return false;
    };
    img7.onselectstart = function () {
        return false;
    };
}

//确认裁剪
function RequeryCrop7() {
    var x = document.getElementById("left7").value;
    var y = document.getElementById("up7").value;
    var w = document.getElementById("cropImageWidth7").value;
    var h = document.getElementById("cropImageHeight7").value;
    console.log('cropImage(img,', x, ',', y, ',', parseInt(w), ',', parseInt(h), ')');
   // cropImage7(img7, x / scaleX7, y / scaleY7, parseInt(w) / scaleX7, parseInt(h) / scaleY7);
}


function cropImage7(img, cropPosX, cropPosY, width, height) {
    /*var cropContainer = ID("cropContainer");
    cropContainer.parentNode.removeChild(cropContainer);*/
    /*ctx.clearRect(0, 0, myCanvas.width, myCanvas.height);*/
    //sx,sy 是相对于图片的坐标。巨坑

    $("#imgDiv7").html('');
    var myCanvas = document.createElement('canvas');
    myCanvas.setAttribute('id', 'myCanvas7');
    myCanvas.style.display = 'block';
    /*myCanvas.style.position = 'absolute';*/
    myCanvas.width = width * scaleX7;
    myCanvas.height = height * scaleY7;
    myCanvas.style.border = "1px solid #d3d3d3";
    myCanvas.innerText = '您的浏览器不支持 HTML5 canvas 标签。';
    myCanvas.style.zIndex = 'auto';

    var newCtx = myCanvas.getContext('2d');
    newCtx.drawImage(img7, cropPosX, cropPosY, width, height, 0, 0, width * scaleX7, height * scaleY7);

    // canvas转化为图片
    var newImage = new Image();
    newImage.src = myCanvas.toDataURL("image/png");
    //newImage.style.marginLeft = '5px';
    newImage.width = bgwidth7;
    newImage.height = bgheight7;
    //clipImgDiv.appendChild(newImage);
    imgDiv7.appendChild(newImage);
    oRelDiv7.innerHTML = '';
}
