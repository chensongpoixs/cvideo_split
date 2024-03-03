layui.use(['form', 'element'], function () {
    var $ = layui.jquery
        , element = layui.element
        , form = layui.form;

    form.on('select(style)', function (data) {
        $("#pjStyle").val(data.value);
        loadJS(parseInt(data.value));
        // if (parseInt(data.value) - num > 0) {
        //     names=[];
        //     for (let index = num+1; index < parseInt(data.value); index++) {
        //         var name=$("#videoName" + index).val();
        //         if(name!='undefined' && name!=null && name!='')
        //           names.push($("#videoName" + index).val());
        //         else
        //           continue;
        //     }
        //     //GetVideoImgs(names,false,-1,num+1);
        // }
        num = parseInt(data.value);
    });
    form.on('select(videoName1)', function (data) { var names = [data.value]; GetVideoImgs(names, false, 1); });
    form.on('select(videoName2)', function (data) { var names = [data.value]; GetVideoImgs(names, false, 2); });
    form.on('select(videoName3)', function (data) { var names = [data.value]; GetVideoImgs(names, false, 3); });
    form.on('select(videoName4)', function (data) { var names = [data.value]; GetVideoImgs(names, false, 4); });
    form.on('select(videoName5)', function (data) { var names = [data.value]; GetVideoImgs(names, false, 5); });
    form.on('select(videoName6)', function (data) { var names = [data.value]; GetVideoImgs(names, false, 6); });
    form.on('select(videoName7)', function (data) { var names = [data.value]; GetVideoImgs(names, false, 7); });
    form.on('select(videoName8)', function (data) { var names = [data.value]; GetVideoImgs(names, false, 8); });
    form.on('select(videoName9)', function (data) { var names = [data.value]; GetVideoImgs(names, false, 9); });
    form.on('select(videoName10)', function (data) { var names = [data.value]; GetVideoImgs(names, false, 10); });
    form.on('select(videoName11)', function (data) { var names = [data.value]; GetVideoImgs(names, false, 11); });
    form.on('select(videoName12)', function (data) { var names = [data.value]; GetVideoImgs(names, false, 12); });
    form.on('select(islock)', function (data) { $("#pjislock").val(data.value); });
});

$(function () {
    var port = getCookie('zbport');
    if (ValidateStringOR(port)) {
        port = 9000;
        setCookie('zbport', 9000);
    }
    else
        port = getCookie('zbport');
    $("#playPort").val(port);
    $("#TID").val(random());
    sendGetTileAjax();//获取所有通道信息  /******************************************************************************/
    InitData();//为下拉列表绑定摄像头资源
    loadJS(num);//根据拼接方式初始化右侧拼接图 num=2/3/4

    for (var i = 4; i > num; i--) {
        $("#up" + i).val('').attr("disabled", "disabled");
        $("#down" + i).val('').attr("disabled", "disabled");
        $("#left" + i).val('').attr("disabled", "disabled");
        $("#right" + i).val('').attr("disabled", "disabled")
    }
});
//获取摄像头图片
window.onload = function () {
    //GetVideoStream($("#videoName1").val(), 1);
    //GetVideoStream($("#videoName2").val(), 2);
  this.names.push($("#videoName1").val());
  this.names.push($("#videoName2").val());

    GetVideoImgs(names,false,-1,1);

}
//初始化相机数据，绑定下拉列表
function InitData() 
{
    var message = '{"msg":"get_camera_info"}';
    console.log('get camera info ...');
        ajaxGet(camera_url, 
            function(result)
            {
                console.log('camera ===> '+result);
                var objs = JSON.parse(result);
                LoadVideo(objs.camera_infos, 12);
                layui.use(['form'], function () {
                    var form = layui.form;
                    form.render();
                });
            },
            function(result)
            {
                console.log('get camera_url '+ camera_url +' failed !!! = ' + result);
            }
        );
    // $.ajax({
    //     type: "POST",
    //     dataType: "text",
    //     //url: '/json',
    //     url: apiAddress,
    //     contentType: "application/json",
    //     data: message,
    //    // async: false,
    //     success: function (result) {
    //         var objs = JSON.parse(result);
    //         LoadVideo(objs.video_souce_info, 12);//12个下拉列表都绑定，只有前2个启用
    //         layui.use(['form'], function () {
    //             var form = layui.form;
    //             form.render();
    //         });
    //     }
    // });

}

