package cn.edu.sgu.www.knife4j.excel;

import org.apache.poi.hssf.usermodel.HSSFCell;
import org.apache.poi.hssf.usermodel.HSSFRow;
import org.apache.poi.hssf.usermodel.HSSFSheet;
import org.apache.poi.hssf.usermodel.HSSFWorkbook;
import org.apache.poi.xssf.usermodel.XSSFCell;
import org.apache.poi.xssf.usermodel.XSSFRow;
import org.apache.poi.xssf.usermodel.XSSFSheet;
import org.apache.poi.xssf.usermodel.XSSFWorkbook;
import org.springframework.web.multipart.MultipartFile;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

public class ExeclUtils
{
    public static int totalRows; //sheet中总行数
    public static int totalCells; //每一行总单元格数


    /**
     * read the Excel .xlsx,.xls
     * @param file jsp中的上传文件
     * @return
     * @throws IOException
     */
    public static List<ArrayList<String>> readExcel(MultipartFile file) throws IOException {
        if(file==null)
        {
            return null;
        }
//        else
//        {
//            String postfix = ExcelTool.getPostfix(file.getOriginalFilename());
//            if(!ExcelTool.EMPTY.equals(postfix)){
//                if(ExcelTool.OFFICE_EXCEL_2003_POSTFIX.equals(postfix)){
                    return readXls(file.getInputStream());
//                }else if(ExcelTool.OFFICE_EXCEL_2010_POSTFIX.equals(postfix)){
//                    return readXlsx(file.getInputStream());
//                }else{
//                    return null;
//                }
//            }
//        }
//        return null;
    }


    /**
     * read the Excel 2010 .xlsx
     * @param file
     * @return
     * @throws IOException
     */
    @SuppressWarnings("deprecation")
    public static List<ArrayList<String>> readXlsx(InputStream input){
        List<ArrayList<String>> list = new ArrayList<ArrayList<String>>();
        // IO流读取文件
//        InputStream input = null;
        XSSFWorkbook wb = null;
        ArrayList<String> rowList = null;
        try {
//            input = new FileInputStream(file);
            // 创建文档
            wb = new XSSFWorkbook(input);
            //读取sheet(页)
            for(int numSheet=0;numSheet<wb.getNumberOfSheets();numSheet++){
                XSSFSheet xssfSheet = wb.getSheetAt(numSheet);
                if(xssfSheet == null){
                    continue;
                }
                totalRows = xssfSheet.getLastRowNum();
                //读取Row,从第二行开始
                for(int rowNum = 0;rowNum <= totalRows;rowNum++){
                    XSSFRow xssfRow = xssfSheet.getRow(rowNum);
                    if(xssfRow!=null){
                        rowList = new ArrayList<String>();
                        totalCells = xssfRow.getLastCellNum();
                        //读取列，从第一列开始
                        for(int c=0;c<=totalCells+1;c++){
                            XSSFCell cell = xssfRow.getCell(c);
                            if(cell==null){
                                rowList.add(ExcelTool.EMPTY);
                                continue;
                            }
                            rowList.add(ExcelTool.getXValue(cell).trim());
                        }
                        list.add(rowList);
                    }
                }
            }
            return list;
        } catch (IOException e) {
            e.printStackTrace();
        } finally{
//            try {
//                input.close();
//            } catch (IOException e) {
//                e.printStackTrace();
//            }
        }
        return null;

    }
    /**
     * read the Excel 2003-2007 .xls
     * @param file
     * @return
     * @throws IOException
     */
    public static List<ArrayList<String>> readXls(InputStream input){
        List<ArrayList<String>> list = new ArrayList<ArrayList<String>>();
        // IO流读取文件
//        InputStream input = null;
        HSSFWorkbook wb = null;
        ArrayList<String> rowList = null;
        try {
//            input = new FileInputStream(file);
            // 创建文档
            wb = new HSSFWorkbook(input);
            //读取sheet(页)
            for(int numSheet=0;numSheet<wb.getNumberOfSheets();numSheet++){
                HSSFSheet hssfSheet = wb.getSheetAt(numSheet);
                if(hssfSheet == null){
                    continue;
                }
                totalRows = hssfSheet.getLastRowNum();
                //读取Row,从第二行开始
                for(int rowNum = 0;rowNum <= totalRows;rowNum++){
                    HSSFRow hssfRow = hssfSheet.getRow(rowNum);
                    if(hssfRow!=null){
                        rowList = new ArrayList<String>();
                        totalCells = hssfRow.getLastCellNum();
                        //读取列，从第一列开始
                        for(short c=0;c<=totalCells+1;c++){
                            HSSFCell cell = hssfRow.getCell(c);
                            if(cell==null){
                                rowList.add(ExcelTool.EMPTY);
                                continue;
                            }
                            rowList.add(ExcelTool.getHValue(cell).trim());
                        }
                        list.add(rowList);
                    }
                }
            }
            return list;
        } catch (IOException e) {
            e.printStackTrace();
        } finally{
//            try {
////                input.close();
//            } catch (IOException e) {
//                e.printStackTrace();
//            }
        }
        return null;
    }



    @SuppressWarnings("deprecation")
    public static List<ArrayList<String>> readMultipartFileXlsx(MultipartFile file){
        List<ArrayList<String>> list = new ArrayList<ArrayList<String>>();
        // IO流读取文件
//        InputStream input = null;
        XSSFWorkbook wb = null;
        ArrayList<String> rowList = null;
        try {
//            input = new FileInputStream(file);
//            String postfix = ExcelTool.getPostfix(file.getName());
//            if(!ExcelTool.EMPTY.equals(postfix)){
//                if(ExcelTool.OFFICE_EXCEL_2003_POSTFIX.equals(postfix)){
//                    return readXls(file);
//                }else if(ExcelTool.OFFICE_EXCEL_2010_POSTFIX.equals(postfix)){
//                    return readXlsx(file);
//                }else{
//                    return null;
//                }
//            }
            // 创建文档
            wb = new XSSFWorkbook(file.getInputStream());
            //读取sheet(页)
            for(int numSheet=0;numSheet<wb.getNumberOfSheets();numSheet++){
                XSSFSheet xssfSheet = wb.getSheetAt(numSheet);
                if(xssfSheet == null){
                    continue;
                }
                totalRows = xssfSheet.getLastRowNum();
                //读取Row,从第二行开始
                for(int rowNum = 0;rowNum <= totalRows;rowNum++){
                    XSSFRow xssfRow = xssfSheet.getRow(rowNum);
                    if(xssfRow!=null){
                        rowList = new ArrayList<String>();
                        totalCells = xssfRow.getLastCellNum();
                        //读取列，从第一列开始
                        for(int c=0;c<=totalCells+1;c++){
                            XSSFCell cell = xssfRow.getCell(c);
                            if(cell==null){
                                rowList.add(ExcelTool.EMPTY);
                                continue;
                            }
                            rowList.add(ExcelTool.getXValue(cell).trim());
                        }
                        list.add(rowList);
                    }
                }
            }
            return list;
        } catch (IOException e) {
            e.printStackTrace();
        } finally{
//            try {
//                input.close();
//            } catch (IOException e) {
//                e.printStackTrace();
//            }
        }
        return null;

    }
}
