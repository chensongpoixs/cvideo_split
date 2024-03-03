package cn.edu.sgu.www.knife4j;

import cn.edu.sgu.www.knife4j.excel.ExeclUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

@SpringBootApplication
public class SpringbootKnife4j {

    static Logger logger = LoggerFactory.getLogger(SpringbootKnife4j.class);

    public static void main(String[] args)
    {
        if (logger.isDebugEnabled()) {
            logger.debug("启动springboot整合knife4j案例项目...");
        }
//        File file = new File("D:\\Work\\cai\\cvideo_split\\res\\execl\\test_read_camera.xlsx");
//        List<ArrayList<String>> list = null;
//        try {
//            list = ExeclUtils.readExcel(file);
//        } catch (IOException e) {
//            throw new RuntimeException(e);
//        }
//        for (int i = 0; i < list.size(); i++) {
//            //第一行全部数据
//            List list1=list.get(i);
//            for (int j = 0; j < list1.size(); j++) {
//                //第一行每个单元格数据
//                System.out.println(list1.get(j));
//            }
//        }

        SpringApplication.run(SpringbootKnife4j.class, args);
    }

}