//绑定相机名称下拉列表 data:数据集合，num：2/3/4
function LoadVideo(data, num) 
{
     cameras = data;
    var parent = $(".layui-row");
    if (num > 0 &&  ValidateStringAnd(data)) 
    {
        for (var i = 1; i <= num; i++) 
        {
            var obj = $("#videoName" + i);
            obj.html('');
            var str = "";
            var inputHidden = "";
            var count = 0;//记录$.each剔除已使用的相机后循环次数
           str += "<option value='-1'>请选择</option>";

            $.each(data, function (index, item) 
            {
                if (ValidateStringAnd(item.camera_name)) 
                {
                    var camera = _.find(cameras, function (obj) { return obj.camera_id == item.camera_id });
                    // if (camera != null) //表示已经使用，不再添加
                    // {
                    //     return true;
                    // }
                    //if (count == (i - 1))
                    if (count == (i - 1) &&(i==1 || i==2))
                    {
                        str += "<option value='" + item.camera_id + "' selected>" + item.camera_name + "</option>";
                    }
                     else
                    {
                        str += "<option value='" + item.camera_id + "'>" + item.camera_name + "</option>"; 
                    }
                    count++;
                }
                if (i == 1) //只给第一个绑定，
                {
                    inputHidden += "<input type='hidden' id='name_" + item.camera_id + "' value='" + item.name + "' />";
                    inputHidden += "<input type='hidden' id='ip_" + item.camera_id + "' value='" + item.ip + "' />";
                    inputHidden += "<input type='hidden' id='address_" + item.camera_id + "' value='" + item.address + "' />";
                    inputHidden += "<input type='hidden' id='port_" + item.camera_id + "' value='" + item.port + "' />";
                    inputHidden += "<input type='hidden' id='state_" + item.camera_id + "' value='" + item.state + "' />";
                }
            });

            $(str).appendTo(obj);
            $(inputHidden).appendTo(parent);
        }
        // console.log('LoadVideo ==>'+ str);
    }
}

//确定事件/***/@@@@@@@@@@@@@@@@@@
function RequeryCropArray() 
{
    console.log('RequeryCropArray');
    var messageAll = '{"msg": "start_tile","video_souce_info": [';
    var style = $("#style").val();
    console.log('style = ' + style);
    //iscrop = true;
    let mess = videoMsg(parseInt(style)); //拼接发送json
    if (!mess) 
    {
        layer.msg('无有效相机进行拼接！');
        return false;
    }
     send_video_split(mess);
    // messageAll += mess;
    // console.log(messageAll);
    // sendAjax(messageAll, "set"); //发送拼接，获取拼接视频流
    // let TID = $("#TID").val();
    // var gettileinfo = '{"msg": "get_tile_info"}';
    // sendAjax(gettileinfo, "get");
}


    function ajaxGet(url, successCallback, errorCallback) 
    {
        console.log("http get = " + url);
      const xhr = new XMLHttpRequest();
      xhr.open('GET', url);
      xhr.onreadystatechange = function() {
        if (xhr.readyState === XMLHttpRequest.DONE) {
          if (xhr.status === 200) {
            successCallback(xhr.responseText);
          } else {
            errorCallback(xhr.statusText);
          }
        }
      };
      xhr.send();
    }


//根据选择方式进行canvas处理
function loadJS(num) {
    if (num - prevnum > 0)//追加js
    {
        for (let index =prevnum+1; index <=num; index++) {
            var head = $("head");
            var script = $("<script>");
            $(script).attr('type', 'text/javascript');
            $(script).attr('src', 'js/Canvas'+index+'.js');
            $(head).append(script);
            InputDisabled(index);
        } 
    } else if (num - prevnum < 0)//移除js
    {
        for (let index = num+1; index <=prevnum; index++) {
            $("script[src='js/Canvas"+index+".js']").remove();
            $("#imgDiv"+index).html('');
        }
    }
    
    // }
    //根据选择方式，禁用input
    for (var i = 12; i > parseInt(num); i--) {
        $("#up" + i).val('').attr("disabled", "disabled");
        $("#down" + i).val('').attr("disabled", "disabled");
        $("#left" + i).val('').attr("disabled", "disabled");
        $("#right" + i).val('').attr("disabled", "disabled")
        $("#videoName" + i).attr("disabled", "disabled")
        layui.use(['form'], function () {
            var form = layui.form;
            form.render();
        });
    }
    prevnum=num;
}

