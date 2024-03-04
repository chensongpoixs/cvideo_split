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
            logger.debug(" Execl Tool Server ");
        }
        SpringApplication.run(SpringbootKnife4j.class, args);
    }

}