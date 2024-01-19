//#include <iostream>
//#include <opencv2/core.hpp>
//#include <opencv2/highgui.hpp>
//#include <opencv2/imgproc.hpp>
//
//__global__ void cropImage(const cv::Mat srcImg, int xOffset, int yOffset, int width, int height) {
//     获取当前线程索引
//    const int threadIdx = blockDim.x * blockIdx.y + blockIdx.x;
//
//    if (threadIdx >= width * height) return;
//
//     计算原始图像上每个像素点的位置
//    int row = threadIdx / width;
//    int col = threadIdx % width;
//
//     根据偏移量调整新图像上的位置
//    int newRow = row - yOffset;
//    int newCol = col - xOffset;
//
//     判断新位置是否越界
//    if ((newRow >= 0 && newRow < height) && (newCol >= 0 && newCol < width)) {
//         将原始图像上的像素值复制到新图像上相应位置
//        dstImg.at<uchar>(row, col) = srcImg.at<uchar>(newRow, newCol);
//    }
//    else {
//         设置超出边界部分为黑色
//        dstImg.at<uchar>(row, col) = 0;
//    }
//}
//
//int main() {
//     加载源图像
//    cv::Mat srcImg = cv::imread("input_image.jpg", cv::IMREAD_GRAYSCALE);
//
//     定义裁剪区域的起始坐标和大小
//    int xOffset = 100;
//    int yOffset = 50;
//    int width = 300;
//    int height = 200;
//
//     创建目标图像
//    cv::Mat dstImg(srcImg.rows, srcImg.cols, CV_8UC1);
//
//     配置CUDA内存
//    uchar* devSrcPtr;
//    uchar* devDstPtr;
//    size_t imgSize = srcImg.total() * sizeof(uchar);
//    cudaMalloc((void**)&devSrcPtr, imgSize);
//    cudaMalloc((void**)&devDstPtr, imgSize);
//
//     将源图像从主机内存传输到设备内存
//    cudaMemcpy(devSrcPtr, srcImg.data, imgSize, cudaMemcpyHostToDevice);
//
//     定义网格和线程块的大小
//    cv::dim3 gridSize((width + BLOCK_SIZE - 1) / BLOCK_SIZE, (height + BLOCK_SIZE - 1) / BLOCK_SIZE);
//    cv::dim3 blockSize(BLOCK_SIZE, BLOCK_SIZE);
//
//     运行CUDA kernel函数进行图像裁剪
//    cropImage << <gridSize, blockSize >> > (devSrcPtr, xOffset, yOffset, width, height);
//
//     等待所有任务完成
//    cudaDeviceSynchronize();
//
//     将结果从设备内存传输回主机内存
//    cudaMemcpy(dstImg.data, devDstPtr, imgSize, cudaMemcpyDeviceToHost);
//
//     保存裁剪后的图像
//    cv::imwrite("output_image.jpg", dstImg);
//
//     清除CUDA内存
//    cudaFree(devSrcPtr);
//    cudaFree(devDstPtr);
//
//    return 0;
//}