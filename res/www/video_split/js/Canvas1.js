let originWidth; // 图片原始宽度
let originHeigh; // 图片原始高度
let container = document.getElementById('container1');
let imgDiv = document.getElementById('imgDiv1');    // 存放mycanvas
//let btnDiv = document.getElementById('btnDiv1');
let clipImgDiv = document.getElementById('clipImgDiv1');    // 显示裁剪所获的图片
var oRelDiv = document.createElement("div"); // 截图框
var scaleX = 1;// 图片宽度缩放比例（当前实际/原始）
var scaleY = 1;  // 图片高度缩放比例（当前实际/原始）
var bgwidth = 417;
var bgheight = 220;
console.log('Canvas1-->>>');
//拖拽与拉伸方法
//拖拽拉伸所需参数
let params = {
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
container.style.display = 'flex';
container.style.flexDirection = 'column';
//btnDiv.style.marginBottom = '20px';
//btnDiv.style.height = '30px';
imgDiv.style.marginBottom = '20px';

/**********************************************************************/
// 创建canvas，用于显示被裁剪图片\
$(function () {
    ExecuteCanvas1();
})

// var img = new Image();

function ExecuteCanvas1( url ) 
{
	console.log('ExecuteCanvas1 --->>');
	var myCanvas = document.createElement('canvas');
	myCanvas.setAttribute('id', 'myCanvas');
	myCanvas.style.display = 'block';
	/*myCanvas.style.position = 'absolute';*/
	myCanvas.width = bgwidth;
	myCanvas.height = bgheight;
	myCanvas.style.border = "1px solid #d3d3d3";
	myCanvas.innerText = '您的浏览器不支持 HTML5 canvas 标签。';
	myCanvas.style.zIndex = 'auto';

	//var ctx = myCanvas.getContext('2d');

	// 被裁剪图片
	   
	//	img.src = image1;
	//img.setAttribute('id', 'img');
	//img.width = bgwidth;
	//img.height = bgheight;
	imgDiv.appendChild(myCanvas);
	// container.appendChild(myCanvas);
	//  img.onload = function () 
	//{
	//	console.log('onload()执行...');
	//	ctx.drawImage(img, 0, 0, bgwidth, bgheight);
	//	originWidth = img.naturalWidth;
	//	originHeight = img.naturalHeight;
	//	console.log('图片原始宽度=', originWidth);
	//	console.log('图片原始高度=', originHeight);
	//
	//	//cutImg();
	//};
	console.log('Canvas -- url = ' + url);
/**********************************************************************/
	camera_play1 = new Player();
	camera_play1.play("",/*url*/ url /*'udp://@224.1.1.3:20000'*/ , myCanvas);
    originWidth = bgwidth; //camera_play1.width();
    originHeigh = bgheight; //camera_play1.height();

    console.log('width = ' + originWidth + ', originHeigh = ' + originHeigh);
	cutImg();
	
}

// 获取指定元素DOM
const ID = function (id) 
{
    return document.getElementById(id);
};

//获取相关CSS属性方法
let getCss = function (o, key) {
    return o.currentStyle ? o.currentStyle[key] : document.defaultView.getComputedStyle(o, false)[key];
};



// 裁剪得到的图片
// let clipImg = new Image();
// clipImg.src = '';
// clipImg.style.height = '100px';
// clipImg.style.width = '100px';
// clipImg.alt = '裁剪获得图片...';



function cutImg() 
{
    var clickFlag = false;
    // 获取canvas中图片实际大小
    var iCurWidth = originWidth; //img.width;
    var iCurHeight = originHeigh; //img.height;
    console.log('图片当前实际宽度=', iCurWidth);
    console.log('图片当前实际高度=', iCurHeight);

    // 可调整截图框
    oRelDiv.innerHTML = '';
    oRelDiv.style.position = "absolute";
    oRelDiv.style.width = iCurWidth + "px";
    oRelDiv.style.height = iCurHeight + "px";
    oRelDiv.style.top ="92px"; //myCanvas.offsetTop + 'px';
    console.log('oRelDiv.style.top = ', oRelDiv.style.top);
    oRelDiv.id = "cropContainer";

    var iOrigWidth = originWidth;
    var iOrigHeight = originHeigh;
    scaleX = iCurWidth / iOrigWidth; // 图片宽度缩放比例（当前实际/原始）
    scaleY = iCurHeight / iOrigHeight;  // 图片高度缩放比例（当前实际/原始）
    console.log('图片横向（宽度）缩放比=', scaleX);
    console.log('图片纵向（高度）缩放比=', scaleY);

    // 将oRelDiv插入到myCanvas前
    myCanvas.parentNode.insertBefore(oRelDiv, myCanvas);

    //初始化坐标与剪裁高宽
    //var cropW = 100; //截图框默认宽度
    //var cropH = 100; //截图框默认高度
    /*console.log('myCanvas.offsetLeft=', myCanvas.offsetLeft);
    console.log('myCanvas.offsetTop=', myCanvas.offsetTop);*/
   // var posX = 0; //myCanvas.width / 2 - cropW / 2;  // 截图框左上角x坐标
// var posY = 0; //myCanvas.height / 2 - cropH / 2;    // 截图框左上角y坐标
    /*console.log('posX=',posX);
    console.log('posY=',posY);*/ 
    // if($("#up1").val()=="") {$("#up1").val(0);}
    // if($("#left1").val()=="") {$("#left1").val(0);}
    // if($("#right1").val()=="") {$("#right1").val(100);}
    // if($("#down1").val()=="") {$("#down1").val(100);}
    var cropW=parseInt($("#right1").val())-parseInt($("#left1").val());
    var cropH=parseInt($("#down1").val())-parseInt($("#up1").val());
    var posY = parseInt($("#up1").val());
    var posX = parseInt($("#left1").val());

    oRelDiv.innerHTML = '<div id="zxxCropBox1" style="height:' + cropH + 'px; width:' + cropW + 'px; position:absolute; left:' +
        posX + 'px; top:' + posY + 'px; border:1px solid black;">' +
        '<div id="zxxDragBg" style="height:100%; background:white; opacity:0.3; filter:alpha(opacity=30); cursor:move"></div>' +
        '<div id="dragLeftTop" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; top:-3px; cursor:nw-resize;"></div>' +
        '<div id="dragLeftBot" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; bottom:-3px; cursor:sw-resize;"></div>' +
        '<div id="dragRightTop" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; top:-3px; cursor:ne-resize;"></div>' +
        '<div id="dragRightBot" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; bottom:-3px; cursor:se-resize;"></div>' +
        '<div id="dragTopCenter" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; top:-3px; left:50%; margin-left:-3px; cursor:n-resize;"></div>' +
        '<div id="dragBotCenter" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; bottom:-3px; left:50%; margin-left:-3px; cursor:s-resize;"></div>' +
        '<div id="dragRightCenter" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; top:50%; margin-top:-3px; cursor:e-resize;"></div> ' +
        '<div id="dragLeftCenter" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; top:50%; margin-top:-3px; cursor:w-resize;"></div>' +
        '</div>';
    //+ '<input type="text" id="cropPosX" value="' + posX / scaleX + '" style="position:relative; top: -26px; width: 30px"/>' +
    //'<input type="text" id="cropPosY" value="' + posY / scaleY + '" style="position:relative; top: -26px; width: 30px"/>' +
    //'<input type="text" id="cropImageWidth" value="' + cropW / scaleX + '" style="position:relative; top: -26px; width: 30px"/>' +
    //'<input type="text" id="cropImageHeight" value="' + cropH / scaleY + '" style="position:relative; top: -26px; width: 30px"/>';

    if($("#up1").val()=="")
    {
        $("#up1").val(0);
    }
    if($("#left1").val()=="")
    {
        $("#left1").val(0);
    }
    if($("#down1").val()=="")
    {
        $("#down1").val(100);
    }
    if($("#right1").val()=="")
    {
        $("#right1").val(100);
    }
    // $("#up1").val(0);
    // $("#left1").val(0);
    // $("#down1").val(100);
    // $("#right1").val(100);
    let cropImageWidth1_id =  document.getElementById('cropImageWidth1');
    console.log('cropImageWidth1_id  = ' + cropImageWidth1_id);
    let cropImageHeight1_id =  document.getElementById('cropImageHeight1');
    console.log('cropImageHeight1_id = ' + cropImageHeight1_id);
    ID("cropImageWidth1").value = originWidth; //parseInt(ID("myCanvas").width);
    ID("cropImageHeight1").value = originHeigh; //parseInt(ID("myCanvas").height);

    var startDrag = function (point, target, kind) 
    {
        if (point === null)
        {
            console.log('point == null' + ', target = ' + target + ',kind = ' + kind);
            return;
        }
        //point是拉伸点，target是被拉伸的目标，其高度及位置会发生改变
        //此处的target与上面拖拽的target是同一目标，故其params.left,params.top可以共用，也必须共用
        //初始化宽高
        params.width = getCss(target, "width");
        params.height = getCss(target, "height");
        //初始化坐标
        if (getCss(target, "left") !== "auto") {
            params.left = getCss(target, "left");
        }
        if (getCss(target, "top") !== "auto") {
            params.top = getCss(target, "top");
        }
        //target是移动对象
       // point.addEventListener('onmousedown', function(event)
        point.onmousedown = function()
        {
            console.log('------>>>>>>>>>>>>>>>>> down===>>>');
            params.kind = kind;
            params.flag = true;
            clickFlag = true;
            if (!event) {
                event = window.event;
            }
            var e = event;
            params.currentX = e.clientX;  //鼠标按下时坐标x轴
            params.currentY = e.clientY;  //鼠标按下时坐标y轴
            /*console.log('params.currentX=', params.currentX);
            console.log('params.currentY=', params.currentY);*/
            //防止IE文字选中，有助于拖拽平滑
           point.onselectstart = function () {
               return false;
           };
              
             // document.addEventListener('onmousemove', function(event){
           document.onmousemove = function(event)
           {
               //  console.log(event);
                //if (iscrop) return false;
                let e = event ? event : window.event;
                clickFlag = false;
                if (params.flag) {
                    var nowX = e.clientX; // 鼠标移动时x坐标
                    var nowY = e.clientY;   // 鼠标移动时y坐标
                    var disX = nowX - params.currentX;  // 鼠标x方向移动距离
                    var disY = nowY - params.currentY;  // 鼠标y方向移动距离
                    if (params.kind === "n") {
                        //上拉伸
                        //高度增加或减小，位置上下移动
                        if (parseInt(params.top) + disY > 0) {
                            target.style.top = parseInt(params.top) + disY + "px";
                            target.style.height = parseInt(params.height) - disY + "px";
                        }
                    } else if (params.kind === "w") { //左拉伸
                        if (parseInt(params.left) + disX > 0) {
                            target.style.left = parseInt(params.left) + disX + "px";
                            target.style.width = parseInt(params.width) - disX + "px";
                        }
                    } else if (params.kind === "e") { //右拉伸
                        if (parseInt(params.width) + disX + parseInt(params.left) < bgwidth)
                            target.style.width = parseInt(params.width) + disX + "px";
                    } else if (params.kind === "s") { //下拉伸
                        if (parseInt(params.height) + disY + parseInt(params.top) < bgheight)
                            target.style.height = parseInt(params.height) + disY + "px";
                    } else if (params.kind === "nw") { //左上拉伸
                        if (parseInt(params.left) + disX > 0 && parseInt(params.top) + disY > 0) {
                            target.style.left = parseInt(params.left) + disX + "px";
                            target.style.width = parseInt(params.width) - disX + "px";
                            target.style.top = parseInt(params.top) + disY + "px";
                            target.style.height = parseInt(params.height) - disY + "px";
                        }
                    } else if (params.kind === "ne") { //右上拉伸
                        if (parseInt(params.width) + disX + parseInt(params.left) < bgwidth && parseInt(params.top) + disY > 0) {
                            target.style.top = parseInt(params.top) + disY + "px";
                            target.style.height = parseInt(params.height) - disY + "px";
                            target.style.width = parseInt(params.width) + disX + "px";
                        }
                    } else if (params.kind === "sw") { //左下拉伸
                        if (parseInt(params.left) + disX > 0 && parseInt(params.height) + disY + parseInt(params.top) < bgheight) {
                            target.style.left = parseInt(params.left) + disX + "px";
                            target.style.width = parseInt(params.width) - disX + "px";
                            target.style.height = parseInt(params.height) + disY + "px";
                        }
                    } else if (params.kind === "se") { //右下拉伸
                        if (parseInt(params.width) + disX + parseInt(params.left) < bgwidth && parseInt(params.height) + disY + parseInt(params.top) < bgheight) {
                            target.style.width = parseInt(params.width) + disX + "px";
                            target.style.height = parseInt(params.height) + disY + "px";
                        }
                    } else { //移动log
                        if (parseInt(params.left) + disX < 0) { target.style.left = 0 + "px"; }//最左侧为0,小于0阻止移动
                        //最右侧，417-截图框宽度为left最大距离，超过阻止移动
                        else if (parseInt(params.left) + disX + parseInt(ID("zxxCropBox1").style.width) > bgwidth) { target.style.left = bgwidth - parseInt(ID("zxxCropBox1").style.width) + "px"; }
                        else target.style.left = parseInt(params.left) + disX + "px";//正常范围
                        if (parseInt(params.top) + disY < 0) { target.style.top = 0 }//高度小于0,阻止向上移动
                        else if (parseInt(params.top) + disY + parseInt(ID("zxxCropBox1").style.height) > bgheight)//移动高度+控件高度超过220，阻止移动
                        { target.style.top = bgheight - parseInt(ID("zxxCropBox1").style.height) + "px"; }
                        else { target.style.top = parseInt(params.top) + disY + "px"; }//正常高度范围内
                    }
                }

                document.onmouseup = function () {
                    document.onmousemove = null;
                    document.onmouseup = null;
                    // if (iscrop) return false;
                    params.flag = false;
                    if (getCss(target, "left") !== "auto") {
                        params.left = getCss(target, "left");
                    }
                    if (getCss(target, "top") !== "auto") {
                        params.top = getCss(target, "top");
                    }
                    params.width = getCss(target, "width");
                    params.height = getCss(target, "height");
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

                    var right = posX + parseInt(ID("zxxCropBox1").style.width);
                    if (right < 0) 
                    {
                        right = 0;
                    }
                    var down = posY + parseInt(ID("zxxCropBox1").style.height);
                    if (down < 0) 
                    {
                        down = 0;
                    }
                    ID("left1").value = posX;
                    ID("up1").value = posY;
                    ID("right1").value = right;
                    ID("down1").value = down;
                    ID("cropImageWidth1").value = parseInt(ID("zxxCropBox1").style.width);
                    ID("cropImageHeight1").value = parseInt(ID("zxxCropBox1").style.height);
                    console.log('posX = ' + posX + ', posY = ' + posY + ', right = ' + right + ', down = ' + down);
                    console.log('cropImageWidth1 = ' + parseInt(ID("zxxCropBox1").style.width) + ', cropImageHeight1 = ' + parseInt(ID("zxxCropBox1").style.height))
                };
             }

         }
        //});    
        
    }
    let myCanvas_dev=  document.getElementById('myCanvas');
    console.log('myCanvas_dev =' + myCanvas_dev);
   
     let zxxCropBox1_dev =  document.getElementById('zxxCropBox1');
    console.log('zxxCropBox1 = ' + zxxCropBox1_dev);
    //绑定拖拽
    
     let zxxDragBg_dev =  document.getElementById('zxxDragBg');
    console.log('zxxDragBg = ' + zxxDragBg_dev);

    startDrag(ID("zxxDragBg"), ID("zxxCropBox1"), "drag");
    //绑定拉伸zxxCropBox
     let dragLeftTop_dev =  document.getElementById('dragLeftTop');
    console.log('dragLeftTop = ' + dragLeftTop_dev);
    startDrag(ID("dragLeftTop"), ID("zxxCropBox1"), "nw");
    startDrag(ID("dragLeftBot"), ID("zxxCropBox1"), "sw");
    startDrag(ID("dragRightTop"), ID("zxxCropBox1"), "ne");
    startDrag(ID("dragRightBot"), ID("zxxCropBox1"), "se");
    startDrag(ID("dragTopCenter"), ID("zxxCropBox1"), "n");
    startDrag(ID("dragBotCenter"), ID("zxxCropBox1"), "s");
    startDrag(ID("dragRightCenter"), ID("zxxCropBox1"), "e");
    startDrag(ID("dragLeftCenter"), ID("zxxCropBox1"), "w");

    //图片不能被选中，目的在于使拖拽顺滑
    // let cropContainer_dev=  document.getElementById('cropContainer');
    // cropContainer_dev.addEventListener('onselectstart', function(event)
    // {
    //     console.log('cropContainer_dev --> onselectstart');
    //     return false;
    // });
    myCanvas_dev.onselectstart = function(event){
        return false;
    };
    // myCanvas_dev.addEventListener('onselectstart', function(event)
    // {
    //     console.log('cropContainer_dev --> onselectstart');
    //     return false;
    // });
    // ID("myCanvas").onselectstart = function () {
    //     return false;
    // };
    // img.onselectstart = function () {
    //     return false;
    // };
}






//确认裁剪
function RequeryCrop() {
    var x = document.getElementById("left1").value;
    var y = document.getElementById("up1").value;
    var w = document.getElementById("cropImageWidth1").value;
    var h = document.getElementById("cropImageHeight1").value;
    console.log('cropImage(img,', x, ',', y, ',', parseInt(w), ',', parseInt(h), ')');
    //cropImage(img, x / scaleX, y / scaleY, parseInt(w) / scaleX, parseInt(h) / scaleY);
}


function cropImage(img, cropPosX, cropPosY, width, height) {
    /*var cropContainer = ID("cropContainer");
    cropContainer.parentNode.removeChild(cropContainer);*/
    /*ctx.clearRect(0, 0, myCanvas.width, myCanvas.height);*/
    //sx,sy 是相对于图片的坐标。巨坑
    console.log('cropImage--->>>>>>>');
    $("#imgDiv1").html('');
    var myCanvas = document.createElement('canvas');
    myCanvas.setAttribute('id', 'myCanvas');
    myCanvas.style.display = 'block';
    /*myCanvas.style.position = 'absolute';*/
    myCanvas.width = width * scaleX;
    myCanvas.height = height * scaleY;
    myCanvas.style.border = "1px solid #d3d3d3";
    myCanvas.innerText = '您的浏览器不支持 HTML5 canvas 标签。';
    myCanvas.style.zIndex = 'auto';

    var newCtx = myCanvas.getContext('2d');
    newCtx.drawImage(img, cropPosX, cropPosY, width, height, 0, 0, width * scaleX, height * scaleY);


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
    newImage.width = bgwidth;
    newImage.height = bgheight;
    //clipImgDiv.appendChild(newImage);
    imgDiv.appendChild(newImage);
    oRelDiv.innerHTML = '';
}

