/*
 * Copyright (c) 2020 the original author or authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

package cn.edu.sgu.www.knife4j.filter;


import static java.util.Collections.emptyList;
import static java.util.Collections.list;
import static java.util.function.Function.identity;

import com.fasterxml.jackson.databind.ObjectMapper;
import java.io.IOException;
import java.util.List;
import java.util.UUID;
import java.util.stream.Collectors;
import javax.servlet.Filter;
import javax.servlet.FilterChain;
import javax.servlet.FilterConfig;
import javax.servlet.ServletException;
import javax.servlet.ServletRequest;
import javax.servlet.ServletResponse;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import lombok.RequiredArgsConstructor;
import lombok.SneakyThrows;
import lombok.extern.slf4j.Slf4j;
import lombok.val;
import org.springframework.context.annotation.Profile;
import org.springframework.core.annotation.Order;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Component;
import org.springframework.util.StringUtils;

/**
 * Filter created to validate if this application has access to requested model
 */

@Component
@RequiredArgsConstructor
@Profile("!integration-test")
@Order(1)
@Slf4j

public class SecurityValidationFilter implements Filter {

    public static final String VERIFICATION = "Verification";

//    private final ResponseExceptionHandler handler;
    private final ObjectMapper objectMapper;

//    private final ModelStatisticCacheProvider modelStatisticCacheProvider;

    @Override
    public void init(final FilterConfig filterConfig) {

    }

    @Override
    public void doFilter(
            final ServletRequest servletRequest,
            final ServletResponse servletResponse,
            final FilterChain filterChain
    ) throws IOException, ServletException {
        val httpRequest = (HttpServletRequest) servletRequest;
        val httpResponse = (HttpServletResponse) servletResponse;
//        httpRequest.g

//        ((HttpServletResponse) servletResponse).setHeader("Access-Control-Allow-Headers", "X-Custom-Header");
//        ((HttpServletResponse) servletResponse).setHeader("Access-Control-Allow-Credentials", "true");
//        Access-Control-Allow-Headers: X-Custom-Header
//        Access-Control-Allow-Credentials: true
//        log.info("==============================================doFilter==============================================" + httpRequest.getMethod().toLowerCase());
        String header_Origin = "*";
        if (httpRequest.getHeader("Origin") != null)
        {
            if (httpRequest.getHeader("Origin").charAt( httpRequest.getHeader("Origin").length()-1) != '/')
            {
                header_Origin = httpRequest.getHeader("Origin");
            }
            else
            {
                header_Origin = httpRequest.getHeader("Origin").substring(0, httpRequest.getHeader("Origin").length()-1);
            }
        }
//        log.info("header_Origin = ==== " + header_Origin);
        httpResponse.setHeader("Access-Control-Allow-Origin", header_Origin);
        if (httpRequest.getMethod().toLowerCase().equals( "options"))
        {
//            log.info("===================options============");
            httpResponse.setStatus(204);
            httpResponse.setHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS, PUT, DELETE");
            httpResponse.setHeader("Access-Control-Request-Headers", "Content-Type");
//            httpResponse.setHeader("Access-Control-Request-Headers", "Content-Type, Access-token, x-api-key, x-token");
            httpResponse.setHeader("Access-Control-Allow-Headers", "DNT,User-Agent,X-Requested-With,If-Modified-Since,Cache-Control,Content-Type,Range,x-api-key,x-token");
            httpResponse.setHeader("Access-Control-Max-Age", "1728000"); // ,Authorization,X-Auth-Token

//            httpResponse.setHeader("Access-Control-Allow-Origin", "http://192.168.1.66:8080");
           httpResponse.setHeader("Content-Length", "0");
            httpResponse.setHeader("Content-Type", "text/plain; charset=UTF-8");

            //            val objectResponseEntity = handler.handleDefinedExceptions(new BadFormatModelKeyException());
//            buildException(httpResponse, objectResponseEntity);

//            response.setStatus(responseEntity.getStatusCode().value());
//            httpResponse.setContentType(MediaType.APPLICATION_JSON_VALUE);
//            httpResponse.getWriter().append(objectMapper.writeValueAsString(responseEntity.getBody()));
            return;
        }

        filterChain.doFilter(servletRequest, servletResponse);
    }

    @Override
    public void destroy() {

    }

    @SneakyThrows
    private void buildException(final HttpServletResponse response, final ResponseEntity<?> responseEntity) {
        response.setStatus(responseEntity.getStatusCode().value());
        response.setContentType(MediaType.APPLICATION_JSON_VALUE);
        response.getWriter().append(objectMapper.writeValueAsString(responseEntity.getBody()));
        //response.getWriter().flush();
        //don't need to flush or close the writer
    }

}