//切换摄像机，加载图片
function ChangeVideoNmae(img, num) {
    $("#imgDiv" + num).html('');
    if (num == 1) 
    {
          image1 = img; 
          ExecuteCanvas1(cameras[num].url);  
    }
    else if (num == 2) 
    { 
        image2 = img; 
        ExecuteCanvas2(cameras[num].url); 
    }
    else if (num == 3) { image3 = img; ExecuteCanvas3(); }
    else if (num == 4) { image4 =  img; ExecuteCanvas4(); }
    else if (num == 5) { image5 = img; ExecuteCanvas5(); }
    else if (num == 6) { image6 = img; ExecuteCanvas6(); }
    else if (num == 7) { image7 = img; ExecuteCanvas7(); }
    else if (num == 8) { image8 =  img; ExecuteCanvas8(); }
    else if (num == 9) { image9 = img; ExecuteCanvas9(); }
    else if (num == 10) { image10 =  img; ExecuteCanvas10(); }
    else if (num == 11) { image11 = img; ExecuteCanvas11(); }
    else if (num == 12) { image12 =  img; ExecuteCanvas12(); }
}

//切换摄像机，加载图片
function ChangeVideoNmae(camera_id, img, num) 
{
    console.log('ChangeVideoNmae  num =' + camera_id);
    console.log('cameras = ' +JSON.stringify(cameras));
    $("#imgDiv" + num).html('');
    if (num == 1) 
    {
          image1 = img; 
          ExecuteCanvas1(cameras[camera_id].url);  
    }
    else if (num == 2) 
    { 
        image2 = img; 
        ExecuteCanvas2(cameras[camera_id].url); 
    }
    else if (num == 3) { image3 = img; ExecuteCanvas3(); }
    else if (num == 4) { image4 =  img; ExecuteCanvas4(); }
    else if (num == 5) { image5 = img; ExecuteCanvas5(); }
    else if (num == 6) { image6 = img; ExecuteCanvas6(); }
    else if (num == 7) { image7 = img; ExecuteCanvas7(); }
    else if (num == 8) { image8 =  img; ExecuteCanvas8(); }
    else if (num == 9) { image9 = img; ExecuteCanvas9(); }
    else if (num == 10) { image10 =  img; ExecuteCanvas10(); }
    else if (num == 11) { image11 = img; ExecuteCanvas11(); }
    else if (num == 12) { image12 =  img; ExecuteCanvas12(); }
}


//启用4个方位input
function InputDisabled(num) {
    $("#up" + num).attr("disabled", false);
    $("#down" + num).attr("disabled", false);
    $("#left" + num).attr("disabled", false);
    $("#right" + num).attr("disabled", false);
    $("#videoName" + num).attr("disabled", false)
}

//根据方位input输入，移动截取框
function ChangeUp1(e, xh) {
    var value = parseInt(e.value);
    if(value<0)
     return false;
    var height = ID("zxxCropBox" + xh).clientHeight;
    if (value + height <= bgheight) {
        $("#zxxCropBox" + xh).css("top", parseInt(e.value));
        $("#down" + xh).val(parseInt(e.value) + height);
    }
}
function ChangeDown1(e, xh) {
    var value = parseInt(e.value);
    if(value<0)
    return false;
    var height = ID("zxxCropBox" + xh).clientHeight;
    if (value <= bgheight && e.value >= height) {
        var top = parseInt(e.value) - height;
        $("#zxxCropBox" + xh).css("top", top);
        $("#up" + xh).val(top);
    }
}
function ChangeLeft1(e, xh) {
    var value = parseInt(e.value);
    if(value<0)
    return false;
    var width = ID("zxxCropBox" + xh).clientWidth;
    if (value + width <= bgwidth) {
        var left = value + width;
        $("#zxxCropBox" + xh).css("left", parseInt(e.value));
        $("#right" + xh).val(left);
    }
}
function ChangeRight1(e, xh) {
    var value = parseInt(e.value);
    var reg=/^[0-9]*$/;

    return false;
    var width = ID("zxxCropBox" + xh).clientWidth;
    if (value <= bgwidth && value > width) {
        var left = bgwidth - width;
        $("#zxxCropBox" + xh).css("left", left);
        $("#left" + xh).val(left);
    }
}
//先获取视频流num

