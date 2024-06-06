// let originWidth3; // 图片原始宽度
// let originHeigh3; // 图片原始高度
// let container3 = document.getElementById('container3');
// let imgDiv3 = document.getElementById('imgDiv3');    // 存放mycanvas
// //let btnDiv = document.getElementById('btnDiv1');
// let clipImgDiv3 = document.getElementById('clipImgDiv3');    // 显示裁剪所获的图片
// var oRelDiv3 = document.createElement("div"); // 截图框
// var scaleX3 = 1;// 图片宽度缩放比例（当前实际/原始）
// var scaleY3 = 1;  // 图片高度缩放比例（当前实际/原始）
// var bgwidth3 = 417;
// var bgheight3 = 220;
// //拖拽与拉伸方法
// //拖拽拉伸所需参数
// let params3 = {
//     left: 0,
//     top: 0,
//     width: 0,
//     height: 0,
//     currentX: 0,
//     currentY: 0,
//     flag: false,
//     kind: "drag"
// };
// // CSS样式修改
// container3.style.display = 'flex';
// container3.style.flexDirection = 'column';
// //btnDiv.style.marginBottom = '20px';
// //btnDiv.style.height = '30px';
// imgDiv3.style.marginBottom = '20px';

// /**********************************************************************/
// $(function () {
//     ExecuteCanvas3( );
// })

//   //  var img2 = new Image();

// function ExecuteCanvas3(url) 
// {
//     // 创建canvas，用于显示被裁剪图片
//     var myCanvas3 = document.createElement('canvas');
//     myCanvas3.setAttribute('id', 'myCanvas3');
//     myCanvas3.style.display = 'block';
//     /*myCanvas.style.position = 'absolute';*/
//     myCanvas3.width = bgwidth3;
//     myCanvas3.height = bgheight3;
//     myCanvas3.style.border = "1px solid #d3d3d3";
//     myCanvas3.innerText = '您的浏览器不支持 HTML5 canvas 标签。';
//     myCanvas3.style.zIndex = 'auto';

    
//     imgDiv3.appendChild(myCanvas3);
//     // console.log('图片原始宽度=', originWidth2);
//     // console.log('图片原始高度=', originHeight2);
//     if ( camera_play3 != null)
//     {
//          camera_play3.stop();
//         // camera_play2 = null;
//     }

//     camera_play3 = new Player();
//     camera_play3.play("",/*url*/ url /*'udp://@224.1.1.3:20000'*/ , myCanvas3);
//     originWidth3 = bgwidth3; //camera_play1.width();
//     originHeigh3 = bgheight3; //camera_play1.height();

//     console.log('3  width3 = ' + originWidth3 + ', originHeigh3 = ' + originHeigh3) ;
//     cutImg3();
// };
// /**********************************************************************/

// // imgDiv2.appendChild(myCanvas2);

// // }
// // 获取指定元素DOM
// const ID3 = function (id) {
//     return document.getElementById(id);
// };

// //获取相关CSS属性方法
// let getCss3 = function (o, key) {
//     return o.currentStyle ? o.currentStyle[key] : document.defaultView.getComputedStyle(o, false)[key];
// };


// // 裁剪得到的图片
// // let clipImg2 = new Image();
// // clipImg2.src = '';
// // clipImg2.style.height = '100px';
// // clipImg2.style.width = '100px';
// // clipImg2.alt = '裁剪获得图片...';

// //$(function () {
// //    cutImg();
// //});


// function cutImg3() 
// {
//     var clickFlag = false;
//     // 获取canvas中图片实际大小
//     var iCurWidth = originWidth3; /// img2.width;
//     var iCurHeight = originHeigh3; //img2.height;
//     console.log('图片当前实际宽度=', iCurWidth);
//     console.log('图片当前实际高度=', iCurHeight);

