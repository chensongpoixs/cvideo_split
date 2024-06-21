layui.use(['form', 'element'], function () {
    var $ = layui.jquery
        , element = layui.element
        , form = layui.form;

      console.log('>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<');
    form.on('select(style)', function (data) {
        $("#pjStyle").val(data.value);
        // loadJS(parseInt(data.value));
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
        // num = parseInt(data.value);
    });
    // form.on('select(videoName1)', function (data) { var names = [data.value]; GetVideoImgs(names, false, 1); });
    // form.on('select(videoName2)', function (data) { var names = [data.value]; GetVideoImgs(names, false, 2); });
    // form.on('select(videoName3)', function (data) { var names = [data.value]; GetVideoImgs(names, false, 3); });
    // form.on('select(videoName4)', function (data) { var names = [data.value]; GetVideoImgs(names, false, 4); });
    // form.on('select(videoName5)', function (data) { var names = [data.value]; GetVideoImgs(names, false, 5); });
    // form.on('select(videoName6)', function (data) { var names = [data.value]; GetVideoImgs(names, false, 6); });
    // form.on('select(videoName7)', function (data) { var names = [data.value]; GetVideoImgs(names, false, 7); });
    // form.on('select(videoName8)', function (data) { var names = [data.value]; GetVideoImgs(names, false, 8); });
    // form.on('select(videoName9)', function (data) { var names = [data.value]; GetVideoImgs(names, false, 9); });
    // form.on('select(videoName10)', function (data) { var names = [data.value]; GetVideoImgs(names, false, 10); });
    // form.on('select(videoName11)', function (data) { var names = [data.value]; GetVideoImgs(names, false, 11); });
    // form.on('select(videoName12)', function (data) { var names = [data.value]; GetVideoImgs(names, false, 12); });
    // form.on('select(islock)', function (data) { $("#pjislock").val(data.value); });
});
// defaulat main
$(function () 
{
         
         console.log('camerain.js main -->');
    });


  //获取摄像头信息
    window.onload=function()
    {
		console.log('window.onload = ');
        //loadJS(num);//根据拼接方式初始化右侧拼接图 num=2/3/4
       // console.log('num ===> ' + num);
        // var names=[];
        // for (var i = 1; i <=parseInt(num); i++) 
        // {
        //     var name=$("#videoName"+i).val();
        //     if(name!='-1')
        //     {
        //         names.push(name);
        //     }
        // }
        // for (var i = 0; i < names.length; ++i)
        // {
        //      GetVideoImgs(names,true,-1);  //正式需开启
        //    // ChangeVideoNmae(i+1, null, num);
        // }
       
      
    }



    function add_camera_info()
    {
    	console.log('add_camera_info');
    	  let cameraName = $("#cameraName").val();
    	  let cameraAddress  = $("#cameraAddress").val();
    	  let multicastAddress  = $("#multicastAddress").val();
    	  let multicastPort  = $("#multicastPort").val();
    	  console.log("cameraName = " + cameraName + "\n, cameraAddress = " +cameraAddress +  "\n, multicastAddress = " + multicastAddress + "\n, multicastPort = " + multicastPort);
    	  // let cameraAddress  = $("#cameraAddress").val();
    	  // let cameraAddress  = $("#cameraAddress").val();

    	  SendAddOneCameraInfo({  'ip': cameraAddress, 'camera_name': cameraName, 'port': multicastPort, 'address': multicastAddress});
    }

    function add_camera_player_play()
    {

    	console.log('add_camera_player_play');
    	  let cameraName = $("#cameraName").val();
    	  let cameraAddress  = $("#cameraAddress").val();
    	  let multicastAddress  = $("#multicastAddress").val();
    	  let multicastPort  = $("#multicastPort").val();
    	  console.log("cameraName = " + cameraName + "\n, cameraAddress = " +cameraAddress +  "\n, multicastAddress = " + multicastAddress + "\n, multicastPort = " + multicastPort);
    	  // let cameraAddress  = $("#cameraAddress").val();
    	  // let cameraAddress  = $("#cameraAddress").val();

    	  SendAddOneCameraInfo({  'ip': cameraAddress, 'camera_name': cameraName, 'port': multicastPort, 'address': multicastAddress});




    }

    //发送execl导入请求
        function SendAddOneCameraInfo(data) 
        {
            var send_str = addCameraInfo(data);
            console.log(send_str);
            $.ajax({
                type: "POST",
                dataType: "text",
                url: video_split_address + '/video_split/v1/add_camera_info',
                contentType: "application/json",
                data: send_str,
                timeout: 5000,
                success: function (result) {
                    var objs = JSON.parse(result);
                    console.log('add_camera_info result = ' + result);
                    if (objs.result == 0)
                    {
                       // InitData();//再读取绑定table
                    	console.log('add camera info  ok  ' + result);
                        //layer.msg('摄像头资源添加成功!');
                    }
                    else 
                    {
                    	console.log("add camera info failed !!! ==> " + result);
                    }
                    
                }
            });
        }


   // play 
        function camera_player_play() 
		{
			     
			    // var domain = document.domain;
			 let multicastAddress  = $("#multicastAddress").val();
    	  	let multicastPort  = $("#multicastPort").val();
			    var domain = "likp.top";
			    if (!document.player) 
			    {
			        document.player = new Player();
			        const canvasId = "playCanvas";
			        var canvas = document.getElementById(canvasId);
			        if (!canvas) 
			        {
			            return false;
			        }
			         canvas.width = 1920;
			         canvas.height = 1080;
			        //var url = " tstream://" + data.camera_id;
			        //var url = " tstream://t100";
			        var url = 'udp://@' + multicastAddress + ':' + multicastPort;
			        console.log('camera_player_play = url = ' + url);
			        document.player.play(domain, url, canvas);
			        // 将点击事件与Canvas关联起来
			        // canvas.addEventListener('dblclick', handleClick);
			    }
			    else 
			    {
			        console.log("is playering");
					// document.player.stop();
			        // document.player = null;
					// document.player = new Player();
			        // const canvasId = "playCanvas_camera";
			        // var canvas = document.getElementById(canvasId);
			        // if (!canvas) 
			        // {
					// 	console.log('->>>>>>>>>>>camera  palyer canvas ');
			        //     return false;
			        // }
			        // //var url = " tstream://" + data.camera_id;
			        // //var url = " tstream://t100";
			        // var url = data.url;
			        // document.player.play(domain, url, canvas);
			    }
			    return true;
		}


		function camera_player_stop()
		{
			if (document.player) 
		    {
		        document.player.stop();
		        document.player = null;
		    }
		}