var names=[];
function GetVideoImgs(names,init, num,i) 
{
    var style=parseInt($("#style").val());
    if(names.length<=0)
    {
        return false;
    }
    console.log('names = ' + names + ', num = ' + num);
    ChangeVideoNmae(names, null, num);
    return true;
    for (let index = 1; index <= names.length; index++) {
           var message = '{"msg": "set_camera_info","video_souce_info": [';
            message += '{';
             if(ValidateStringOR(names[index-1]))
              continue;

             message += '"camera_id":"' + names[index-1] + '",';
            // message+='"urlorfile":"/root/1.264",';
            //message+='"urlorfile":"'+modifyStyle+'",';
        
            var cam_ip=$("#ip_"+names[index-1]).val();
            // if(ValidateStringOR(cam_ip))
            // {
            //     //message += '"urlorfile":"udp://' + $("#address_"+names[index-1]).val() + ':' +$("#port_"+names[index-1]).val() + '",';
            //     message += '"urlorfile":"rtsp://admin:zhuoshi123@' + $("#address_"+names[index-1]).val() +'",';
            // }
            // else
            // {
            //     message += '"urlorfile":"rtsp://admin:admin12345@' + $("#ip_"+names[index-1]).val() +'",';
            // }
            message += '"urlorfile":"udp://' + $("#address_"+names[index-1]).val() + ':' +$("#port_"+names[index-1]).val() + '",';
            message += '"name":"' + $("#name_"+names[index-1]).val() + '",';
            message += '"ip":"' + $("#ip_"+names[index-1]).val() + '",';
            message += '"address":"' + $("#address_"+names[index-1]).val() + '",';
            message += '"port":"' + $("#port_"+names[index-1]).val() + '",';
            message += '"state":"' + $("#state_"+names[index-1]).val() + '"';
            message += '}';    
           message += "]}";
           console.log(message);
       $.ajax({
        type: "POST",
        dataType: "text",
        //url: '/json',
        url: apiAddress,
        contentType: "application/json",
        data: message,
        timeout: 7000,
        //  async:false,
        success: function (result) {
            if (result != "") {
                console.log(result);
                var objs = JSON.parse(result);
                   if(index==names.length){
                    var send_str = '{"msg": "cap_pic","camera_id":' + JSON.stringify(names) + '}';
                    console.log(send_str);
                    GetCameraImage(send_str,init,num,i);
                   } 
            }
        }
    });
}
}

function GetCameraImage(message,init,num,i)
{
   $.ajax({
            type: "POST",
            dataType: "text",
            //url: '/json',
            url: apiAddress,
            contentType: "application/json",
            data: message,
            timeout: 5000,
            async: false,
            success: function (result) {
                if (result != "") {
                    console.log(result);
                    var objs = JSON.parse(result);
                    if (objs.pics.length > 0) {
                        for (let index = 0; index < objs.pics.length; index++) {
                            if(num<0)
                            {
                                var number=index+1;
                                if(objs.pics[index].status!="failed")
                                {
                                    ChangeVideoNmae(videoAddress+objs.pics[index].pic_url,index+i); 
                                    $("#container"+number).attr("data-state",true);
                                }else
                                {
                                    $("#container"+number).attr("data-state",false);
                                }
                            }
                              else
                              {
                                if(objs.pics[index].status!="failed")
                                {
                                 ChangeVideoNmae(videoAddress+objs.pics[0].pic_url,num); 
                                 $("#container"+num).attr("data-state",true);
                                }else
                                {
                                    $("#container"+num).attr("data-state",false);
                                    ChangeVideoNmae('../images/bg.png',num); 
                                    
                                }
                              }
                        }
                        if(init) {RequeryCropArray();}
                    }
                }
            }
    });
}