//     // 可调整截图框
//     oRelDiv3.innerHTML = '';
//     oRelDiv3.style.position = "absolute";
//     oRelDiv3.style.width = iCurWidth + "px";
//     oRelDiv3.style.height = iCurHeight + "px";
//     oRelDiv3.style.top ="92px"; //myCanvas2.offsetTop + 'px';
//     console.log('oRelDiv.style.top = ', oRelDiv3.style.top);
//     oRelDiv3.id = "cropContainer3";

//     var iOrigWidth = originWidth3;
//     var iOrigHeight = originHeigh3;
//     scaleX3 = iCurWidth / iOrigWidth; // 图片宽度缩放比例（当前实际/原始）
//     scaleY3 = iCurHeight / iOrigHeight;  // 图片高度缩放比例（当前实际/原始）
//     console.log('图片横向（宽度）缩放比=', scaleX3);
//     console.log('图片纵向（高度）缩放比=', scaleY3);

//     // 将oRelDiv插入到myCanvas前
//     myCanvas3.parentNode.insertBefore(oRelDiv3, myCanvas3);

//     //初始化坐标与剪裁高宽
//     //var cropW = 100; //截图框默认宽度
//    // var cropH = 100; //截图框默认高度
//     /*console.log('myCanvas.offsetLeft=', myCanvas.offsetLeft);
//     console.log('myCanvas.offsetTop=', myCanvas.offsetTop);*/
//     //var posX = 0; //myCanvas.width / 2 - cropW / 2;  // 截图框左上角x坐标
//     //var posY = 0; //myCanvas.height / 2 - cropH / 2;    // 截图框左上角y坐标
//     /*console.log('posX=',posX);
//     console.log('posY=',posY);*/
//     // if($("#up2").val()=="") {$("#up2").val(0);}
//     // if($("#left2").val()=="") {$("#left2").val(0);}
//     // if($("#right2").val()=="") {$("#right2").val(100);}
//     // if($("#down2").val()=="") {$("#down2").val(100);}
//     var cropW=parseInt($("#right3").val())-parseInt($("#left3").val());
//     var cropH=parseInt($("#down3").val())-parseInt($("#up3").val());
//     var posY = parseInt($("#up3").val());
//     var posX = parseInt($("#left3").val());

//     oRelDiv3.innerHTML = '<div id="zxxCropBox3" style="height:' + cropH + 'px; width:' + cropW + 'px; position:absolute; left:' +
//         posX + 'px; top:' + posY + 'px; border:1px solid black;">' +
//         '<div id="zxxDragBg3" style="height:100%; background:white; opacity:0.3; filter:alpha(opacity=30); cursor:move"></div>' +
//         '<div id="dragLeftTop3" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; top:-3px; cursor:nw-resize;"></div>' +
//         '<div id="dragLeftBot3" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; bottom:-3px; cursor:sw-resize;"></div>' +
//         '<div id="dragRightTop3" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; top:-3px; cursor:ne-resize;"></div>' +
//         '<div id="dragRightBot3" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; bottom:-3px; cursor:se-resize;"></div>' +
//         '<div id="dragTopCenter3" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; top:-3px; left:50%; margin-left:-3px; cursor:n-resize;"></div>' +
//         '<div id="dragBotCenter3" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; bottom:-3px; left:50%; margin-left:-3px; cursor:s-resize;"></div>' +
//         '<div id="dragRightCenter3" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; right:-3px; top:50%; margin-top:-3px; cursor:e-resize;"></div> ' +
//         '<div id="dragLeftCenter3" style="position:absolute; width:4px; height:4px; border:1px solid #000; background:white; overflow:hidden; left:-3px; top:50%; margin-top:-3px; cursor:w-resize;"></div>' +
//         '</div>';
  
    
//     if($("#up3").val()=="")
//     {
//         $("#up3").val(100);
//     }
//     if($("#left3").val()=="")
//     {
//         $("#left3").val(100);
//     }
//     if($("#down3").val()=="")
//     {
//         $("#down3").val(100);
//     }
//     if($("#right3").val()=="")
//     {
//         $("#right3").val(100);
//     }
//     ID("cropImageWidth3").value = parseInt(ID("zxxCropBox3").style.width);
//     ID("cropImageHeight3").value = parseInt(ID("zxxCropBox3").style.height);

