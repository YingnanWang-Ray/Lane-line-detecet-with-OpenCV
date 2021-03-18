#include<cv.h>
#include<cxcore.h>
#include<highgui.h>
#include <core.hpp>
#include <highgui.hpp>
#include <stdio.h>
#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
#include<iostream>    
#include <opencv2/imgproc/imgproc.hpp>  
#include<cstdio>
#include<iostream>
using namespace std;
using namespace cv;
#define INF 99999999 

int main(){
	//声明IplImage指针
	IplImage* pFrame = NULL;
	IplImage* pCutFrame = NULL;
	IplImage* pCutFrImg = NULL;
	IplImage* pCutBkImg = NULL;
	//声明CvMat指针
	CvMat* pCutFrameMat = NULL;
	CvMat* pCutFrMat = NULL;
	CvMat* pCutBkMat = NULL;
	//声明CvCapture指针
	CvCapture* pCapture = NULL;
	//声明CvMemStorage和CvSeg指针
	CvMemStorage* storage = cvCreateMemStorage();
	CvSeq* lines = NULL;
	//当前帧数
	int nFrmNum = 0;
	//裁剪的天空高度
	int CutHeight = 225;
	//不能打开则退出
	if (!(pCapture = cvCaptureFromFile("./1.avi"))){
		fprintf(stderr, "Can not open video file\n");
		return -2;
	}
	//每次读取一桢的视频
	while (pFrame = cvQueryFrame(pCapture)){
		//显示输入的图像
		//设置ROI裁剪图像
		cvSetImageROI(pFrame, cvRect(0, CutHeight, pFrame->width, pFrame->height - CutHeight));
		nFrmNum++;
		//第一次要申请内存p
		if (nFrmNum == 1){
			pCutFrame = cvCreateImage(cvSize(pFrame->width, pFrame->height - CutHeight), pFrame->depth, pFrame->nChannels);
			cvCopy(pFrame, pCutFrame, 0);
			pCutBkImg = cvCreateImage(cvSize(pCutFrame->width, pCutFrame->height), IPL_DEPTH_8U, 1);
			pCutFrImg = cvCreateImage(cvSize(pCutFrame->width, pCutFrame->height), IPL_DEPTH_8U, 1);

			pCutBkMat = cvCreateMat(pCutFrame->height, pCutFrame->width, CV_32FC1);
			pCutFrMat = cvCreateMat(pCutFrame->height, pCutFrame->width, CV_32FC1);
			pCutFrameMat = cvCreateMat(pCutFrame->height, pCutFrame->width, CV_32FC1);
			//转化成单通道图像再处理
			cvCvtColor(pCutFrame, pCutBkImg, CV_BGR2GRAY);
			cvCvtColor(pCutFrame, pCutFrImg, CV_BGR2GRAY);
			//转换成矩阵
			cvConvert(pCutFrImg, pCutFrameMat);
			cvConvert(pCutFrImg, pCutFrMat);
			cvConvert(pCutFrImg, pCutBkMat);
		}
		else{
			//获得剪切图
			cvCopy(pFrame, pCutFrame, 0);
			IplImage*img = pCutFrame;//img 为指向其指针
			namedWindow("example", WINDOW_AUTOSIZE);   //显示输入的图像
			cvShowImage("example", img);
			//cvWaitKey(0);
			IplImage *ImageCut2 = cvCreateImage(cvGetSize(img), 8, 1);  //创建用于Canny变换的图像  
			cvCvtColor(img, ImageCut2, CV_BGR2GRAY); //显示输入图对应的灰度图
			cvShowImage("AfterGRAY", ImageCut2);
			//cvWaitKey(0);
			IplImage *img_thres = cvCreateImage(cvGetSize(ImageCut2), 8, 1);
			cvSmooth(ImageCut2, ImageCut2, CV_GAUSSIAN, 3, 3, 0, 0);//高斯模糊平滑处理 
			cvCanny(ImageCut2, img_thres, 100, 200);
			//有的视频使用模糊处理后对直线检测更好 
			cvShowImage("AfterCanny", img_thres);
			//cvWaitKey(0);
			CvMemStorage *storage = cvCreateMemStorage();//内存块，存储中间变量  
			CvSeq *lines = 0; //存储Hough变换 所得结果
			lines = cvHoughLines2(img_thres, storage, CV_HOUGH_PROBABILISTIC, 1, CV_PI / 360, 75, 4, 10);
			printf("Lines number: %d\n", lines->total);  //显示检测到的直线的总数量
			double kmax[2] = { 0.1, 0.1 };          //根据Hough变换后所得线段的斜率筛选出条件合适的，//也即斜率正的最大，和负的最小的两个
			for (int i = 0; i<lines->total; i++)
			{
				double k = 0.1;
				CvPoint *line = (CvPoint *)cvGetSeqElem(lines, i); //line包含两个点line[0]和//line[1]  
				if (line[0].x - line[1].x != 0) k = (double)(line[0].y - line[1].y) / (double)(line[0].x - line[1].x);
				if (k>kmax[0])
				{
					kmax[0] = k; //先找到斜率最大的那条直线
				}
			}
			for (int i = 0; i<lines->total; i++)     //随后找到斜率负最小的那条直线
			{
				double k = 0.1;
				CvPoint *line = (CvPoint *)cvGetSeqElem(lines, i);//line包含两个点line[0]和line[1]  
				if (line[0].x - line[1].x != 0)
				{
					k = (double)(line[0].y - line[1].y) / (double)(line[0].x - line[1].x);
				}
				if (abs(k)>abs(kmax[1]) && k<0)
				{
					kmax[1] = k;
				}
			}
			for (int i = 0; i<lines->total; i++)
			{
				double k = INF;//初始化斜率为无限大  
				CvPoint *line = (CvPoint *)cvGetSeqElem(lines, i);//line包含两个点line[0]和line[1]  
				if (line[0].x - line[1].x != 0) k = (double)(line[0].y - line[1].y) / (double)(line[0].x - line[1].x);
				printf("x1: %d,  y1: %d,  x2: %d,  y2: %d\n", line[0].x, line[0].y, line[1].x, line[1].y);
				printf("k: %lf\n\n", k);
				if ((k <= -0.15 || k >= 0.15) && (k == kmax[0] || k == kmax[1])) //显示这两条直线，即为//车道线
				{
					cvLine(img, line[0], line[1], CV_RGB(0, 255, 0), 2, CV_AA);
					if (k>2 || k<-2)
					{
						printf("yaxian\n"); //若检测出的直线的斜率超过一//定阈值，车即将越线，做出警示
					}
				}
				cvLine(img, line[0], line[1], CV_RGB(0, 255, 0), 2, CV_AA);
				//因为cvLine绘图只有图是3通道图时才能显示线的颜色，所以用ImageIPM作为绘线的地图  
				//第二三个参数为线的起点终点，第四个为四射，第五个为线的粗细  
			}
			//cvMoveWindow("result", 300, 30);
			namedWindow("result", WINDOW_AUTOSIZE); //显示结果图
			cvShowImage("result", img);
			cvReleaseImage(&ImageCut2);   //释放存储中间结果的内存块
			cvReleaseImage(&img_thres);   //释放存储结果的内存块
			//按键事件，空格暂停，其他跳出循环
			int temp = cvWaitKey(2);
			if (temp == 32){
				while (cvWaitKey() == -1);
			}
			else if (temp >= 0){
				break;
			}
		}
		//恢复ROI区域（多余可去掉）
		cvResetImageROI(pFrame);
	}
	//释放图像和矩阵
	cvReleaseImage(&pCutFrImg);
	cvReleaseImage(&pCutBkImg);
	cvReleaseImage(&pCutFrame);
	cvReleaseMat(&pCutFrameMat);
	cvReleaseMat(&pCutFrMat);
	cvReleaseMat(&pCutBkMat);
	cvReleaseCapture(&pCapture);

	return 0;
}