//拼接发送信息  playCanvas
let videoMsg = function (num)
{
    let Tname = $("#Tname").val();
    let TID = $("#TID").val();
    let address = $("#playAdd").val();
    let islock = $("#islock").val();
    let port = $("#playPort").val();
    var style = $("#style").val();
    var isfold=$("#isfold").val();
    let messgae = "";
    if (ValidateStringOR(Tname)) 
    { 
        layer.msg('请输入拼接通道名称'); 
        console.log(' not find split_channel_id == null');
        return false; 
    }
    if (ValidateStringOR(TID)) 
    { 
        layer.msg('请输入通道ID'); 
        return false; 
    }
    if (ValidateStringOR(address)) 
    { 
        layer.msg('请输入组播地址'); 
        return false; 
    }
    if (ValidateStringOR(port)) 
    { 
        layer.msg('请输入组播端口'); 
        return false; 
    }
    let camera_infos_group = [];
    if (num > 0) 
    {
        for (var i = 1; i <= num; i++) {
            var state=$("#container"+i).attr("data-state");
            if(state=="false")
            {
                // continue;
            }    

            var msg = "{";
            let videoName = $("#videoName" + i).val();
            // if(videoName=='-1' || typeof(videoName)=='undefined')
               // continue;
            let up = $("#up" + i).val();
            let down = $("#down" + i).val();
            let left = $("#left" + i).val();
            let right = $("#right" + i).val();
            //转换0-1坐标
            up = parseInt(up) / bgheight;
            down = parseInt(down) / bgheight;
            left = parseInt(left) / bgwidth;
            right = parseInt(right) / bgwidth;
            let cropImageWidth = $("#cropImageWidth" + i).val();
            let cropImageHeight = $("#cropImageHeight" + i).val();
            // msg += '"camera_id": "' + videoName + '",';
            //msg+='"urlorfile":"'+addStyle+'",';
            //msg+='"urlorfile":"/root/1.264",';
            // var cam_ip = $("#ip_" + videoName).val();
            // // if (ValidateStringOR(cam_ip)) {
            // //    // msg += '"urlorfile":"udp://' + $("#address_" + videoName).val() + ':' + $("#port_" + videoName).val() + '",';
            // //    msg += '"urlorfile":"rtsp://admin:zhuoshi123@' + $("#address_" + videoName).val() + '",';
            // // } else {
            // //     msg += '"urlorfile":"rtsp://admin:admin12345@' + $("#ip_" + videoName).val() + '",';
            // // }
            // msg += '"urlorfile":"udp://' + $("#address_" + videoName).val() + ':' + $("#port_" + videoName).val() + '",';
            // msg += '"vendor": 1,';
            // msg += '"crop": { "l":' + left + ',"t":' + up + ',"r":' + right + ',"b":' + down + '},';
            // msg += '"tile_video_id":"' + TID + '",';
            // msg += '"idx_in_tile":' + (i - 1) + '},';
            // messgae += msg;

            cropImageWidth = parseInt(cropImageWidth)/ bgwidth;
            cropImageHeight = parseInt(cropImageHeight)/ bgheight;
            //tileArray[]
            
            camera_infos_group.push({   camera_id: parseInt(videoName),
                                        index: parseInt((i-1)),
                                        x: left,
                                        y: up,
                                        w: cropImageWidth ,
                                        h: cropImageHeight});
        }
      
        // messageAll += messgae;
    }
        let split_video_message = {
                "split_channel_name": Tname,
                "split_channel_id": TID,
                "multicast_ip": address,
                "multicast_port": parseInt(port), 
                "split_method": 0 ,
                "lock_1080p": parseInt(islock) ,
                "overlay": 0,  
                "camera_group" : camera_infos_group

        };

        console.log('split_video_message = ' + JSON.stringify(split_video_message));

    return  JSON.stringify(split_video_message); 
    // if(messgae=="")
    // {
    //     console.log('messgae num = ' + num);
    //     return messgae;
    // }
    // messgae = messgae.substring(0, messgae.length - 1) + "],";
    // messgae += '"video_tile": [{';
    // messgae += '"ip": "' + TID + '",';
    // messgae += '"name": "' + Tname + '",';
    // messgae += '"address": "' + address + '",';
    // messgae += '"port": "' + port + '",';
    // messgae += '"style": ' + style + ',';
    // messgae += '"state": 1,';
    // messgae += '"islock":' + parseInt(islock) + ',';
    // messgae += '"id": "' + TID + '",';
    // messgae += '"fix_1080p":' + parseInt(islock) + ',';
    // messgae += '"is_fold": ' + isfold + ',';
    // messgae += '"multicast_ip":"' + address + '",';
    // messgae += '"multicast_port":' + port + ' }]';
    // messgae += "}";
    // console.log('messgae = ' + messgae);
    // return messgae;
}
function send_video_split(msg)
{
    restart_video_split(1);
    $.ajax({
        type: "POST",
        dataType: "text",
        //url: '/json',
        url: modify_video_url,
        contentType: "application/json",
        data: msg,
        async:false,
        timeout: 5 * 1000,
        success: function (result) 
        {
            console.log(result); 
            var objs= JSON.parse(result);
            if (objs.result == 0)
            {
                // 设置高度和宽度
                $("#pj_width").val(objs.data.out_video_width);
                $("#pj_height").val(objs.data.out_video_height);
                restart_video_split(0);
               
            }
                
        }
    }); 
}
function  restart_video_split(code)
{
     let TID = $("#TID").val();
    var   http_url = cmd_split_video_url + 'split_channel_id=' + TID + '&cmd=';

    ajaxGet(http_url + code, function(result){
        if (code == 0)
        {
             playVideo();
        }
    }, function(result){});
     
}

