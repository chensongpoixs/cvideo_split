const video_split_address = 'http://192.168.2.91:9700';
//const video_split_address = 'http://127.0.0.1:9700';
const api_execl_address = 'http://192.168.2.91:8808/';
const decoder_url_address = 'ws://192.168.2.91:9600';




 		var addCameraInfo = function EachAddCameraData(data) 
        {
           
            let camera_infos_ = [];
             
            camera_infos_.push({
            						"camera_id": data.camera_id,
            						"ip": data.ip,
                                    "camera_name": data.camera_name,
                                    "address": data.address, 
                                    "port": parseInt(data.port),
                                    "url": "udp://@"+data.address+":"+ parseInt(data.port) });
            
            
            return JSON.stringify({
                "camera_infos": camera_infos_
            });
        }

 