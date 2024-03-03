let originWidth2; // 图片原始宽度
let originHeigh2; // 图片原始高度
let container2 = document.getElementById('container2');
let imgDiv2 = document.getElementById('imgDiv2');    // 存放mycanvas
//let btnDiv = document.getElementById('btnDiv1');
let clipImgDiv2 = document.getElementById('clipImgDiv2');    // 显示裁剪所获的图片
var oRelDiv2 = document.createElement("div"); // 截图框
var scaleX2 = 1;// 图片宽度缩放比例（当前实际/原始）
var scaleY2 = 1;  // 图片高度缩放比例（当前实际/原始）
var bgwidth2 = 417;
var bgheight2 = 220;
//拖拽与拉伸方法
//拖拽拉伸所需参数
let params2 = {
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
container2.style.display = 'flex';
container2.style.flexDirection = 'column';
//btnDiv.style.marginBottom = '20px';
//btnDiv.style.height = '30px';
imgDiv2.style.marginBottom = '20px';

/**********************************************************************/
$(function () {
    ExecuteCanvas2( );
})

  //  var img2 = new Image();

function ExecuteCanvas2(url) 
{
    // 创建canvas，用于显示被裁剪图片
    var myCanvas2 = document.createElement('canvas');
    myCanvas2.setAttribute('id', 'myCanvas2');
    myCanvas2.style.display = 'block';
    /*myCanvas.style.position = 'absolute';*/
    myCanvas2.width = bgwidth2;
    myCanvas2.height = bgheight2;
    myCanvas2.style.border = "1px solid #d3d3d3";
    myCanvas2.innerText = '您的浏览器不支持 HTML5 canvas 标签。';
    myCanvas2.style.zIndex = 'auto';

   // var ctx2 = myCanvas2.getContext('2d');

    // 被裁剪图片
    //     img2.src = image2;
    // img2.setAttribute('id', 'img');
    // img2.width = bgwidth2;
    // img2.height = bgheight2;
    // img2.onload = function () {
    // console.log('onload()执行...');
    // ctx2.drawImage(img2, 0, 0, bgwidth2, bgheight2);
    // originWidth2 = img2.naturalWidth;
    // originHeight2 = img2.naturalHeight;
    imgDiv2.appendChild(myCanvas2);
    // console.log('图片原始宽度=', originWidth2);
    // console.log('图片原始高度=', originHeight2);
    if ( camera_play2 != null)
    {
         camera_play2.stop();
        // camera_play2 = null;
    }

    camera_play2 = new Player();
    camera_play2.play("",/*url*/ url /*'udp://@224.1.1.3:20000'*/ , myCanvas2);
    originWidth2 = bgwidth2; //camera_play1.width();
    originHeigh2 = bgheight2; //camera_play1.height();

    console.log('2  width2 = ' + originWidth2 + ', originHeigh2 = ' + originHeigh2) ;
    cutImg2();
};
/**********************************************************************/

// imgDiv2.appendChild(myCanvas2);

// }
// 获取指定元素DOM
const ID2 = function (id) {
    return document.getElementById(id);
};

//获取相关CSS属性方法
let getCss2 = function (o, key) {
    return o.currentStyle ? o.currentStyle[key] : document.defaultView.getComputedStyle(o, false)[key];
};


// 裁剪得到的图片
// let clipImg2 = new Image();
// clipImg2.src = '';
// clipImg2.style.height = '100px';
// clipImg2.style.width = '100px';
// clipImg2.alt = '裁剪获得图片...';

//$(function () {
//    cutImg();
//});


function cutImg2() 
{
    var clickFlag = false;
    // 获取canvas中图片实际大小
    var iCurWidth = originWidth2; /// img2.width;
    var iCurHeight = originHeigh2; //img2.height;
    console.log('图片当前实际宽度=', iCurWidth);
    console.log('图片当前实际高度=', iCurHeight);

    // 可调整截图框
    oRelDiv2.innerHTML = '';
    oRelDiv2.style.position = "absolute";
    oRelDiv2.style.width = iCurWidth + "px";
    oRelDiv2.style.height = iCurHeight + "px";
    oRelDiv2.style.top ="92px"; //myCanvas2.offsetTop + 'px';
    console.log('oRelDiv.style.top = ', oRelDiv2.style.top);
    oRelDiv2.id = "cropContainer2";

    var iOrigWidth = originWidth2;
    var iOrigHeight = originHeigh2;
    scaleX2 = iCurWidth / iOrigWidth; // 图片宽度缩放比例（当前实际/原始）
    scaleY2 = iCurHeight / iOrigHeight;  // 图片高度缩放比例（当前实际/原始）
    console.log('图片横向（宽度）缩放比=', scaleX2);
    console.log('图片纵向（高度）缩放比=', scaleY2);

    // 将oRelDiv插入到myCanvas前
    myCanvas2.parentNode.insertBefore(oRelDiv2, myCanvas2);

    //初始化坐标与剪裁高宽
    //var cropW = 100; //截图框默认宽度
   // var cropH = 100; //截图框默认高度
    /*console.log('myCanvas.offsetLeft=', myCanvas.offsetLeft);
    console.log('myCanvas.offsetTop=', myCanvas.offsetTop);*/
    //var posX = 0; //myCanvas.width / 2 - cropW / 2;  // 截图框左上角x坐标
    //var posY = 0; //myCanvas.height / 2 - cropH / 2;    // 截图框左上角y坐标
    /*console.log('posX=',posX);
    console.log('posY=',posY);*/
    // if($("#up2").val()=="") {$("#up2").val(0);}
    // if($("#left2").val()=="") {$("#left2").val(0);}
    // if($("#right2").val()=="") {$("#right2").val(100);}
    // if($("#down2").val()=="") {$("#down2").val(100);}
    var cropW=parseInt($("#right2").val())-parseInt($("#left2").val());
    var cropH=parseInt($("#down2").val())-parseInt($("#up2").val());
    var posY = parseInt($("#up2").val());
    var posX = parseInt($("#left2").val());

    oRelDiv2.innerHTML = '<div id="zxxCropBox2" style="height:' + cropH + 'px; width:' + cropW + 'px; position:absolute; left:' +
        posX + 'px; top:' + posY + 'px; border:1px solid black;">' +
        '<div id="zxxDragBg2" style="height:100%; background:white; opacity:0.3; filter:alpha(opacity=30); cursor:move"></div>' +
        '<div id="dragLeftTop2" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; top:-3px; cursor:nw-resize;"></div>' +
        '<div id="dragLeftBot2" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; bottom:-3px; cursor:sw-resize;"></div>' +
        '<div id="dragRightTop2" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; top:-3px; cursor:ne-resize;"></div>' +
        '<div id="dragRightBot2" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; bottom:-3px; cursor:se-resize;"></div>' +
        '<div id="dragTopCenter2" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; top:-3px; left:50%; margin-left:-3px; cursor:n-resize;"></div>' +
        '<div id="dragBotCenter2" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; bottom:-3px; left:50%; margin-left:-3px; cursor:s-resize;"></div>' +
        '<div id="dragRightCenter2" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; top:50%; margin-top:-3px; cursor:e-resize;"></div> ' +
        '<div id="dragLeftCenter2" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; top:50%; margin-top:-3px; cursor:w-resize;"></div>' +
        '</div>';
  
    // $("#up2").val(0);
    // $("#left2").val(0);
    // $("#down2").val(100);
    // $("#right2").val(100);
    if($("#up2").val()=="")
  $("#up2").val(0);
if($("#left2").val()=="")
$("#left2").val(0);
if($("#down2").val()=="")
$("#down2").val(100);
if($("#right2").val()=="")
$("#right2").val(100);
    ID("cropImageWidth2").value = parseInt(ID("zxxCropBox2").style.width);
    ID("cropImageHeight2").value = parseInt(ID("zxxCropBox2").style.height);

    var startDrag2 = function (point, target, kind) {
        //point是拉伸点，target是被拉伸的目标，其高度及位置会发生改变
        //此处的target与上面拖拽的target是同一目标，故其params.left,params.top可以共用，也必须共用
        //初始化宽高
        params2.width = getCss2(target, "width");
        params2.height = getCss2(target, "height");
        //初始化坐标
        if (getCss2(target, "left") !== "auto") {
            params2.left = getCss2(target, "left");
        }
        if (getCss2(target, "top") !== "auto") {
            params2.top = getCss2(target, "top");
        }
        //target是移动对象
        point.onmousedown = function (event) 
        {
            console.log('---->>>> dsflkjd down');
            params2.kind = kind;
            params2.flag = true;
            clickFlag = true;
            if (!event) {
                event = window.event;
            }
            var e = event;
            params2.currentX = e.clientX;  //鼠标按下时坐标x轴
            params2.currentY = e.clientY;  //鼠标按下时坐标y轴
            /*console.log('params.currentX=', params.currentX);
            console.log('params.currentY=', params.currentY);*/
            //防止IE文字选中，有助于拖拽平滑
            point.onselectstart = function () {
                return false;
            };

            document.onmousemove = function (event) 
            {
                console.log('mouse move --> 2');
                let e = event ? event : window.event;
                clickFlag = false;
                if (params2.flag) {
                    var nowX = e.clientX; // 鼠标移动时x坐标
                    var nowY = e.clientY;   // 鼠标移动时y坐标
                    var disX = nowX - params2.currentX;  // 鼠标x方向移动距离
                    var disY = nowY - params2.currentY;  // 鼠标y方向移动距离
                    if (params2.kind === "n") {
                        //上拉伸
                        //高度增加或减小，位置上下移动
                        if (parseInt(params2.top) + disY > 0) {
                            target.style.top = parseInt(params2.top) + disY + "px";
                            target.style.height = parseInt(params2.height) - disY + "px";
                        }
                    } else if (params2.kind === "w") { //左拉伸
                        if (parseInt(params2.left) + disX > 0) {
                            target.style.left = parseInt(params2.left) + disX + "px";
                            target.style.width = parseInt(params2.width) - disX + "px";
                        }
                    } else if (params2.kind === "e") { //右拉伸
                        if (parseInt(params2.width) + disX + parseInt(params2.left) < bgwidth2)
                           target.style.width = parseInt(params2.width) + disX + "px";
                    } else if (params2.kind === "s") { //下拉伸
                        if (parseInt(params2.height) + disY + parseInt(params2.top) < bgheight2) 
                           target.style.height = parseInt(params2.height) + disY + "px";
                    } else if (params2.kind === "nw") { //左上拉伸
                        if (parseInt(params2.left) + disX > 0 && parseInt(params2.top) + disY > 0) {
                            target.style.left = parseInt(params2.left) + disX + "px";
                            target.style.width = parseInt(params2.width) - disX + "px";
                            target.style.top = parseInt(params2.top) + disY + "px";
                            target.style.height = parseInt(params2.height) - disY + "px";
                        }
                    } else if (params2.kind === "ne") { //右上拉伸
                        if (parseInt(params2.width) + disX + parseInt(params2.left) < bgwidth2 && parseInt(params2.top) + disY > 0) {
                            target.style.top = parseInt(params2.top) + disY + "px";
                            target.style.height = parseInt(params2.height) - disY + "px";
                            target.style.width = parseInt(params2.width) + disX + "px";
                        }
                    } else if (params2.kind === "sw") { //左下拉伸
                        if (parseInt(params2.left) + disX > 0 && parseInt(params2.height) + disY + parseInt(params2.top) < bgheight2) {
                            target.style.left = parseInt(params2.left) + disX + "px";
                            target.style.width = parseInt(params2.width) - disX + "px";
                            target.style.height = parseInt(params2.height) + disY + "px";
                        }
                    } else if (params2.kind === "se") { //右下拉伸
                        if (parseInt(params2.width) + disX + parseInt(params2.left) < bgwidth2 && parseInt(params2.height) + disY + parseInt(params2.top) < bgheight2) {
                            target.style.width = parseInt(params2.width) + disX + "px";
                            target.style.height = parseInt(params2.height) + disY + "px";
                        }
                    } else { //移动
                        if (parseInt(params2.left) + disX < 0) { target.style.left = 0 + "px"; }//最左侧为0,小于0阻止移动
                        //最右侧，417-截图框宽度为left最大距离，超过阻止移动
                        else if (parseInt(params2.left) + disX + parseInt(ID("zxxCropBox2").style.width) > bgwidth2) { target.style.left = bgwidth2 - parseInt(ID("zxxCropBox2").style.width) + "px"; }
                        else target.style.left = parseInt(params2.left) + disX + "px";//正常范围
                        if (parseInt(params2.top) + disY < 0) { target.style.top = 0 }//高度小于0,阻止向上移动
                        else if (parseInt(params2.top) + disY + parseInt(ID("zxxCropBox2").style.height) > bgheight2)//移动高度+控件高度超过220，阻止移动
                        { target.style.top = bgheight2 - parseInt(ID("zxxCropBox2").style.height) + "px"; }
                        else { target.style.top = parseInt(params2.top) + disY + "px"; }//正常高度范围内
                    }
                }

                document.onmouseup = function () {
                    document.onmousemove = null;
                    document.onmouseup = null;
                    params2.flag = false;
                    if (getCss2(target, "left") !== "auto") {
                        params2.left = getCss2(target, "left");
                    }
                    if (getCss2(target, "top") !== "auto") {
                        params2.top = getCss2(target, "top");
                    }
                    params2.width = getCss2(target, "width");
                    params2.height = getCss2(target, "height");
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
                    var right = posX + parseInt(ID("zxxCropBox2").style.width);
                    if (right < 0) right = 0;
                    var down =  posY + parseInt(ID("zxxCropBox2").style.height);
                    if (down < 0) down = 0;
                    ID("left2").value = posX;
                    ID("up2").value = posY;
                    ID("right2").value = right;
                    ID("down2").value = down;
                    ID("cropImageWidth2").value = parseInt(ID("zxxCropBox2").style.width);
                    ID("cropImageHeight2").value = parseInt(ID("zxxCropBox2").style.height);
                };
            }
        };
    };

    //绑定拖拽
    startDrag2(ID2("zxxDragBg2"), ID2("zxxCropBox2"), "drag");
    //绑定拉伸
    startDrag2(ID2("dragLeftTop2"), ID2("zxxCropBox2"), "nw");
    startDrag2(ID2("dragLeftBot2"), ID2("zxxCropBox2"), "sw");
    startDrag2(ID2("dragRightTop2"), ID2("zxxCropBox2"), "ne");
    startDrag2(ID2("dragRightBot2"), ID2("zxxCropBox2"), "se");
    startDrag2(ID2("dragTopCenter2"), ID2("zxxCropBox2"), "n");
    startDrag2(ID2("dragBotCenter2"), ID2("zxxCropBox2"), "s");
    startDrag2(ID2("dragRightCenter2"), ID2("zxxCropBox2"), "e");
    startDrag2(ID2("dragLeftCenter2"), ID2("zxxCropBox2"), "w");


    //图片不能被选中，目的在于使拖拽顺滑
    ID2("myCanvas2").onselectstart = function () 
    {
        console.log('onselectstart --> ');
        return false;
    };
    // img2.onselectstart = function () 
    // {
    //     console.log('onselectstart --->>>');
    //     return false;
    // };
}

//确认裁剪
function RequeryCrop2() {
    var x = document.getElementById("left2").value;
    var y = document.getElementById("up2").value;
    var w = document.getElementById("cropImageWidth2").value;
    var h = document.getElementById("cropImageHeight2").value;
    console.log('cropImage(img,', x, ',', y, ',', parseInt(w), ',', parseInt(h), ')');
   // cropImage2(img2, x / scaleX2, y / scaleY2, parseInt(w) / scaleX2, parseInt(h) / scaleY2);
}


function cropImage2(img, cropPosX, cropPosY, width, height) {
    /*var cropContainer = ID("cropContainer");
    cropContainer.parentNode.removeChild(cropContainer);*/
    /*ctx.clearRect(0, 0, myCanvas.width, myCanvas.height);*/
    //sx,sy 是相对于图片的坐标。巨坑

    $("#imgDiv2").html('');
    var myCanvas = document.createElement('canvas');
    myCanvas.setAttribute('id', 'myCanvas2');
    myCanvas.style.display = 'block';
    /*myCanvas.style.position = 'absolute';*/
    myCanvas.width = width * scaleX2;
    myCanvas.height = height * scaleY2;
    myCanvas.style.border = "1px solid #d3d3d3";
    myCanvas.innerText = '您的浏览器不支持 HTML5 canvas 标签。';
    myCanvas.style.zIndex = 'auto';

    var newCtx = myCanvas.getContext('2d');
    newCtx.drawImage(img2, cropPosX, cropPosY, width, height, 0, 0, width * scaleX2, height * scaleY2);


    //var newCanvas = document.createElement('canvas');
    //newCanvas.setAttribute('id', 'newCanvas');
    //newCanvas.width = width * scaleX;
    //newCanvas.height = height * scaleY;
    //newCanvas.style.border = "1px solid #d3d3d3";
    //var newCtx = newCanvas.getContext('2d');
    //clipImgDiv.appendChild(newCanvas);
    //newCtx.drawImage(img, cropPosX, cropPosY, width, height, 0, 0, width * scaleX, height * scaleY);

    // canvas转化为图片
    var newImage = new Image();
    newImage.src = myCanvas.toDataURL("image/png");
    //newImage.style.marginLeft = '5px';
    newImage.width = bgwidth2;
    newImage.height = bgheight2;
    //clipImgDiv.appendChild(newImage);
    imgDiv2.appendChild(newImage);
    oRelDiv2.innerHTML = '';
}
