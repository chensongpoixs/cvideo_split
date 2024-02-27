package cn.edu.sgu.www.knife4j.controller;

import cn.edu.sgu.www.knife4j.dto.CameraInfo;
import cn.edu.sgu.www.knife4j.dto.UserLoginDTO;
import cn.edu.sgu.www.knife4j.excel.ExeclUtils;
import com.fasterxml.jackson.annotation.JsonProperty;
import io.swagger.annotations.*;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.multipart.MultipartFile;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

/**
 * @author heyunlin
 * @version 1.0
 */
@Api(tags = "用户管理")
@Slf4j
@RestController
@RequestMapping(value = "/user", produces = "application/json;charset=utf-8")
public class UserController {

    @ApiOperation("登录认证")
    @RequestMapping(value = "/login", method = RequestMethod.POST)
    public String login(UserLoginDTO userLoginDTO) {
        return "登录成功~";
    }

    @ApiOperation("用户注册")
    @RequestMapping(value = "/register", method = RequestMethod.POST)
    public String register(
            @ApiParam(value = "用户名",  required = true) String username,
            @ApiParam(value = "密码",  required = true) String password) {
        return "注册成功~";
    }

    @ApiOperation("解析表格")
    @RequestMapping(value = "/parse_table", method = RequestMethod.POST)
    @ApiImplicitParams({
            @ApiImplicitParam(name = "username", value = "用户名", allowEmptyValue = true),
            @ApiImplicitParam(name = "password", value = "密码", allowEmptyValue = true),

    })
    public ExcelTableInfo parse_table(
            @ApiParam(value = "xx.zip", required = true)
            @RequestParam
            final MultipartFile file)
    {

        List<CameraInfo> cameraInfos = new ArrayList<>();
        List<ArrayList<String>> list = null;
        list = ExeclUtils.readMultipartFileXlsx(file );
        for (int i = 1; i < list.size(); i++) {
            //第一行全部数据
            List list1=list.get(i);

            CameraInfo cameraInfo = new CameraInfo();
            cameraInfo.setCamera_name(list1.get(1).toString());
            cameraInfo.setIp(list1.get(2).toString());
            cameraInfo.setAddress(list1.get(3).toString());
            cameraInfo.setPort(Integer.parseInt(list1.get(4).toString()));
            cameraInfos.add(cameraInfo);
            log.info(cameraInfos.toString());
//            for (int j = 1; j < list1.size(); j++) {
//                //第一行每个单元格数据
//                log.info(list1.get(j).toString());
////                System.out.println(list1.get(j));
//                if (j == )
//            }
        }
        if(cameraInfos.size()<=0)
        {
            return new ExcelTableInfo(1, cameraInfos);
        }

        return new ExcelTableInfo(0, cameraInfos);
    }


    @RequiredArgsConstructor
    private static final class ExcelTableInfo {
        private final int code;

        private final List<CameraInfo> cameraInfos;
        @JsonProperty("code")
        private int get_code () {return code;}


        @JsonProperty("camera_group")
        private List<CameraInfo> getCameraGroups () {return cameraInfos;}

    }

}