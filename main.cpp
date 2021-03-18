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
	//����IplImageָ��
	IplImage* pFrame = NULL;
	IplImage* pCutFrame = NULL;
	IplImage* pCutFrImg = NULL;
	IplImage* pCutBkImg = NULL;
	//����CvMatָ��
	CvMat* pCutFrameMat = NULL;
	CvMat* pCutFrMat = NULL;
	CvMat* pCutBkMat = NULL;
	//����CvCaptureָ��
	CvCapture* pCapture = NULL;
	//����CvMemStorage��CvSegָ��
	CvMemStorage* storage = cvCreateMemStorage();
	CvSeq* lines = NULL;
	//��ǰ֡��
	int nFrmNum = 0;
	//�ü�����ո߶�
	int CutHeight = 225;
	//���ܴ����˳�
	if (!(pCapture = cvCaptureFromFile("./1.avi"))){
		fprintf(stderr, "Can not open video file\n");
		return -2;
	}
	//ÿ�ζ�ȡһ�����Ƶ
	while (pFrame = cvQueryFrame(pCapture)){
		//��ʾ�����ͼ��
		//����ROI�ü�ͼ��
		cvSetImageROI(pFrame, cvRect(0, CutHeight, pFrame->width, pFrame->height - CutHeight));
		nFrmNum++;
		//��һ��Ҫ�����ڴ�p
		if (nFrmNum == 1){
			pCutFrame = cvCreateImage(cvSize(pFrame->width, pFrame->height - CutHeight), pFrame->depth, pFrame->nChannels);
			cvCopy(pFrame, pCutFrame, 0);
			pCutBkImg = cvCreateImage(cvSize(pCutFrame->width, pCutFrame->height), IPL_DEPTH_8U, 1);
			pCutFrImg = cvCreateImage(cvSize(pCutFrame->width, pCutFrame->height), IPL_DEPTH_8U, 1);

			pCutBkMat = cvCreateMat(pCutFrame->height, pCutFrame->width, CV_32FC1);
			pCutFrMat = cvCreateMat(pCutFrame->height, pCutFrame->width, CV_32FC1);
			pCutFrameMat = cvCreateMat(pCutFrame->height, pCutFrame->width, CV_32FC1);
			//ת���ɵ�ͨ��ͼ���ٴ���
			cvCvtColor(pCutFrame, pCutBkImg, CV_BGR2GRAY);
			cvCvtColor(pCutFrame, pCutFrImg, CV_BGR2GRAY);
			//ת���ɾ���
			cvConvert(pCutFrImg, pCutFrameMat);
			cvConvert(pCutFrImg, pCutFrMat);
			cvConvert(pCutFrImg, pCutBkMat);
		}
		else{
			//��ü���ͼ
			cvCopy(pFrame, pCutFrame, 0);
			IplImage*img = pCutFrame;//img Ϊָ����ָ��
			namedWindow("example", WINDOW_AUTOSIZE);   //��ʾ�����ͼ��
			cvShowImage("example", img);
			//cvWaitKey(0);
			IplImage *ImageCut2 = cvCreateImage(cvGetSize(img), 8, 1);  //��������Canny�任��ͼ��  
			cvCvtColor(img, ImageCut2, CV_BGR2GRAY); //��ʾ����ͼ��Ӧ�ĻҶ�ͼ
			cvShowImage("AfterGRAY", ImageCut2);
			//cvWaitKey(0);
			IplImage *img_thres = cvCreateImage(cvGetSize(ImageCut2), 8, 1);
			cvSmooth(ImageCut2, ImageCut2, CV_GAUSSIAN, 3, 3, 0, 0);//��˹ģ��ƽ������ 
			cvCanny(ImageCut2, img_thres, 100, 200);
			//�е���Ƶʹ��ģ��������ֱ�߼����� 
			cvShowImage("AfterCanny", img_thres);
			//cvWaitKey(0);
			CvMemStorage *storage = cvCreateMemStorage();//�ڴ�飬�洢�м����  
			CvSeq *lines = 0; //�洢Hough�任 ���ý��
			lines = cvHoughLines2(img_thres, storage, CV_HOUGH_PROBABILISTIC, 1, CV_PI / 360, 75, 4, 10);
			printf("Lines number: %d\n", lines->total);  //��ʾ��⵽��ֱ�ߵ�������
			double kmax[2] = { 0.1, 0.1 };          //����Hough�任�������߶ε�б��ɸѡ���������ʵģ�//Ҳ��б��������󣬺͸�����С������
			for (int i = 0; i<lines->total; i++)
			{
				double k = 0.1;
				CvPoint *line = (CvPoint *)cvGetSeqElem(lines, i); //line����������line[0]��//line[1]  
				if (line[0].x - line[1].x != 0) k = (double)(line[0].y - line[1].y) / (double)(line[0].x - line[1].x);
				if (k>kmax[0])
				{
					kmax[0] = k; //���ҵ�б����������ֱ��
				}
			}
			for (int i = 0; i<lines->total; i++)     //����ҵ�б�ʸ���С������ֱ��
			{
				double k = 0.1;
				CvPoint *line = (CvPoint *)cvGetSeqElem(lines, i);//line����������line[0]��line[1]  
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
				double k = INF;//��ʼ��б��Ϊ���޴�  
				CvPoint *line = (CvPoint *)cvGetSeqElem(lines, i);//line����������line[0]��line[1]  
				if (line[0].x - line[1].x != 0) k = (double)(line[0].y - line[1].y) / (double)(line[0].x - line[1].x);
				printf("x1: %d,  y1: %d,  x2: %d,  y2: %d\n", line[0].x, line[0].y, line[1].x, line[1].y);
				printf("k: %lf\n\n", k);
				if ((k <= -0.15 || k >= 0.15) && (k == kmax[0] || k == kmax[1])) //��ʾ������ֱ�ߣ���Ϊ//������
				{
					cvLine(img, line[0], line[1], CV_RGB(0, 255, 0), 2, CV_AA);
					if (k>2 || k<-2)
					{
						printf("yaxian\n"); //��������ֱ�ߵ�б�ʳ���һ//����ֵ��������Խ�ߣ�������ʾ
					}
				}
				cvLine(img, line[0], line[1], CV_RGB(0, 255, 0), 2, CV_AA);
				//��ΪcvLine��ͼֻ��ͼ��3ͨ��ͼʱ������ʾ�ߵ���ɫ��������ImageIPM��Ϊ���ߵĵ�ͼ  
				//�ڶ���������Ϊ�ߵ�����յ㣬���ĸ�Ϊ���䣬�����Ϊ�ߵĴ�ϸ  
			}
			//cvMoveWindow("result", 300, 30);
			namedWindow("result", WINDOW_AUTOSIZE); //��ʾ���ͼ
			cvShowImage("result", img);
			cvReleaseImage(&ImageCut2);   //�ͷŴ洢�м������ڴ��
			cvReleaseImage(&img_thres);   //�ͷŴ洢������ڴ��
			//�����¼����ո���ͣ����������ѭ��
			int temp = cvWaitKey(2);
			if (temp == 32){
				while (cvWaitKey() == -1);
			}
			else if (temp >= 0){
				break;
			}
		}
		//�ָ�ROI���򣨶����ȥ����
		cvResetImageROI(pFrame);
	}
	//�ͷ�ͼ��;���
	cvReleaseImage(&pCutFrImg);
	cvReleaseImage(&pCutBkImg);
	cvReleaseImage(&pCutFrame);
	cvReleaseMat(&pCutFrameMat);
	cvReleaseMat(&pCutFrMat);
	cvReleaseMat(&pCutBkMat);
	cvReleaseCapture(&pCapture);

	return 0;
}