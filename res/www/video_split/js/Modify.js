layui.use(['form', 'element'], function () {
    var $ = layui.jquery
        , element = layui.element
        , form = layui.form;

        form.on('select(style)', function (data) 
		{
			console.log('select (style --> )');
            $("#pjStyle").val(data.value);
            loadJS(parseInt(data.value));
            num = parseInt(data.value);
        });
        form.on('select(videoName1)', function (data) {var names=[data.value]; GetVideoImgs(names,false,1); });
        form.on('select(videoName2)', function (data) { var names=[data.value]; GetVideoImgs(names,false,2); });
        form.on('select(videoName3)', function (data) { var names=[data.value]; GetVideoImgs(names,false,3); });
        form.on('select(videoName4)', function (data) { var names=[data.value]; GetVideoImgs(names,false,4); });
        form.on('select(videoName5)', function (data) {var names=[data.value]; GetVideoImgs(names,false,5); });
        form.on('select(videoName6)', function (data) {var names=[data.value]; GetVideoImgs(names,false,6); });
        form.on('select(videoName7)', function (data) {var names=[data.value]; GetVideoImgs(names,false,7); });
        form.on('select(videoName8)', function (data) {var names=[data.value]; GetVideoImgs(names,false,8); });
        form.on('select(videoName9)', function (data) {var names=[data.value]; GetVideoImgs(names,false,9); });
        form.on('select(videoName10)', function (data) {var names=[data.value]; GetVideoImgs(names,false,10); });
        form.on('select(videoName11)', function (data) {var names=[data.value]; GetVideoImgs(names,false,11); });
        form.on('select(videoName12)', function (data) {var names=[data.value]; GetVideoImgs(names,false,12); });
        form.on('select(islock)', function (data) {$("#pjislock").val(data.value); });
    });
	
    $(function () {
        GetParams();//获取路径参数
        sendGetTileAjax();
        InitData();//下拉列表绑定摄像头名称
        for (var i = 12; i > num; i--) {
            $("#up" + i).val('').attr("disabled", "disabled");
            $("#down" + i).val('').attr("disabled", "disabled");
            $("#left" + i).val('').attr("disabled", "disabled");
            $("#right" + i).val('').attr("disabled", "disabled")
        }
        //console.log('------------------>');
		//ExecuteCanvas1();
		//camera_paly();
    });
	
	function ajaxGet(url, successCallback, errorCallback) 
	{
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
     function InitCoordinate(num,t,l,d,r)
     {
        console.log('InitCoordinate num= ' + num + 't =' + t);
         $("#up"+num).val(parseInt(t*bgheight));
         $("#down"+num).val(parseInt(d*bgheight));
         $("#left"+num).val(parseInt(l*bgwidth));
         $("#right"+num).val(parseInt(r*bgwidth));
         $("#zxxCropBox" + num).css("width",parseInt(r*bgwidth)-parseInt(l*bgwidth));
         $("#zxxCropBox" + num).css("height",parseInt(d*bgheight)-parseInt(t*bgheight));
         $("#zxxCropBox" + num).css("top", parseInt(t*bgheight));
         $("#zxxCropBox" + num).css("left", parseInt(l*bgheight));
     }

        //获取摄像头图片
    window.onload=function()
    {
		console.log('window.onload = ');
        loadJS(num);//根据拼接方式初始化右侧拼接图 num=2/3/4
        var names=[];
        for (var i = 1; i <=parseInt(num); i++) {
            var name=$("#videoName"+i).val();
            if(name!='-1')
              names.push(name);
        }
        GetVideoImgs(names,true,-1);  //正式需开启
    }

   function GetParams()//如果是修改拼接，需要解析URL传递过来的参数
   {
        var query = window.location.search.substring(1);
        var vars = query.split("&");
		console.log('=============GetParams============'+ vars);
        if(vars.length===8)
        {
            $("#Tname").val(decodeURIComponent(vars[0]).split('=')[1]);
            $("#TID").val(vars[1].split('=')[1]);
            $("#playAdd").val(vars[2].split('=')[1]);
            $("#playPort").val(vars[3].split('=')[1]);
            $("#state").val(decodeURIComponent(vars[4]).split('=')[1]);
            $("#style").val(vars[5].split('=')[1]);
            $("#islock").val(vars[6].split('=')[1]);
            $("#isfold").val(vars[7].split('=')[1]);
            var height = vars[7].split('=')[1] === "1" ? 290 : 260;
            $("#playCanvas").css("height", height + "px");
            num=parseInt(vars[5].split('=')[1]);
            layui.use(['form'], function () {
            var form = layui.form;
            form.render();
           });
        }
   }
    //初始化相机数据，绑定下拉列表
    function InitData() 
	{
		
		console.log('get camera info ...');
		ajaxGet(camera_url, 
			function(result)
			{
				//console.log('camera ===> '+result);
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
		
        //var message = '{"msg":"get_camera_info"}';
        //$.ajax({
        //    type: "POST",
        //    dataType: "text",
        //    //url: '/json',
        //    url: apiAddress,
        //    contentType: "application/json",
        //    data: message,
        //   // async:false,
        //    timeout:3000,
        //    success: function (result) {
        //        var objs = JSON.parse(result);
        //        LoadVideo(objs.video_souce_info, 12);
        //        layui.use(['form'], function () {
        //            var form = layui.form;
        //            form.render();
        //        });
        //    }
        //});
      
    }

    //绑定相机名称下拉列表 data:数据集合，num：2/3/4
    function LoadVideo(data, num) 
    {
        console.log('LoadVideo = data = ' + JSON.stringify( data));
        cameras = data;
        var parent=$(".layui-row");
        if (num > 0 && ValidateStringAnd(data)) 
		{
            for (var i = 1; i <= num; i++) 
			{
				// console.log('------|||>>>>>>>');
                var obj = $("#videoName" + i);
                obj.html('');
                var str = "";
                var inputHidden="";
                var count=0;//记录$.each剔除已使用的相机后循环次数
                str += "<option value='-1' >请选择</option>";
                //循环绑定所有通道未使用的相机
                $.each(data, function (index, item) 
				{
				//	console.log('dd--->>> dfa ' + JSON.stringify(item));
                    if(ValidateStringAnd(item.camera_name))
                    {
                      //  console.log('item.camera_name  ===== > '+ item.camera_name);
                        var camera = _.find(cameras, function (obj) { return obj.camera_id == item.camera_id });//已经使用过
                        var camera1= _.find(tileArray, function (obj) { return obj.camera_id == item.camera_id });//此通道不存在
                        if (camera != null && camera1==null) //表示已经使用，不再添加
                        {
                            return true;
                        }
                       // console.log('');
                        // if (count == (i-1))
                        // str += "<option value='" + item.camera_id + "' selected>" + item.name + "</option>";
                        // else
                        str += "<option value='" + item.camera_id + "'>" + item.camera_name + "</option>";
                        //count++;
                    }
					// console.log('------>>>>>>>' + JSON.stringify(item));
                    if (i == 1) //只给第一个绑定，
                    {
                        inputHidden += "<input type='hidden' id='name_" + item.camera_id + "' value='" + item.camera_name + "' />";
                        inputHidden += "<input type='hidden' id='ip_" + item.camera_id + "' value='" + item.address + "' />";
                        inputHidden += "<input type='hidden' id='address_" + item.camera_id + "' value='" + item.address + "' />";
                        inputHidden += "<input type='hidden' id='port_" + item.camera_id + "' value='" + item.port + "' />";
                        inputHidden += "<input type='hidden' id='state_" + item.camera_id + "' value='" + item.state + "' />";
                    }
                });
				//console.log('str = ' + str);
				//console.log('obj = ' + obj);
				//console.log('inputHidden = '+  inputHidden);
                $(str).appendTo(obj);
                $(inputHidden).appendTo(parent);
            }
            LoadTileVideo(data);//上面是给每个下拉列表绑定数据，此方法主要是让相应的下拉列表选中相机
        }
    }


function LoadTileVideo(data)
{
	
	console.log('data == ' + JSON.stringify(data));
	console.log('tileArray.camera_group = ' + JSON.stringify(tileArray));
	//console.log('LoadTileVideo [lenght = '+ tileArray.camera_group.size +'] [tileArray = ' + JSON.stringify(tileArray.camera_group) + ']');
	// $.each(tileArray,function(index,item)
	// 	{
	// 		console.log('find item = ' + JSON.stringify(item));
	// 		var cam = _.find(data, 
	// 		function (obj) 
	// 		{ 
	// 			console.log('find obj = ' + JSON.stringify(obj));
	// 			return index == obj.camera_id 
	// 			});
	// 		console.log('[index = '+index +']cam = ' + cam);
	// 		if(cam!=undefined && cam!=null)
	// 		{
	// 			console.log('+++++ cam.camera_name =' + cam.camera_name);
	// 			$("#videoName"+(index+1)).val(cam.camera_name);
	// 		}
	// 		else
	// 		{
	// 			console.log('----');
	// 			$("#videoName"+(index+1)).val(data[item.camera_id].camera_name);
	// 		}
	// })
	//tileArray.camera_group.forEach(function(item, index, array) 
	//{
	//  console.log(item, index, array);
	//});
	
    for (let i = 0; i < tileArray.length; ++i)
    {
        //console.log('tileArray = ' + JSON.stringify(tileArray[i]));
        for (let j = 0; j < data.length; ++j)
        {
          //   console.log('=== ' + JSON.stringify(data[j]));
            if (data[j].camera_id === tileArray[i].camera_id)
            {

                var videoName = "videoName" +(parseInt(i) + 1);
                let cur_select = document.getElementById(videoName);//.selectedIndex = 2;
                cur_select.selectedIndex = parseInt(j);
               // $("#videoName"+(i+1)).val(data[j].camera_name);
                console.log('videoName = '+videoName+'cur_select = '+cur_select + '=== ' + $("#videoName"+(i+1)).val() + ", name = " + data[j].camera_name);
               // break;
            }
        }
    }

	
    layui.use(['form'], function () {
        var form = layui.form;
        form.render();
    });
}


    //确定事件/***/@@@@@@@@@@@@@@@@@@
    function RequeryCropArray() 
	{
		console.log('RequeryCropArray');
		
        var messageAll = '{"msg": "start_tile","video_souce_info": [';
        var style = $("#style").val();
        //iscrop = true;
        let mess = videoMsg(parseInt(style)); //拼接发送json
        if (!mess) return false;
        messageAll += mess;
        console.log(messageAll);
        sendAjax(messageAll, "set"); //发送拼接，获取拼接视频流
        let TID = $("#TID").val();
        var gettileinfo = '{"msg": "get_tile_info"}';
        //  console.log(gettileinfo);
        sendAjax(gettileinfo, "get");
    }
    //根据选择方式进行canvas处理
    function loadJS(num) 
	{
		console.log('loadJS');
        if (num - prevnum > 0)//追加js
        {
            for (let index = prevnum + 1; index <= num; index++) {
                var head = $("head");
                var script = $("<script>");
                $(script).attr('type', 'text/javascript');
                $(script).attr('src', 'js/Canvas' + index + '.js');
                $(head).append(script);
                InputDisabled(index);
            }
        } else if (num - prevnum < 0)//移除js
        {
            for (let index = num + 1; index <= prevnum; index++) {
                $("script[src='js/Canvas" + index + ".js']").remove();
                $("#imgDiv" + index).html('');
            }
        }
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

	function   camera_paly()
	{
		if (camera_play1 != null)
		{
			return;
		}
		console.log('==camera_paly== ');
		let container1 = document.getElementById('container1');
		let imgDiv = document.getElementById('imgDiv1');    // 存放mycanvas
		let clipImgDiv = document.getElementById('clipImgDiv1');    // 显示裁剪所获的图片
		var scaleX = 1;// 图片宽度缩放比例（当前实际/原始）
		var scaleY = 1;  // 图片高度缩放比例（当前实际/原始）
		var bgwidth = 417;
		var bgheight = 220;
		var myCanvas = document.createElement('canvas');
		myCanvas.setAttribute('id', 'myCanvas');
		myCanvas.style.display = 'block';
		/*myCanvas.style.position = 'absolute';*/
		myCanvas.width = bgwidth;
		myCanvas.height = bgheight;
		myCanvas.style.border = "1px solid #d3d3d3";
		myCanvas.innerText = '您的浏览器不支持 HTML5 canvas 标签。';
		myCanvas.style.zIndex = 'auto';
		container1.appendChild(myCanvas);
		camera_play1 = new Player();
		camera_play1.play("",/*url*/ 'udp://@224.1.1.3:20000' , myCanvas);
	}
    //切换摄像机，加载图片
    function ChangeVideoNmae(img, num) 
	{
		
		console.log('ChangeVideoNmae  num =' + num);
		$("#imgDiv" + num).html('');
		if (num == 1) 
		{ 
			image1 =  img; 
            console.log('cameras[num].url = '+cameras[num]);
			ExecuteCanvas1(cameras[num].url); 
		}
		else if (num == 2) { image2 =  img; ExecuteCanvas2(); }
		else if (num == 3) { image3 =  img; ExecuteCanvas3(); }
		else if (num == 4) { image4 =  img; ExecuteCanvas4(); }
		else if (num == 5) { image5 =  img; ExecuteCanvas5(); }
		else if (num == 6) { image6 =  img; ExecuteCanvas6(); }
		else if (num == 7) { image7 =  img; ExecuteCanvas7(); }
		else if (num == 8) { image8 =  img; ExecuteCanvas8(); }
		else if (num == 9) { image9 =  img; ExecuteCanvas9(); }
		else if (num == 10) { image10 =  img; ExecuteCanvas10(); }
		else if (num == 11) { image11 =  img; ExecuteCanvas11(); }
		else if (num == 12) { image12 =  img; ExecuteCanvas12(); }

		if (tileArray.length > 0 && num - 1 < tileArray.length)//初始化的数量存在和手动选择拼接方式数值不一样
		{
			for (let index = 0; index < tileArray.length; index++) 
            {
				if (tileArray[index].index == (num - 1))
				{
                    InitCoordinate(num, tileArray[index].x, tileArray[index].y, tileArray[index].w, tileArray[index].h);
                }
			}
		}
	}

    //启用4个方位input
    function InputDisabled(num) {
        $("#up" + num).attr("disabled", false);
        $("#down" + num).attr("disabled", false);
        $("#left" + num).attr("disabled", false);
        $("#right" + num).attr("disabled", false);
        $("#videoName" + num).attr("disabled", false)
        layui.use(['form'], function () {
            var form = layui.form;
            form.render();
        });
    }

    //根据方位input输入，移动截取框
    function ChangeUp1(e, xh) {
        var value = parseInt(e.value);
        var height = ID("zxxCropBox" + xh).clientHeight;
        if (value + height <= bgheight) {
            $("#zxxCropBox" + xh).css("top", parseInt(e.value));
            $("#down" + xh).val(parseInt(e.value) + height);
        }
    }
    function ChangeDown1(e, xh) {
        var value = parseInt(e.value);
        var height = ID("zxxCropBox" + xh).clientHeight;
        if (value <= bgheight && e.value >= height) {
            var top = parseInt(e.value) - height;
            $("#zxxCropBox" + xh).css("top", top);
            $("#up" + xh).val(top);
        }
    }
    function ChangeLeft1(e, xh) {
        var value = parseInt(e.value);
        var width = ID("zxxCropBox" + xh).clientWidth;
        if (value + width <= bgwidth) {
            var left = value + width;
            $("#zxxCropBox" + xh).css("left", parseInt(e.value));
            $("#right" + xh).val(left);
        }
    }
    function ChangeRight1(e, xh) {
        var value = parseInt(e.value);
        var width = ID("zxxCropBox" + xh).clientWidth;
        if (value <= bgwidth && value > width) {
            var left = bgwidth - width;
            $("#zxxCropBox" + xh).css("left", left);
            $("#left" + xh).val(left);
        }
    }
 
    var names=[];
    function GetVideoImgs(names,init,num) 
	{
		console.log('GetVideoImgs = ' + names);
        var style=parseInt($("#style").val());
        if(names.length<=0)
		{
            console.log('num = ' + num);
			 return false;
		}
        ChangeVideoNmae(null, num);
		return true;
        for (let index = 1; index <= names.length; index++) 
		{
			console.log('names = ' + names[index -1]);
               var message = '{"msg": "set_camera_info","video_souce_info": [';
                message += '{';

                if(ValidateStringOR(names[index-1]))
                {
					continue;
				}
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
                        GetCameraImage(send_str,init,num);
                       } 
                }
            }
        });
    }
    }

   function GetCameraImage(message,init,num)
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
                        if (objs.pics.length > 0) 
						{
                            for (let index = 1; index <= objs.pics.length; index++) 
							{
                                if(num<0)
                                {
                                    var number=index+1;
                                    if(objs.pics[index-1].status!="failed")
                                    {
                                        ChangeVideoNmae(videoAddress+objs.pics[index-1].pic_url,index); 
                                        $("#container"+number).attr("data-state",true);
                                    }else
                                    {
                                        $("#container"+number).attr("data-state",false);
                                    }
                                }
                                  else
                                  {
                                    if(objs.pics[index-1].status!="failed")
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
                            if(init) 
							{
								RequeryCropArray();
							}
                        }
                    }
                }
        });
   }


    //拼接发送信息  playCanvas
    let videoMsg = function (num) {
        let Tname = $("#Tname").val();
        let TID = $("#TID").val();
        let address = $("#playAdd").val();
        let islock = $("#islock").val();
        let port = $("#playPort").val();
        var style = $("#style").val();
        var isfold=$("#isfold").val();
        let messgae = "";
        if ( ValidateStringOR(Tname)) { layer.msg('请输入拼接通道名称'); return false; }
        if ( ValidateStringOR(TID)) { layer.msg('请输入通道ID'); return false; }
        if ( ValidateStringOR(address))  { layer.msg('请输入组播地址'); return false; }
        if ( ValidateStringOR(port)) { layer.msg('请输入组播端口'); return false; }
        if (num > 0) {
            for (var i = 1; i <= num; i++) {
                var state=$("#container"+i).attr("data-state");
                if(state=="false")
                  continue;
                var msg = "{";
                let videoName = $("#videoName" + i).val();
                if(ValidateStringOR(videoName))
               continue;
                let up = $("#up" + i).val();
                let down = $("#down" + i).val();
                let left = $("#left" + i).val();
                let right = $("#right" + i).val();
                //转换0-1坐标
                up = parseInt(up) / bgheight;
                down = parseInt(down) / bgheight;
                left = parseInt(left) / bgwidth;
                right = parseInt(right) / bgwidth;
                msg += '"camera_id": "' + videoName + '",';

                var cam_ip = $("#ip_" + videoName).val();
                // if (ValidateStringOR(cam_ip)) {
                //     //msg += '"urlorfile":"udp://' + $("#address_"+videoName).val() + ':' +$("#port_"+videoName).val() + '",';
                //     msg += '"urlorfile":"rtsp://admin:zhuoshi123@' + $("#address_"+videoName).val() + '",';
                // } else {
                //     msg += '"urlorfile":"rtsp://admin:admin12345@' + $("#ip_" + videoName).val() + '",';
                // }
                msg += '"urlorfile":"udp://' + $("#address_"+videoName).val() + ':' +$("#port_"+videoName).val() + '",';
                //msg+='"urlorfile":"'+modifyStyle+'",';
                //msg+='"urlorfile":"/root/1.264",';
                msg += '"vendor": 1,';
                msg += '"crop": { "l":' + left + ',"t":' + up + ',"r":' + right + ',"b":' + down + '},';
                msg += '"tile_video_id":"'+TID+'",';
                msg += '"idx_in_tile":' + (i - 1) + '},';
                messgae += msg;
            }
        }
        if(messgae=="")
        return messgae;
        messgae = messgae.substring(0, messgae.length - 1) + "],";
        messgae += '"video_tile": [{';
        messgae += '"ip": "'+TID+'",';
        messgae += '"name": "' + Tname + '",';
        messgae += '"address": "' + address + '",';
        messgae += '"port": "' + port + '",';
        messgae += '"style": ' + style + ',';
        messgae += '"state": 1,';
        messgae += '"islock":' + parseInt(islock) + ',';
        messgae += '"is_fold": ' + isfold + ',';
        messgae += '"id": "'+TID+'",';
        messgae += '"fix_1080p":' + parseInt(islock) + ',';
        messgae += '"multicast_ip":"'+address+'",';
        messgae += '"multicast_port":'+port+' }]';
        messgae += "}";
        return messgae;
    }

    //发送ajax，获取拼接流地址信息，拼接视频播放
    function sendAjax(send_str,action) {
        console.log('====================>>>video split send_str = ' + send_str);
        $.ajax({
            type: "POST",
            dataType: "text",
            //url: '/json',
            url: apiAddress,
            contentType: "application/json",
            data: send_str,
            async:false,
            timeout: 5 * 1000,
            success: function (result) {
                console.log(result);
                if(action==='get'){
                    var objs= JSON.parse(result);
                    if(objs.video_tile.length>0)
                    {
                        for (let index = 0; index < objs.video_tile.length; index++) {
                           if(objs.video_tile[index].id===$("#TID").val())
                            {
                                $("#pj_width").val(objs.video_tile[index].w);
                                $("#pj_height").val(objs.video_tile[index].h);
                                playVideo();
                            }
                        }
                    }
                  
                }else if(action==='set')
                {
                   // console.log(result);
                }
            }
        });
    }

    //播放视频
    function playVideo(osd) {
        var domain = document.domain;
        //var domain = "likp.top";
        var canvas = document.getElementById("playCanvas");
        if (!canvas) {
            return false;
        }
        if (!document.player) {
            document.player = new Player();
        }
        //var playAdd = "tstream://"+id;
        var playAdd = "tstream://"+$("#TID").val();
        document.player.play(domain, playAdd, canvas);
        return true;
    }

    //停止播放
    function stopVideo() {
        var canvas = document.getElementById("playCanvas");
        if(document.player!=null)
           document.player.stop();
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
	

    
    function sendGetTileAjax() 
	{
		//camera_url
		//let TID = $("#TID").val();
		//var new_video_split_url = video_split_url + TID;
		ajaxGet(all_video_split_url, 
			function(result)
			{
				//console.log('video split info all  ===> '+result);
				var objs = JSON.parse(result);
				//LoadVideo(objs.camera_infos, 12);
				//layui.use(['form'], function () {
				//	var form = layui.form;
				//	form.render();
				//});
				 if(objs!=null&&typeof(objs)!='undefined')
                 {
                     for(var i=0;i<objs.video_split_infos.length;i++)
                     {
                         if($("#TID").val()===objs.video_split_infos[i].split_channel_id)
						 {
							  
							 for (let ww = 0; ww < objs.video_split_infos[i].camera_group.length; ++ww)
                             {
                                console.log('push tile Array  --> ' +JSON.stringify(objs.video_split_infos[i].camera_group[ww]));
                                 tileArray.push(objs.video_split_infos[i].camera_group[ww]);
                             }

                             console.log('tileArray length = '+ tileArray.length);
						 }
                     }
                     cameras=objs.video_split_infos;
                 }
			},
			function(result)
			{
				console.log('get camera_url '+ camera_url +' failed !!! = ' + result);
			}
		);
		//var send_str='{"msg": "get_tile_info"}';
        //$.ajax({
        //    type: "POST",
        //    dataType: "text",
        //    //url: '/json',
        //    url: apiAddress,
        //    contentType: "application/json",
        //    data: send_str,
        //    timeout: 3000,
        //    //async:false,
        //    success: function (result) {
        //        //console.log(result);
        //        var objs = JSON.parse(result);
        //         if(objs!=null&&typeof(objs)!='undefined')
        //         {
        //             for(var i=0;i<objs.video_souce_info.length;i++)
        //             {
        //                 if($("#TID").val()==objs.video_souce_info[i].tile_video_id)
        //                 tileArray.push(objs.video_souce_info[i]);
        //             }
        //             cameras=objs.video_souce_info;
        //         }
        //    }
        //});
    }

    function BtnOkClick()
    {
        var flag = JudgeCamID();
        if (!flag) 
        { 
            layer.msg("不能有重复的摄像机！"); 
            return false; 
        }
        var isfold = $("#isfold").val();
        var height = isfold === "1" ? 290 : 260;
        $("#playCanvas").css("height", height + "px");
        var style = $("#style").val();
        var messageAll = '{"msg": "start_tile","video_souce_info": [';

        //iscrop = true;
        let mess = videoMsg(parseInt(style)); //拼接发送json
        if (!mess) {layer.msg('无有效相机进行拼接！');return false;}
        messageAll += mess;
        console.log(messageAll);
        sendAjax(messageAll, "set"); //发送拼接，获取拼接视频流
        stopVideo();
        let TID = $("#TID").val();
        var gettileinfo = '{"msg": "get_tile_info"}';
        console.log(gettileinfo);
        sendAjax(gettileinfo, "get");      
    }

    function JudgeCamID() {
        var style = $("#style").val();
        console.log('JudgeCamID ==> '+ style);
        if (parseInt(style) > 0)
        {
            var cam_id = "";
            for (let index = 1; index <= parseInt(style); index++) 
            {
                var cam = $("#videoName" + index).val();
                console.log('videoName = ' + cam );
                if (cam_id.indexOf(cam) >= 0) 
                {

                    return false;
                }
                else if (cam == '-1') 
                { }
                else
                {
                    cam_id += $("#videoName" + index).val() + ",";
                }
            }
            return true;
        }
    }

        function ModifyOSDsTYLE()
        {
            $("#osd").css("display","block");
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
            document.onmousemove = function (e) 
			{
			//	console.log('Modify --> '+e);
                if (isDown == false) {
                    return;
                }

                var nx = e.clientX;
                var ny = e.clientY;
                var clientx=parseInt(ID("osd").clientWidth);
                var clientY=parseInt(ID("osd").clientHeight);
                //计算移动后的左偏移量和顶部的偏移量
                var nl = nx - (x - l);
                var nt = ny - (y - t);
                if(nl<0)
                nl=0;
                else if(nl+clientx>519)
                nl=519-clientx;
                if(nt<0)
                nt=0;
                else if(nt+clientY>260)
                nt=260-clientY;
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

    function SendOSD()
    {
        var obj=document.getElementById("txtOSD");//获取文本框字体大小
        var top=parseInt(getCss(ID("osd"), "top"));
        var left=parseInt(getCss(ID("osd"), "left"));
        //实际top值为：top+input-fontsize   parseInt(obj.style.fontSize)
        var send_str='{"msg":"set_tile_osd",';
        if($("#pj_width").val()=="" || $("#pj_height").val()=="")
        { 
            layer.msg('请先进行摄像机拼接');return false;
        }
        var videoX= parseInt( $("#pj_width").val());
        var videoY=  parseInt($("#pj_height").val());
        var canvasHei= parseInt(getCss(ID("playCanvas"), "height")); 
        left=parseInt(left)/520;
        top=parseInt(top)/canvasHei;
         var fontSize=Math.ceil( Math.round(parseInt(videoX)/520,2)*20);  
         fontSize=fontSize<100?fontSize:100;
        send_str+='"tile_id":"'+$("#TID").val()+'","txt":"'+obj.innerText+'","x":'+left+',"y":'+top+',"ft_size":'+fontSize+'}';
        console.log(send_str);
        sendAjax(send_str,"osd");
        var gettileinfo = '{"msg": "get_tile_info"}';
        console.log(gettileinfo);
        sendAjax(gettileinfo, "get");
        $("#osd").css("display","none");
    } 
    function ValidateStringOR(str)
    {
        var flag=false;
        if(typeof (str) == 'undefined' || str == null || str == ''|| str=='null' ||str=='-1'|| str=='undefined')
        flag= true;
        return flag;
    }
    function ValidateStringAnd(str)
    {
        var flag=false;
        if(typeof (str) != 'undefined' && str != null && str != ''&& str!='null'&& str!='undefined')
        flag= true;
        return flag;
    }