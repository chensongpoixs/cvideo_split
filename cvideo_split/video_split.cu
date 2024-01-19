//#include <iostream>
//#include <opencv2/core.hpp>
//#include <opencv2/highgui.hpp>
//#include <opencv2/imgproc.hpp>
//
//__global__ void cropImage(const cv::Mat srcImg, int xOffset, int yOffset, int width, int height) {
//     ��ȡ��ǰ�߳�����
//    const int threadIdx = blockDim.x * blockIdx.y + blockIdx.x;
//
//    if (threadIdx >= width * height) return;
//
//     ����ԭʼͼ����ÿ�����ص��λ��
//    int row = threadIdx / width;
//    int col = threadIdx % width;
//
//     ����ƫ����������ͼ���ϵ�λ��
//    int newRow = row - yOffset;
//    int newCol = col - xOffset;
//
//     �ж���λ���Ƿ�Խ��
//    if ((newRow >= 0 && newRow < height) && (newCol >= 0 && newCol < width)) {
//         ��ԭʼͼ���ϵ�����ֵ���Ƶ���ͼ������Ӧλ��
//        dstImg.at<uchar>(row, col) = srcImg.at<uchar>(newRow, newCol);
//    }
//    else {
//         ���ó����߽粿��Ϊ��ɫ
//        dstImg.at<uchar>(row, col) = 0;
//    }
//}
//
//int main() {
//     ����Դͼ��
//    cv::Mat srcImg = cv::imread("input_image.jpg", cv::IMREAD_GRAYSCALE);
//
//     ����ü��������ʼ����ʹ�С
//    int xOffset = 100;
//    int yOffset = 50;
//    int width = 300;
//    int height = 200;
//
//     ����Ŀ��ͼ��
//    cv::Mat dstImg(srcImg.rows, srcImg.cols, CV_8UC1);
//
//     ����CUDA�ڴ�
//    uchar* devSrcPtr;
//    uchar* devDstPtr;
//    size_t imgSize = srcImg.total() * sizeof(uchar);
//    cudaMalloc((void**)&devSrcPtr, imgSize);
//    cudaMalloc((void**)&devDstPtr, imgSize);
//
//     ��Դͼ��������ڴ洫�䵽�豸�ڴ�
//    cudaMemcpy(devSrcPtr, srcImg.data, imgSize, cudaMemcpyHostToDevice);
//
//     ����������߳̿�Ĵ�С
//    cv::dim3 gridSize((width + BLOCK_SIZE - 1) / BLOCK_SIZE, (height + BLOCK_SIZE - 1) / BLOCK_SIZE);
//    cv::dim3 blockSize(BLOCK_SIZE, BLOCK_SIZE);
//
//     ����CUDA kernel��������ͼ��ü�
//    cropImage << <gridSize, blockSize >> > (devSrcPtr, xOffset, yOffset, width, height);
//
//     �ȴ������������
//    cudaDeviceSynchronize();
//
//     ��������豸�ڴ洫��������ڴ�
//    cudaMemcpy(dstImg.data, devDstPtr, imgSize, cudaMemcpyDeviceToHost);
//
//     ����ü����ͼ��
//    cv::imwrite("output_image.jpg", dstImg);
//
//     ���CUDA�ڴ�
//    cudaFree(devSrcPtr);
//    cudaFree(devDstPtr);
//
//    return 0;
//}