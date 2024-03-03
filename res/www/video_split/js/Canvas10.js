let originWidth10; // 图片原始宽度
let originHeigh10; // 图片原始高度
let container10 = document.getElementById('container10');
let imgDiv10 = document.getElementById('imgDiv10');    // 存放mycanvas
//let btnDiv = document.getElementById('btnDiv1');
let clipImgDiv10 = document.getElementById('clipImgDiv10');    // 显示裁剪所获的图片
var oRelDiv10 = document.createElement("div"); // 截图框
var scaleX10 = 1;// 图片宽度缩放比例（当前实际/原始）
var scaleY10 = 1;  // 图片高度缩放比例（当前实际/原始）
var bgwidth10 = 417;
var bgheight10 = 220;

var img10 = new Image();
//拖拽与拉伸方法
//拖拽拉伸所需参数
let params10 = {
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
container10.style.display = 'flex';
container10.style.flexDirection = 'column';
//btnDiv.style.marginBottom = '20px';
//btnDiv.style.height = '100px';
imgDiv10.style.marginBottom = '20px';

/**********************************************************************/
$(function () {
    ExecuteCanvas10();
})


function ExecuteCanvas10() {
    // 创建canvas，用于显示被裁剪图片
    var myCanvas10 = document.createElement('canvas');
    myCanvas10.setAttribute('id', 'myCanvas10');
    myCanvas10.style.display = 'block';
    /*myCanvas.style.position = 'absolute';*/
    myCanvas10.width = bgwidth10;
    myCanvas10.height = bgheight10;
    myCanvas10.style.border = "1px solid #d3d3d3";
    myCanvas10.innerText = '您的浏览器不支持 HTML5 canvas 标签。';
    myCanvas10.style.zIndex = 'auto';

    var ctx10 = myCanvas10.getContext('2d');

    // 被裁剪图片

    img10.src = image10;
    img10.setAttribute('id', 'img');
    img10.width = bgwidth10;
    img10.height = bgheight10;
    img10.onload = function () {
        console.log('onload()执行...');
        ctx10.drawImage(img10, 0, 0, bgwidth10, bgheight10);
        originWidth10 = img10.naturalWidth;
        originHeight10 = img10.naturalHeight;
        console.log('图片原始宽度=', originWidth10);
        console.log('图片原始高度=', originHeight10);

        cutImg10();
    };
    /**********************************************************************/

    imgDiv10.appendChild(myCanvas10);
}

// 获取指定元素DOM
const ID10 = function (id) {
    return document.getElementById(id);
};

//获取相关CSS属性方法
let getCss10 = function (o, key) {
    return o.currentStyle ? o.currentStyle[key] : document.defaultView.getComputedStyle(o, false)[key];
};


// 裁剪得到的图片
let clipImg10 = new Image();
clipImg10.src = '';
clipImg10.style.height = '100px';
clipImg10.style.width = '100px';
clipImg10.alt = '裁剪获得图片...';

function cutImg10() {
    var clickFlag = false;
    // 获取canvas中图片实际大小
    var iCurWidth = img10.width;
    var iCurHeight = img10.height;
    console.log('图片当前实际宽度=', iCurWidth);
    console.log('图片当前实际高度=', iCurHeight);

    // 可调整截图框
    oRelDiv10.innerHTML = '';
    oRelDiv10.style.position = "absolute";
    oRelDiv10.style.width = iCurWidth + "px";
    oRelDiv10.style.height = iCurHeight + "px";
    oRelDiv10.style.top ="92px";// myCanvas10.offsetTop + 'px';
    console.log('oRelDiv.style.top = ', oRelDiv10.style.top);
    oRelDiv10.id = "cropContainer10";

    var iOrigWidth = originWidth10;
    var iOrigHeight = originHeight10;
    scaleX10 = iCurWidth / iOrigWidth; // 图片宽度缩放比例（当前实际/原始）
    scaleY10 = iCurHeight / iOrigHeight;  // 图片高度缩放比例（当前实际/原始）
    console.log('图片横向（宽度）缩放比=', scaleX10);
    console.log('图片纵向（高度）缩放比=', scaleY10);

    // 将oRelDiv插入到myCanvas前
    myCanvas10.parentNode.insertBefore(oRelDiv10, myCanvas10);

    //初始化坐标与剪裁高宽
   if($("#up10").val()=="") {$("#up10").val(0);}
   if($("#left10").val()=="") {$("#left10").val(0);}
   if($("#right10").val()=="") {$("#right10").val(100);}
   if($("#down10").val()=="") {$("#down10").val(100);}

    var cropW=parseInt($("#right10").val())-parseInt($("#left10").val());
    var cropH=parseInt($("#down10").val())-parseInt($("#up10").val());
    var posY = parseInt($("#up10").val());
    var posX = parseInt($("#left10").val());
 
    oRelDiv10.innerHTML = '<div id="zxxCropBox10" style="height:' + cropH + 'px; width:' + cropW + 'px; position:absolute; left:' +
        posX + 'px; top:' + posY + 'px; border:1px solid black;">' +
        '<div id="zxxDragBg10" style="height:100%; background:white; opacity:0.3; filter:alpha(opacity=30); cursor:move"></div>' +
        '<div id="dragLeftTop10" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; top:-3px; cursor:nw-resize;"></div>' +
        '<div id="dragLeftBot10" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; bottom:-3px; cursor:sw-resize;"></div>' +
        '<div id="dragRightTop10" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; top:-3px; cursor:ne-resize;"></div>' +
        '<div id="dragRightBot10" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; bottom:-3px; cursor:se-resize;"></div>' +
        '<div id="dragTopCenter10" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; top:-3px; left:50%; margin-left:-3px; cursor:n-resize;"></div>' +
        '<div id="dragBotCenter10" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; bottom:-3px; left:50%; margin-left:-3px; cursor:s-resize;"></div>' +
        '<div id="dragRightCenter10" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; top:50%; margin-top:-3px; cursor:e-resize;"></div> ' +
        '<div id="dragLeftCenter10" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; top:50%; margin-top:-3px; cursor:w-resize;"></div>' +
        '</div>';
    if($("#up10").val()=="")
    $("#up10").val(0);
  if($("#left10").val()=="")
  $("#left10").val(0);
  if($("#down10").val()=="")
  $("#down10").val(100);
  if($("#right10").val()=="")
  $("#right10").val(100);
    ID("cropImageWidth10").value = parseInt(ID("zxxCropBox10").style.width);
    ID("cropImageHeight10").value = parseInt(ID("zxxCropBox10").style.height);

    var startDrag10 = function (point, target, kind) {
        //point是拉伸点，target是被拉伸的目标，其高度及位置会发生改变
        //此处的target与上面拖拽的target是同一目标，故其params.left,params.top可以共用，也必须共用
        //初始化宽高
        params10.width = getCss10(target, "width");
        params10.height = getCss10(target, "height");
        //初始化坐标
        if (getCss10(target, "left") !== "auto") {
            params10.left = getCss10(target, "left");
        }
        if (getCss10(target, "top") !== "auto") {
            params10.top = getCss10(target, "top");
        }
        //target是移动对象
        point.onmousedown = function (event) {
            params10.kind = kind;
            params10.flag = true;
            clickFlag = true;
            if (!event) {
                event = window.event;
            }
            var e = event;
            params10.currentX = e.clientX;  //鼠标按下时坐标x轴
            params10.currentY = e.clientY;  //鼠标按下时坐标y轴
            /*console.log('params.currentX=', params.currentX);
            console.log('params.currentY=', params.currentY);*/
            //防止IE文字选中，有助于拖拽平滑
            point.onselectstart = function () {
                return false;
            };

            document.onmousemove = function (event) {
                let e = event ? event : window.event;
                clickFlag = false;
                if (params10.flag) {
                    var nowX = e.clientX; // 鼠标移动时x坐标
                    var nowY = e.clientY;   // 鼠标移动时y坐标
                    var disX = nowX - params10.currentX;  // 鼠标x方向移动距离
                    var disY = nowY - params10.currentY;  // 鼠标y方向移动距离
                    if (params10.kind === "n") {
                        //上拉伸
                        //高度增加或减小，位置上下移动
                        if (parseInt(params10.top) + disY > 0) {
                            target.style.top = parseInt(params10.top) + disY + "px";
                            target.style.height = parseInt(params10.height) - disY + "px";
                        }
                    } else if (params10.kind === "w") { //左拉伸
                        if (parseInt(params10.left) + disX > 0) {
                            target.style.left = parseInt(params10.left) + disX + "px";
                            target.style.width = parseInt(params10.width) - disX + "px";
                        }
                    } else if (params10.kind === "e") { //右拉伸
                        if (parseInt(params10.width) + disX + parseInt(params10.left) < bgwidth10)
                           target.style.width = parseInt(params10.width) + disX + "px";
                    } else if (params10.kind === "s") { //下拉伸
                        if (parseInt(params10.height) + disY + parseInt(params10.top) < bgheight10)
                            target.style.height = parseInt(params10.height) + disY + "px";
                    } else if (params10.kind === "nw") { //左上拉伸
                        if (parseInt(params10.left) + disX > 0 && parseInt(params10.top) + disY > 0) {
                            target.style.left = parseInt(params10.left) + disX + "px";
                            target.style.width = parseInt(params10.width) - disX + "px";
                            target.style.top = parseInt(params10.top) + disY + "px";
                            target.style.height = parseInt(params10.height) - disY + "px";
                        }
                    } else if (params10.kind === "ne") { //右上拉伸
                        if (parseInt(params10.width) + disX + parseInt(params10.left) < bgwidth10 && parseInt(params10.top) + disY > 0) {
                            target.style.top = parseInt(params10.top) + disY + "px";
                            target.style.height = parseInt(params10.height) - disY + "px";
                            target.style.width = parseInt(params10.width) + disX + "px";
                        }
                    } else if (params10.kind === "sw") { //左下拉伸
                        if (parseInt(params10.left) + disX > 0 && parseInt(params10.height) + disY + parseInt(params10.top) < bgheight10) {
                            target.style.left = parseInt(params10.left) + disX + "px";
                            target.style.width = parseInt(params10.width) - disX + "px";
                            target.style.height = parseInt(params10.height) + disY + "px";
                        }
                    } else if (params10.kind === "se") { //右下拉伸
                        if (parseInt(params10.width) + disX + parseInt(params10.left) < bgwidth10 && parseInt(params10.height) + disY + parseInt(params10.top) < bgheight10) {
                            target.style.width = parseInt(params10.width) + disX + "px";
                            target.style.height = parseInt(params10.height) + disY + "px";
                        }
                    } else { //移动
                        if (parseInt(params10.left) + disX < 0) { target.style.left = 0 + "px"; }//最左侧为0,小于0阻止移动
                        //最右侧，417-截图框宽度为left最大距离，超过阻止移动
                        else if (parseInt(params10.left) + disX + parseInt(ID("zxxCropBox10").style.width) > bgwidth10) { target.style.left = bgwidth10 - parseInt(ID("zxxCropBox10").style.width) + "px"; }
                        else target.style.left = parseInt(params10.left) + disX + "px";//正常范围
                        if (parseInt(params10.top) + disY < 0) { target.style.top = 0 }//高度小于0,阻止向上移动
                        else if (parseInt(params10.top) + disY + parseInt(ID("zxxCropBox10").style.height) > bgheight10)//移动高度+控件高度超过220，阻止移动
                        { target.style.top = bgheight10 - parseInt(ID("zxxCropBox10").style.height) + "px"; }
                        else { target.style.top = parseInt(params10.top) + disY + "px"; }//正常高度范围内
                    }
                }

                document.onmouseup = function () {
                    document.onmousemove = null;
                    document.onmouseup = null;
                    params10.flag = false;
                    if (getCss10(target, "left") !== "auto") {
                        params10.left = getCss10(target, "left");
                    }
                    if (getCss10(target, "top") !== "auto") {
                        params10.top = getCss10(target, "top");
                    }
                    params10.width = getCss10(target, "width");
                    params10.height = getCss10(target, "height");
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
                    var right = posX + parseInt(ID("zxxCropBox10").style.width);
                    if (right < 0) right = 0;
                    var down = posY + parseInt(ID("zxxCropBox10").style.height);
                    if (down < 0) down = 0;
                    ID("left10").value = posX;
                    ID("up10").value = posY;
                    ID("right10").value = right;
                    ID("down10").value = down;
                    ID("cropImageWidth10").value = parseInt(ID("zxxCropBox10").style.width);
                    ID("cropImageHeight10").value = parseInt(ID("zxxCropBox10").style.height);
                };
            }
        };
    };

    //绑定拖拽
    startDrag10(ID10("zxxDragBg10"), ID10("zxxCropBox10"), "drag");
    //绑定拉伸
    startDrag10(ID10("dragLeftTop10"), ID10("zxxCropBox10"), "nw");
    startDrag10(ID10("dragLeftBot10"), ID10("zxxCropBox10"), "sw");
    startDrag10(ID10("dragRightTop10"), ID10("zxxCropBox10"), "ne");
    startDrag10(ID10("dragRightBot10"), ID10("zxxCropBox10"), "se");
    startDrag10(ID10("dragTopCenter10"), ID10("zxxCropBox10"), "n");
    startDrag10(ID10("dragBotCenter10"), ID10("zxxCropBox10"), "s");
    startDrag10(ID10("dragRightCenter10"), ID10("zxxCropBox10"), "e");
    startDrag10(ID10("dragLeftCenter10"), ID10("zxxCropBox10"), "w");


    //图片不能被选中，目的在于使拖拽顺滑
    ID10("myCanvas10").onselectstart = function () {
        return false;
    };
    img10.onselectstart = function () {
        return false;
    };
}

//确认裁剪
function RequeryCrop10() {
    var x = document.getElementById("left10").value;
    var y = document.getElementById("up10").value;
    var w = document.getElementById("cropImageWidth10").value;
    var h = document.getElementById("cropImageHeight10").value;
    console.log('cropImage(img,', x, ',', y, ',', parseInt(w), ',', parseInt(h), ')');
   // cropImage10(img10, x / scaleX10, y / scaleY10, parseInt(w) / scaleX10, parseInt(h) / scaleY10);
}


function cropImage10(img, cropPosX, cropPosY, width, height) {
    /*var cropContainer = ID("cropContainer");
    cropContainer.parentNode.removeChild(cropContainer);*/
    /*ctx.clearRect(0, 0, myCanvas.width, myCanvas.height);*/
    //sx,sy 是相对于图片的坐标。巨坑

    $("#imgDiv10").html('');
    var myCanvas = document.createElement('canvas');
    myCanvas.setAttribute('id', 'myCanvas10');
    myCanvas.style.display = 'block';
    /*myCanvas.style.position = 'absolute';*/
    myCanvas.width = width * scaleX10;
    myCanvas.height = height * scaleY10;
    myCanvas.style.border = "1px solid #d3d3d3";
    myCanvas.innerText = '您的浏览器不支持 HTML5 canvas 标签。';
    myCanvas.style.zIndex = 'auto';

    var newCtx = myCanvas.getContext('2d');
    newCtx.drawImage(img10, cropPosX, cropPosY, width, height, 0, 0, width * scaleX10, height * scaleY10);

    // canvas转化为图片
    var newImage = new Image();
    newImage.src = myCanvas.toDataURL("image/png");
    //newImage.style.marginLeft = '5px';
    newImage.width = bgwidth10;
    newImage.height = bgheight10;
    //clipImgDiv.appendChild(newImage);
    imgDiv10.appendChild(newImage);
    oRelDiv10.innerHTML = '';
}