//发送ajax，获取拼接流地址信息，拼接视频播放
function sendAjax(send_str, action) {
    $.ajax({
        type: "POST",
        dataType: "text",
        //url: '/json',
        url: apiAddress,
        contentType: "application/json",
        data: send_str,
        async: false,
        timeout: 5 * 1000,
        success: function (result) {
            console.log(result);
            if (action === 'get') {
                var objs = JSON.parse(result);
                if (objs.video_tile.length > 0) {
                    for (let index = 0; index < objs.video_tile.length; index++) {
                        if (objs.video_tile[index].id === $("#TID").val()) {
                            $("#pj_width").val(objs.video_tile[index].w);
                            $("#pj_height").val(objs.video_tile[index].h);
                            setCookie('zbport', parseInt($("#playPort").val()) + 1);
                            playVideo();//拼接视频播放
                        }
                    }
                }

            } else if (action === 'set') {
                // console.log(result);
            }
        }
    });
}

//播放视频
function playVideo() 
{
    var domain = document.domain;
    let address = $("#playAdd").val();
         
    let port = $("#playPort").val();
    if ( ValidateStringOR(address))  { layer.msg('请输入组播地址'); return false; }
    if ( ValidateStringOR(port)) { layer.msg('请输入组播端口'); return false; }
    //var domain = "likp.top";
    var canvas = document.getElementById("playCanvas");
    if (!canvas) {
        return false;
    }
    if (!document.player)
     {
        document.player = new Player();
    }
    //var playAdd = "tstream://"+id;
    // var playAdd = "tstream://" + $("#TID").val();
    // document.player.play(domain, playAdd, canvas);
     var playAdd = "udp://@" + address + ":" + port;
    console.log('split video url = ' + playAdd);
    document.player.play(domain, playAdd, canvas);
    return true;
}

//停止播放
function stopVideo()
{
    var canvas = document.getElementById("playCanvas");
    if(document.player!=null)
    {
        document.player.stop();
    }
}
//点击取消事件，摄像头1，2重置， 注意：num为全局变量
function BtnCancel() {
    var index = parent.layer.getFrameIndex(window.name); 
    parent.layer.close(index);
    // isThree = false;
    // //主要取消1，2的效果
    // for (var i = 1; i <= 2; i++) {
    //     var head = $("head");
    //     var script = $("<script>");
    //     $("script[src='js/Canvas" + i + ".js']").remove();
    //     $("#imgDiv" + i + "").html('');
    //     $(script).attr('type', 'text/javascript');
    //     $(script).attr('src', 'js/Canvas' + i + '.js');
    //     $(head).append(script);
    // }
    // loadJS(num);//注销3，4效果
}