//     var startDrag3 = function (point, target, kind) {
//         //point是拉伸点，target是被拉伸的目标，其高度及位置会发生改变
//         //此处的target与上面拖拽的target是同一目标，故其params.left,params.top可以共用，也必须共用
//         //初始化宽高
//         params3.width = getCss3(target, "width");
//         params3.height = getCss3(target, "height");
//         //初始化坐标
//         if (getCss3(target, "left") !== "auto") {
//             params3.left = getCss3(target, "left");
//         }
//         if (getCss3(target, "top") !== "auto") {
//             params3.top = getCss3(target, "top");
//         }
//         //target是移动对象
//         point.onmousedown = function (event) 
//         {
//             console.log('---->>>> dsflkjd down');
//             params3.kind = kind;
//             params3.flag = true;
//             clickFlag = true;
//             if (!event) {
//                 event = window.event;
//             }
//             var e = event;
//             params3.currentX = e.clientX;  //鼠标按下时坐标x轴
//             params3.currentY = e.clientY;  //鼠标按下时坐标y轴
//             /*console.log('params.currentX=', params.currentX);
//             console.log('params.currentY=', params.currentY);*/
//             //防止IE文字选中，有助于拖拽平滑
//             point.onselectstart = function () {
//                 return false;
//             };

//             document.onmousemove = function (event) 
//             {
//                 console.log('mouse move --> 2');
//                 let e = event ? event : window.event;
//                 clickFlag = false;
//                 if (params3.flag) {
//                     var nowX = e.clientX; // 鼠标移动时x坐标
//                     var nowY = e.clientY;   // 鼠标移动时y坐标
//                     var disX = nowX - params3.currentX;  // 鼠标x方向移动距离
//                     var disY = nowY - params3.currentY;  // 鼠标y方向移动距离
//                     if (params3.kind === "n") {
//                         //上拉伸
//                         //高度增加或减小，位置上下移动
//                         if (parseInt(params3.top) + disY > 0) {
//                             target.style.top = parseInt(params3.top) + disY + "px";
//                             target.style.height = parseInt(params3.height) - disY + "px";
//                         }
//                     } else if (params3.kind === "w") { //左拉伸
//                         if (parseInt(params3.left) + disX > 0) {
//                             target.style.left = parseInt(params3.left) + disX + "px";
//                             target.style.width = parseInt(params3.width) - disX + "px";
//                         }
//                     } else if (params3.kind === "e") { //右拉伸
//                         if (parseInt(params3.width) + disX + parseInt(params3.left) < bgwidth3)
//                            target.style.width = parseInt(params3.width) + disX + "px";
//                     } else if (params3.kind === "s") { //下拉伸
//                         if (parseInt(params3.height) + disY + parseInt(params3.top) < bgheight3) 
//                            target.style.height = parseInt(params3.height) + disY + "px";
//                     } else if (params3.kind === "nw") { //左上拉伸
//                         if (parseInt(params3.left) + disX > 0 && parseInt(params3.top) + disY > 0) {
//                             target.style.left = parseInt(params3.left) + disX + "px";
//                             target.style.width = parseInt(params3.width) - disX + "px";
//                             target.style.top = parseInt(params3.top) + disY + "px";
//                             target.style.height = parseInt(params3.height) - disY + "px";
//                         }
//                     } else if (params3.kind === "ne") { //右上拉伸
//                         if (parseInt(params3.width) + disX + parseInt(params3.left) < bgwidth3 && parseInt(params3.top) + disY > 0) {
//                             target.style.top = parseInt(params3.top) + disY + "px";
//                             target.style.height = parseInt(params3.height) - disY + "px";
//                             target.style.width = parseInt(params3.width) + disX + "px";
//                         }
//                     } else if (params3.kind === "sw") { //左下拉伸
//                         if (parseInt(params3.left) + disX > 0 && parseInt(params3.height) + disY + parseInt(params3.top) < bgheight3) {
//                             target.style.left = parseInt(params3.left) + disX + "px";
//                             target.style.width = parseInt(params3.width) - disX + "px";
//                             target.style.height = parseInt(params3.height) + disY + "px";
//                         }
//                     } else if (params3.kind === "se") { //右下拉伸
//                         if (parseInt(params3.width) + disX + parseInt(params3.left) < bgwidth3 && parseInt(params3.height) + disY + parseInt(params3.top) < bgheight3) {
//                             target.style.width = parseInt(params3.width) + disX + "px";
//                             target.style.height = parseInt(params3.height) + disY + "px";
//                         }
//                     } else { //移动
//                         if (parseInt(params3.left) + disX < 0) { target.style.left = 0 + "px"; }//最左侧为0,小于0阻止移动
//                         //最右侧，417-截图框宽度为left最大距离，超过阻止移动
//                         else if (parseInt(params3.left) + disX + parseInt(ID("zxxCropBox3").style.width) > bgwidth3) { target.style.left = bgwidth3 - parseInt(ID("zxxCropBox3").style.width) + "px"; }
//                         else target.style.left = parseInt(params3.left) + disX + "px";//正常范围
//                         if (parseInt(params3.top) + disY < 0) { target.style.top = 0 }//高度小于0,阻止向上移动
//                         else if (parseInt(params3.top) + disY + parseInt(ID("zxxCropBox3").style.height) > bgheight3)//移动高度+控件高度超过220，阻止移动
//                         { target.style.top = bgheight3 - parseInt(ID("zxxCropBox3").style.height) + "px"; }
//                         else { target.style.top = parseInt(params3.top) + disY + "px"; }//正常高度范围内
//                     }
//                 }

