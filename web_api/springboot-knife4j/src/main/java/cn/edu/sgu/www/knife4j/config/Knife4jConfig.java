package cn.edu.sgu.www.knife4j.config;

import com.github.xiaoymin.knife4j.spring.extension.OpenApiExtensionResolver;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import springfox.documentation.builders.ApiInfoBuilder;
import springfox.documentation.builders.PathSelectors;
import springfox.documentation.builders.RequestHandlerSelectors;
import springfox.documentation.service.ApiInfo;
import springfox.documentation.service.Contact;
import springfox.documentation.spi.DocumentationType;
import springfox.documentation.spring.web.plugins.Docket;
import springfox.documentation.swagger2.annotations.EnableSwagger2WebMvc;

/**
 * Knife4j配置类
 * @author heyunlin
 * @version 1.0
 */
@Configuration
@EnableSwagger2WebMvc
public class Knife4jConfig {

    // 指定Controller包路径（必须）
    private static final String basePackage = "cn.edu.sgu.www.knife4j.controller";
    private static final String host = "LAPTOP-0N6P8HTP";
    private static final String title = "Spring Boot整合Knife4j案例项目";
    private static final String description = title + "在线API文档";
    private static final String termsOfServiceUrl = "https://www.apache.org/licenses/LICENSE-2.0";
    private static final String contactName = "heyunlin"; // 联系人
    private static final String contactUrl = "https://gitee.com/he-yunlin";
    private static final String contactEmail = "heyl163_com@163.com";
    private static final String version = "1.0.0";

    private final OpenApiExtensionResolver openApiExtensionResolver;

    @Autowired
    public Knife4jConfig(OpenApiExtensionResolver openApiExtensionResolver) {
        this.openApiExtensionResolver = openApiExtensionResolver;
    }

    @Bean
    public Docket docket() {
        String groupName = "1.0.0";

        return new Docket(DocumentationType.SWAGGER_2)
                .host(host)
                .apiInfo(apiInfo())
                .groupName(groupName)
                .select()
                .apis(RequestHandlerSelectors.basePackage(basePackage))
                .paths(PathSelectors.any())
                .build()
                .extensions(openApiExtensionResolver.buildExtensions(groupName));
    }

    private ApiInfo apiInfo() {
        return new ApiInfoBuilder()
                .title(title)
                .description(description)
                .termsOfServiceUrl(termsOfServiceUrl)
                .contact(new Contact(contactName, contactUrl, contactEmail))
                .version(version)
                .build();
    }

}