function sendGetTileAjax() {
    cameras = [];
    var send_str = '{"msg": "get_tile_info"}';
    // $.ajax({
    //     type: "POST",
    //     dataType: "text",
    //     //url: '/json',
    //     url: apiAddress,
    //     contentType: "application/json",
    //     data: send_str,
    //     timeout: 5000,
    //     //async: false,
    //     success: function (result) {
    //         console.log(result);
    //         var objs = JSON.parse(result);
    //         if (objs != null && typeof (objs) != 'undefined') {
    //             cameras = objs.video_souce_info;
    //         }
    //     }
    // });
    return;
    ajaxGet(all_video_split_url, 
            function(result)
            {
                console.log('video split info all  ===> '+result);
                var objs = JSON.parse(result);
                //LoadVideo(objs.camera_infos, 12);
                //layui.use(['form'], function () {
                //  var form = layui.form;
                //  form.render();
                //});
                 if(objs!=null&&typeof(objs)!='undefined')
                 {
                    // console.log(JSON.);
                     cameras=objs.video_split_infos;
                 }
            },
            function(result)
            {
                console.log('get camera_url '+ camera_url +' failed !!! = ' + result);
            }
        );
}

var isFirst = true;
function BtnOkClick() 
{
    console.log('BtnOkClick');
    var flag = JudgeCamID();
    if (!flag) 
    { 
        layer.msg("不能有重复的摄像机！"); 
        return false; 
    }
    var isfold=$("#isfold").val();
    var height= isfold==="1"?290:260;
    $("#playCanvas").css("height",height+"px");
    if (!isFirst) {
        var messageAll = '{"msg": "start_tile","video_souce_info": [';
        var style = $("#style").val();
        //iscrop = true;
        console.log('style = ' + style);
        let mess = videoMsg(parseInt(style)); //拼接发送json
        if (!mess) 
        {
            layer.msg('无有效相机进行拼接！');
            return false;
        };
        send_video_split(mess);
        // messageAll += mess;
        // console.log(messageAll);
        // sendAjax(messageAll, "set"); //发送拼接，获取拼接视频流
        // stopVideo();
        // let TID = $("#TID").val();
        // var gettileinfo = '{"msg": "get_tile_info"}';
        // console.log(gettileinfo);
        // sendAjax(gettileinfo, "get");
    } else {
        RequeryCropArray();
        isFirst = false;
    }
}
//判断拼接相机ID是否重复
function JudgeCamID() {
    var style = $("#style").val();
    if (parseInt(style) > 0) {
        var cam_id = "";
        for (let index = 1; index <= parseInt(style); index++) {
            var cam = $("#videoName" + index).val();
            if (cam_id.indexOf(cam) >= 0) {
                return false;
            }
            else if (cam == '-1') { }
            else
                cam_id += $("#videoName" + index).val() + ",";
        }
        return true;
    }
}

var random = function getDtId() {
    var d = new Date();
    var year = d.getFullYear();
    var month = d.getMonth() + 1;
    var date = d.getDate();
    var day = d.getDay();
    var hours = d.getHours();
    var minutes = d.getMinutes();
    var seconds = d.getSeconds();
    var ms = d.getMilliseconds();
    year = (year + "").substring(2);
    if (month <= 9)
        month = "0" + month;
    if (date <= 9)
        date = "0" + date;
    if (hours <= 9)
        hours = "0" + hours;
    if (minutes <= 9)
        minutes = "0" + minutes;
    if (seconds <= 9)
        seconds = "0" + seconds;
    let num = Math.ceil(Math.random() * 100);
    var id = year + month + date + hours + minutes + seconds + num;
    return id;
}

function setCookie(name, value) {
    var Days = 30;
    var exp = new Date();
    exp.setTime(exp.getTime() + Days * 24 * 60 * 60 * 1000);
    document.cookie = name + "=" + escape(value) + ";expires=" + exp.toGMTString();
}

function getCookie(name) {
    var arr, reg = new RegExp("(^| )" + name + "=([^;]*)(;|$)");
    if (arr = document.cookie.match(reg)) return unescape(arr[2]);
    else return null;
}

function ModifyOSDsTYLE() {
    $("#osd").css("display", "block");
    moveOSD();
}