//                 document.onmouseup = function () {
//                     document.onmousemove = null;
//                     document.onmouseup = null;
//                     params2.flag = false;
//                     if (getCss3(target, "left") !== "auto") {
//                         params3.left = getCss3(target, "left");
//                     }
//                     if (getCss3(target, "top") !== "auto") {
//                         params3.top = getCss3(target, "top");
//                     }
//                     params3.width = getCss3(target, "width");
//                     params3.height = getCss3(target, "height");
//                     /*console.log('params.width=', params.width);
//                     console.log('params.height', params.width);*/

//                     //给隐藏文本框赋值
//                     posX = parseInt(target.style.left);
//                     posY = parseInt(target.style.top);
//                     cropW = parseInt(target.style.width);
//                     cropH = parseInt(target.style.height);
//                     if (posX < 0) {
//                         posX = 0;
//                     }
//                     if (posY < 0) {
//                         posY = 0;
//                     }
//                     if ((posX + cropW) > iCurWidth) {
//                         cropW = iCurWidth - posX;
//                     }
//                     if ((posY + cropH) > iCurHeight) {
//                         cropH = iCurHeight - posY;
//                     }
//                     //赋值
//                     //ID("cropPosX").value = posX;
//                     //ID("cropPosY").value = posY;
//                     //ID("cropImageWidth").value = parseInt(ID("zxxCropBox").style.width);
//                     //ID("cropImageHeight").value = parseInt(ID("zxxCropBox").style.height);
//                     var right = posX + parseInt(ID("zxxCropBox3").style.width);
//                     if (right < 0) right = 0;
//                     var down =  posY + parseInt(ID("zxxCropBox3").style.height);
//                     if (down < 0) down = 0;
//                     ID("left3").value = posX;
//                     ID("up3").value = posY;
//                     ID("right3").value = right;
//                     ID("down3").value = down;
//                     ID("cropImageWidth3").value = parseInt(ID("zxxCropBox3").style.width);
//                     ID("cropImageHeight3").value = parseInt(ID("zxxCropBox3").style.height);
//                 };
//             }
//         };
//     };

