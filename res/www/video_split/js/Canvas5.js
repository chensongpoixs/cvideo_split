let originWidth5; // 图片原始宽度
let originHeigh5; // 图片原始高度
let container5 = document.getElementById('container5');
let imgDiv5 = document.getElementById('imgDiv5');    // 存放mycanvas
//let btnDiv = document.getElementById('btnDiv1');
let clipImgDiv5 = document.getElementById('clipImgDiv5');    // 显示裁剪所获的图片
var oRelDiv5 = document.createElement("div"); // 截图框
var scaleX5 = 1;// 图片宽度缩放比例（当前实际/原始）
var scaleY5 = 1;  // 图片高度缩放比例（当前实际/原始）
var bgwidth5 = 417;
var bgheight5 = 220;

var img5 = new Image();
//拖拽与拉伸方法
//拖拽拉伸所需参数
let params5 = {
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
container5.style.display = 'flex';
container5.style.flexDirection = 'column';
imgDiv5.style.marginBottom = '20px';

/**********************************************************************/
$(function () {
    ExecuteCanvas5();
})

var myCanvas5;
function ExecuteCanvas5() {
    // 创建canvas，用于显示被裁剪图片
    myCanvas5 = document.createElement('canvas');
    myCanvas5.setAttribute('id', 'myCanvas5');
    myCanvas5.style.display = 'block';
    /*myCanvas.style.position = 'absolute';*/
    myCanvas5.width = bgwidth5;
    myCanvas5.height = bgheight5;
    myCanvas5.style.border = "1px solid #d3d3d3";
    myCanvas5.innerText = '您的浏览器不支持 HTML5 canvas 标签。';
    myCanvas5.style.zIndex = 'auto';

    var ctx5 = myCanvas5.getContext('2d');

    // 被裁剪图片

    img5.src = image5;
    img5.setAttribute('id', 'img');
    img5.width = bgwidth5;
    img5.height = bgheight5;
    img5.onload = function () {
        console.log('onload()执行...');
        ctx5.drawImage(img5, 0, 0, bgwidth5, bgheight5);
        originWidth5 = img5.naturalWidth;
        originHeight5 = img5.naturalHeight;
        console.log('图片原始宽度=', originWidth5);
        console.log('图片原始高度=', originHeight5);

        cutImg5();
    };
    /**********************************************************************/

    imgDiv5.appendChild(myCanvas5);
}

// 获取指定元素DOM
const ID5 = function (id) {
    return document.getElementById(id);
};

//获取相关CSS属性方法
let getCss5 = function (o, key) {
    return o.currentStyle ? o.currentStyle[key] : document.defaultView.getComputedStyle(o, false)[key];
};


// 裁剪得到的图片
let clipImg5 = new Image();
clipImg5.src = '';
clipImg5.style.height = '100px';
clipImg5.style.width = '100px';
clipImg5.alt = '裁剪获得图片...';

function cutImg5() {
    var clickFlag = false;
    // 获取canvas中图片实际大小
    var iCurWidth = img5.width;
    var iCurHeight = img5.height;
    console.log('图片当前实际宽度=', iCurWidth);
    console.log('图片当前实际高度=', iCurHeight);

    // 可调整截图框
    oRelDiv5.innerHTML = '';
    oRelDiv5.style.position = "absolute";
    oRelDiv5.style.width = iCurWidth + "px";
    oRelDiv5.style.height = iCurHeight + "px";
    oRelDiv5.style.top = "92px";//myCanvas5.offsetTop + 'px';
    console.log('oRelDiv.style.top = ', oRelDiv5.style.top);
    oRelDiv5.id = "cropContainer5";

    var iOrigWidth = originWidth5;
    var iOrigHeight = originHeight5;
    scaleX5 = iCurWidth / iOrigWidth; // 图片宽度缩放比例（当前实际/原始）
    scaleY5 = iCurHeight / iOrigHeight;  // 图片高度缩放比例（当前实际/原始）
    console.log('图片横向（宽度）缩放比=', scaleX5);
    console.log('图片纵向（高度）缩放比=', scaleY5);

    // 将oRelDiv插入到myCanvas前
    myCanvas5.parentNode.insertBefore(oRelDiv5, myCanvas5);

    //初始化坐标与剪裁高宽
   if($("#up5").val()=="") {$("#up5").val(0);}
   if($("#left5").val()=="") {$("#left5").val(0);}
   if($("#right5").val()=="") {$("#right5").val(100);}
   if($("#down5").val()=="") {$("#down5").val(100);}

    var cropW=parseInt($("#right5").val())-parseInt($("#left5").val());
    var cropH=parseInt($("#down5").val())-parseInt($("#up5").val());
    var posY = parseInt($("#up5").val());
    var posX = parseInt($("#left5").val());
 
    oRelDiv5.innerHTML = '<div id="zxxCropBox5" style="height:' + cropH + 'px; width:' + cropW + 'px; position:absolute; left:' +
        posX + 'px; top:' + posY + 'px; border:1px solid black;">' +
        '<div id="zxxDragBg5" style="height:100%; background:white; opacity:0.3; filter:alpha(opacity=30); cursor:move"></div>' +
        '<div id="dragLeftTop5" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; top:-3px; cursor:nw-resize;"></div>' +
        '<div id="dragLeftBot5" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; bottom:-3px; cursor:sw-resize;"></div>' +
        '<div id="dragRightTop5" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; top:-3px; cursor:ne-resize;"></div>' +
        '<div id="dragRightBot5" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; bottom:-3px; cursor:se-resize;"></div>' +
        '<div id="dragTopCenter5" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; top:-3px; left:50%; margin-left:-3px; cursor:n-resize;"></div>' +
        '<div id="dragBotCenter5" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; bottom:-3px; left:50%; margin-left:-3px; cursor:s-resize;"></div>' +
        '<div id="dragRightCenter5" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; top:50%; margin-top:-3px; cursor:e-resize;"></div> ' +
        '<div id="dragLeftCenter5" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; top:50%; margin-top:-3px; cursor:w-resize;"></div>' +
        '</div>';
    if($("#up5").val()=="")
    $("#up5").val(0);
  if($("#left5").val()=="")
  $("#left5").val(0);
  if($("#down5").val()=="")
  $("#down5").val(100);
  if($("#right5").val()=="")
  $("#right5").val(100);
    ID5("cropImageWidth5").value = parseInt(ID5("zxxCropBox5").style.width);
    ID5("cropImageHeight5").value = parseInt(ID5("zxxCropBox5").style.height);

    var startDrag5 = function (point, target, kind) {
        //point是拉伸点，target是被拉伸的目标，其高度及位置会发生改变
        //此处的target与上面拖拽的target是同一目标，故其params.left,params.top可以共用，也必须共用
        //初始化宽高
        params5.width = getCss5(target, "width");
        params5.height = getCss5(target, "height");
        //初始化坐标
        if (getCss5(target, "left") !== "auto") {
            params5.left = getCss5(target, "left");
        }
        if (getCss5(target, "top") !== "auto") {
            params5.top = getCss5(target, "top");
        }
        //target是移动对象
        point.onmousedown = function (event) {
            params5.kind = kind;
            params5.flag = true;
            clickFlag = true;
            if (!event) {
                event = window.event;
            }
            var e = event;
            params5.currentX = e.clientX;  //鼠标按下时坐标x轴
            params5.currentY = e.clientY;  //鼠标按下时坐标y轴
            /*console.log('params.currentX=', params.currentX);
            console.log('params.currentY=', params.currentY);*/
            //防止IE文字选中，有助于拖拽平滑
            point.onselectstart = function () {
                return false;
            };

            document.onmousemove = function (event) {
                let e = event ? event : window.event;
                clickFlag = false;
                if (params5.flag) {
                    var nowX = e.clientX; // 鼠标移动时x坐标
                    var nowY = e.clientY;   // 鼠标移动时y坐标
                    var disX = nowX - params5.currentX;  // 鼠标x方向移动距离
                    var disY = nowY - params5.currentY;  // 鼠标y方向移动距离
                    if (params5.kind === "n") {
                        //上拉伸
                        //高度增加或减小，位置上下移动
                        if (parseInt(params5.top) + disY > 0) {
                            target.style.top = parseInt(params5.top) + disY + "px";
                            target.style.height = parseInt(params5.height) - disY + "px";
                        }
                    } else if (params5.kind === "w") { //左拉伸
                        if (parseInt(params5.left) + disX > 0) {
                            target.style.left = parseInt(params5.left) + disX + "px";
                            target.style.width = parseInt(params5.width) - disX + "px";
                        }
                    } else if (params5.kind === "e") { //右拉伸
                        if (parseInt(params5.width) + disX + parseInt(params5.left) < bgwidth5)
                           target.style.width = parseInt(params5.width) + disX + "px";
                    } else if (params5.kind === "s") { //下拉伸
                        if (parseInt(params5.height) + disY + parseInt(params5.top) < bgheight5)
                            target.style.height = parseInt(params5.height) + disY + "px";
                    } else if (params5.kind === "nw") { //左上拉伸
                        if (parseInt(params5.left) + disX > 0 && parseInt(params5.top) + disY > 0) {
                            target.style.left = parseInt(params5.left) + disX + "px";
                            target.style.width = parseInt(params5.width) - disX + "px";
                            target.style.top = parseInt(params5.top) + disY + "px";
                            target.style.height = parseInt(params5.height) - disY + "px";
                        }
                    } else if (params5.kind === "ne") { //右上拉伸
                        if (parseInt(params5.width) + disX + parseInt(params5.left) < bgwidth5 && parseInt(params5.top) + disY > 0) {
                            target.style.top = parseInt(params5.top) + disY + "px";
                            target.style.height = parseInt(params5.height) - disY + "px";
                            target.style.width = parseInt(params5.width) + disX + "px";
                        }
                    } else if (params5.kind === "sw") { //左下拉伸
                        if (parseInt(params5.left) + disX > 0 && parseInt(params5.height) + disY + parseInt(params5.top) < bgheight5) {
                            target.style.left = parseInt(params5.left) + disX + "px";
                            target.style.width = parseInt(params5.width) - disX + "px";
                            target.style.height = parseInt(params5.height) + disY + "px";
                        }
                    } else if (params5.kind === "se") { //右下拉伸
                        if (parseInt(params5.width) + disX + parseInt(params5.left) < bgwidth5 && parseInt(params5.height) + disY + parseInt(params5.top) < bgheight5) {
                            target.style.width = parseInt(params5.width) + disX + "px";
                            target.style.height = parseInt(params5.height) + disY + "px";
                        }
                    } else { //移动
                        if (parseInt(params5.left) + disX < 0) { target.style.left = 0 + "px"; }//最左侧为0,小于0阻止移动
                        //最右侧，417-截图框宽度为left最大距离，超过阻止移动
                        else if (parseInt(params5.left) + disX + parseInt(ID5("zxxCropBox5").style.width) > bgwidth5) { target.style.left = bgwidth5 - parseInt(ID5("zxxCropBox5").style.width) + "px"; }
                        else target.style.left = parseInt(params5.left) + disX + "px";//正常范围
                        if (parseInt(params5.top) + disY < 0) { target.style.top = 0 }//高度小于0,阻止向上移动
                        else if (parseInt(params5.top) + disY + parseInt(ID5("zxxCropBox5").style.height) > bgheight5)//移动高度+控件高度超过220，阻止移动
                        { target.style.top = bgheight5 - parseInt(ID5("zxxCropBox5").style.height) + "px"; }
                        else { target.style.top = parseInt(params5.top) + disY + "px"; }//正常高度范围内
                    }
                }

                document.onmouseup = function () {
                    document.onmousemove = null;
                    document.onmouseup = null;
                    params5.flag = false;
                    if (getCss5(target, "left") !== "auto") {
                        params5.left = getCss5(target, "left");
                    }
                    if (getCss5(target, "top") !== "auto") {
                        params5.top = getCss5(target, "top");
                    }
                    params5.width = getCss5(target, "width");
                    params5.height = getCss5(target, "height");
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
                    var right = posX + parseInt(ID("zxxCropBox5").style.width);
                    if (right < 0) right = 0;
                    var down = posY + parseInt(ID("zxxCropBox5").style.height);
                    if (down < 0) down = 0;
                    ID("left5").value = posX;
                    ID("up5").value = posY;
                    ID("right5").value = right;
                    ID("down5").value = down;
                    ID("cropImageWidth5").value = parseInt(ID("zxxCropBox5").style.width);
                    ID("cropImageHeight5").value = parseInt(ID("zxxCropBox5").style.height);
                };
            }
        };
    };

    //绑定拖拽
    startDrag5(ID5("zxxDragBg5"), ID5("zxxCropBox5"), "drag");
    //绑定拉伸
    startDrag5(ID5("dragLeftTop5"), ID5("zxxCropBox5"), "nw");
    startDrag5(ID5("dragLeftBot5"), ID5("zxxCropBox5"), "sw");
    startDrag5(ID5("dragRightTop5"), ID5("zxxCropBox5"), "ne");
    startDrag5(ID5("dragRightBot5"), ID5("zxxCropBox5"), "se");
    startDrag5(ID5("dragTopCenter5"), ID5("zxxCropBox5"), "n");
    startDrag5(ID5("dragBotCenter5"), ID5("zxxCropBox5"), "s");
    startDrag5(ID5("dragRightCenter5"), ID5("zxxCropBox5"), "e");
    startDrag5(ID5("dragLeftCenter5"), ID5("zxxCropBox5"), "w");


    //图片不能被选中，目的在于使拖拽顺滑
    ID5("myCanvas5").onselectstart = function () {
        return false;
    };
    img5.onselectstart = function () {
        return false;
    };
}

//确认裁剪
function RequeryCrop5() {
    var x = document.getElementById("left5").value;
    var y = document.getElementById("up5").value;
    var w = document.getElementById("cropImageWidth5").value;
    var h = document.getElementById("cropImageHeight5").value;
    console.log('cropImage(img,', x, ',', y, ',', parseInt(w), ',', parseInt(h), ')');
}


function cropImage5(img, cropPosX, cropPosY, width, height) {
    $("#imgDiv5").html('');
    var myCanvas = document.createElement('canvas');
    myCanvas.setAttribute('id', 'myCanvas5');
    myCanvas.style.display = 'block';
    myCanvas.width = width * scaleX5;
    myCanvas.height = height * scaleY5;
    myCanvas.style.border = "1px solid #d3d3d3";
    myCanvas.innerText = '您的浏览器不支持 HTML5 canvas 标签。';
    myCanvas.style.zIndex = 'auto';

    var newCtx = myCanvas.getContext('2d');
    newCtx.drawImage(img5, cropPosX, cropPosY, width, height, 0, 0, width * scaleX5, height * scaleY5);

    // canvas转化为图片
    var newImage = new Image();
    newImage.src = myCanvas.toDataURL("image/png");
    newImage.width = bgwidth5;
    newImage.height = bgheight5;
    imgDiv5.appendChild(newImage);
    oRelDiv5.innerHTML = '';
}