var moveOSD = function () {
    //获取元素
    var dv = document.getElementById('osd');
    var x = 0;
    var y = 0;
    var l = 0;
    var t = 0;
    //鼠标按下事件
    dv.onmousedown = function (event) {
        let e = event ? event : window.event;
        //获取x坐标和y坐标
        var x = e.clientX; // 鼠标移动时x坐标
        var y = e.clientY;   // 鼠标移动时y坐标

        //获取左部和顶部的偏移量
        l = dv.offsetLeft;
        t = dv.offsetTop;
        //开关打开
        isDown = true;
        //设置样式  
        dv.style.cursor = 'move';
        //鼠标移动
        document.onmousemove = function (e) {
            if (isDown == false) {
                return;
            }

            var nx = e.clientX;
            var ny = e.clientY;
            var clientx = parseInt(ID("osd").clientWidth);
            var clientY = parseInt(ID("osd").clientHeight);
            //计算移动后的左偏移量和顶部的偏移量
            var nl = nx - (x - l);
            var nt = ny - (y - t);
            if (nl < 0)
                nl = 0;
            else if (nl + clientx > 519)
                nl = 519 - clientx;
            if (nt < 0)
                nt = 0;
            else if (nt + clientY > 260)
                nt = 260 - clientY;
            dv.style.left = nl + 'px';
            dv.style.top = nt + 'px';
        }
        //鼠标抬起事件
        document.onmouseup = function () {
            // dv.onmousedown=null;
            // dv.onmousemove=null;
            //开关关闭
            isDown = false;
            dv.style.cursor = 'default';
        }
    }

}
    function send_osd(msg)
    {
            restart_video_split(1);
            $.ajax({
            type: "POST",
            dataType: "text",
            //url: '/json',
            url: modify_video_osd_url,
            contentType: "application/json",
            data: msg,
            async:false,
            timeout: 5 * 1000,
                success: function (result) 
                {
                        console.log(result); 
                        var objs= JSON.parse(result);
                        if (objs.result == 0)
                        {
                            $("#pj_width").val(objs.out_video_width);
                            $("#pj_height").val(objs.out_video_height);
                            //console.log(objs);
                            restart_video_split(0);
                          //  playVideo();
                        }   
                }
            });
    }
function SendOSD() {
    var obj = document.getElementById("txtOSD");//获取文本框字体大小
    var top = parseInt(getCss(ID("osd"), "top"));
    var left = parseInt(getCss(ID("osd"), "left"));
    //实际top值为：top+input-fontsize   parseInt(obj.style.fontSize)
    var send_str = '{"msg":"set_tile_osd",';
    if ($("#pj_width").val() == "" || $("#pj_height").val() == "")
    {
        layer.msg('请先进行摄像机拼接'); return false;
    }
    var videoX = parseInt($("#pj_width").val());
    var videoY = parseInt($("#pj_height").val());

    var fontSize = Math.ceil(Math.round(parseInt(videoX) / 520, 2) * 20);
    var canvasHei= parseInt(getCss(ID("playCanvas"), "height")); 
    left = parseInt(left) / 520;
    top = parseInt(top) / canvasHei;
     let osd_msg = {
            split_channel_id : $("#TID").val(),
            txt : obj.innerText,
            fontsize: fontSize,
            out_video_width: videoX,
            out_video_height: videoY,
            x : left,
            y: top 
        };

    console.log(send_str);
    console.log(JSON.stringify(osd_msg));
    send_osd(JSON.stringify(osd_msg));

    // send_str += '"tile_id":"' + $("#TID").val() + '","txt":"' + obj.innerText + '","x":' + left + ',"y":' + top + ',"ft_size":' + fontSize + '}';
    // console.log(send_str);
    // sendAjax(send_str, "osd");
    // var gettileinfo = '{"msg": "get_tile_info"}';
    // console.log(gettileinfo);
    // sendAjax(gettileinfo, "get");
    $("#osd").css("display", "none");
} 

function ValidateStringOR(str)
{
    var flag=false;
    if(typeof (str) == 'undefined' || str == null || str == ''|| str=='null' || str=='undefined')
      flag= true;
     return flag;
}
function ValidateStringAnd(str)
{
    var flag=false;
    if(typeof (str) != 'undefined' && str != null && str != ''&& str!='null' && str!='undefined')
      flag= true;
     return flag;
}