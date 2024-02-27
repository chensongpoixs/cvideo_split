package cn.edu.sgu.www.knife4j.dto;

import io.swagger.annotations.ApiModel;
import io.swagger.annotations.ApiModelProperty;
import lombok.Data;

/**
 * @author heyunlin
 * @version 1.0
 */
@Data
@ApiModel
public class UserLoginDTO {

    @ApiModelProperty(value = "用户名", required = true)
    private String username;

    @ApiModelProperty(value = "密码", required = true)
    private String password;
}