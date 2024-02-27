package cn.edu.sgu.www.knife4j.dto;


import com.fasterxml.jackson.annotation.JsonProperty;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class CameraInfo
{
    @JsonProperty("ip")
    private String ip;

    @JsonProperty("camera_name")
    private String camera_name;

    @JsonProperty("address")
    private String address;


    @JsonProperty("port")
    private int port;



}