//     //绑定拖拽
//     startDrag3(ID3("zxxDragBg3"), ID3("zxxCropBox3"), "drag");
//     //绑定拉伸
//     startDrag3(ID3("dragLeftTop3"), ID3("zxxCropBox3"), "nw");
//     startDrag3(ID3("dragLeftBot3"), ID3("zxxCropBox3"), "sw");
//     startDrag3(ID3("dragRightTop3"), ID3("zxxCropBox3"), "ne");
//     startDrag3(ID3("dragRightBot3"), ID3("zxxCropBox3"), "se");
//     startDrag3(ID3("dragTopCenter3"), ID3("zxxCropBox3"), "n");
//     startDrag3(ID3("dragBotCenter3"), ID3("zxxCropBox3"), "s");
//     startDrag3(ID3("dragRightCenter3"), ID3("zxxCropBox3"), "e");
//     startDrag3(ID3("dragLeftCenter3"), ID3("zxxCropBox3"), "w");


//     //图片不能被选中，目的在于使拖拽顺滑
//     ID3("myCanvas3").onselectstart = function () 
//     {
//         console.log('onselectstart --> ');
//         return false;
//     };
//     // img2.onselectstart = function () 
//     // {
//     //     console.log('onselectstart --->>>');
//     //     return false;
//     // };
// }

// //确认裁剪
// function RequeryCrop3() {
//     var x = document.getElementById("left3").value;
//     var y = document.getElementById("up3").value;
//     var w = document.getElementById("cropImageWidth3").value;
//     var h = document.getElementById("cropImageHeight3").value;
//     console.log('cropImage(img,', x, ',', y, ',', parseInt(w), ',', parseInt(h), ')');
//    // cropImage2(img2, x / scaleX2, y / scaleY2, parseInt(w) / scaleX2, parseInt(h) / scaleY2);
// }


// function cropImage3(img, cropPosX, cropPosY, width, height) {
//     /*var cropContainer = ID("cropContainer");
//     cropContainer.parentNode.removeChild(cropContainer);*/
//     /*ctx.clearRect(0, 0, myCanvas.width, myCanvas.height);*/
//     //sx,sy 是相对于图片的坐标。巨坑

//     $("#imgDiv3").html('');
//     var myCanvas = document.createElement('canvas');
//     myCanvas.setAttribute('id', 'myCanvas3');
//     myCanvas.style.display = 'block';
//     /*myCanvas.style.position = 'absolute';*/
//     myCanvas.width = width * scaleX3;
//     myCanvas.height = height * scaleY3;
//     myCanvas.style.border = "1px solid #d3d3d3";
//     myCanvas.innerText = '您的浏览器不支持 HTML5 canvas 标签。';
//     myCanvas.style.zIndex = 'auto';

//     var newCtx = myCanvas.getContext('2d');
//     newCtx.drawImage(img3, cropPosX, cropPosY, width, height, 0, 0, width * scaleX3, height * scaleY3);


//     //var newCanvas = document.createElement('canvas');
//     //newCanvas.setAttribute('id', 'newCanvas');
//     //newCanvas.width = width * scaleX;
//     //newCanvas.height = height * scaleY;
//     //newCanvas.style.border = "1px solid #d3d3d3";
//     //var newCtx = newCanvas.getContext('2d');
//     //clipImgDiv.appendChild(newCanvas);
//     //newCtx.drawImage(img, cropPosX, cropPosY, width, height, 0, 0, width * scaleX, height * scaleY);

//     // canvas转化为图片
//     var newImage = new Image();
//     newImage.src = myCanvas.toDataURL("image/png");
//     //newImage.style.marginLeft = '5px';
//     newImage.width = bgwidth3;
//     newImage.height = bgheight3;
//     //clipImgDiv.appendChild(newImage);
//     imgDiv3.appendChild(newImage);
//     oRelDiv3.innerHTML = '';
// }
