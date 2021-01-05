#pragma once
#include "../../Transfer.h"

#define XIAO_LEFT		0		// TRS matrices
#define XIAO_RIGHT		1		// SRT matrices

// Xiao's method
cv::Mat Xiao(img_trans& source, std::map<unsigned, img_trans*>& layers);
void GetTRS(cv::Mat input, cv::Mat& T, cv::Mat& R, cv::Mat& S);
void GetSRT(cv::Mat input, cv::Mat& T, cv::Mat& R, cv::Mat& S);
cv::Mat AddChannel(cv::Mat mat);
cv::Mat RemoveChannel(cv::Mat mat);
cv::Mat ScalarMultiple(cv::Mat img, cv::Scalar value);

class CTP_Xiao: public CTParams // it stores both left and right parts of T*R*S*S*R*T
{
public:
	void SetParams(cv::Mat img);
	cv::Mat GetParam(int param);
protected:
	cv::Mat TRS;
	cv::Mat SRT;
};