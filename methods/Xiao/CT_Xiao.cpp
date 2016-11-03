#include "CT_Xiao.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <conio.h>
/*Xiao's method*/


cv::Mat Xiao(img_trans& source, std::map<unsigned, img_trans*>& layers)
{
	cv::Mat res;
	cv::Scalar divider = CalcDivider(source, layers);

	// init
	cv::Mat src1;
	source.img.convertTo(src1, CV_64FC3, 1 / 255.0);
	src1 = AddChannel(src1);
	cv::Mat SRT = source.params[METHOD_XIAO]->GetParam(XIAO_RIGHT);
	cv::multiply(source.img, source.channel_w.mul(divider), res);
	// add
	for(auto& layer :  layers)
	{	
		cv::Mat mega = layer.second->params[METHOD_XIAO]->GetParam(XIAO_LEFT) * SRT;
		cv::Mat temp_res;
		transform(src1, temp_res, mega);
		temp_res = RemoveChannel(temp_res);
		temp_res.convertTo(temp_res, CV_8UC3, 255);
		cv::multiply(temp_res, layer.second->channel_w.mul(divider), temp_res);
		res += temp_res;
	}
	return res;
}
void CTP_Xiao::SetParams(cv::Mat img)
{
	cv::Mat temp;
	img.convertTo(temp, CV_64FC3, 1 / 255.0);
	cv::Mat src_T, src_R, src_S;
	cv::Mat tar_T, tar_R, tar_S;
	GetTRS(temp, src_T, src_R, src_S);
	GetSRT(temp, tar_T, tar_R, tar_S);
	TRS = src_T * src_R * src_S;
	SRT = tar_S * tar_R * tar_T;
}
cv::Mat CTP_Xiao::GetParam(int param)
{
	switch(param)
	{
	case XIAO_LEFT:
		return TRS;
	case XIAO_RIGHT:
		return SRT;
	}
	return cv::Mat();
}
void GetTRS(cv::Mat input, cv::Mat& T, cv::Mat& R, cv::Mat& S)
{
	cv::Mat cov, means;
	cv::calcCovarMatrix(input.reshape(1, input.cols * input.rows), cov, means, CV_COVAR_NORMAL | CV_COVAR_ROWS, CV_64F);
	cv::Mat U, A, VT;
	cv::SVD::compute(cov, A, U, VT);
	T = cv::Mat::eye(4, 4, CV_64FC1);
	R = cv::Mat::eye(4, 4, CV_64FC1);
	S = cv::Mat::eye(4, 4, CV_64FC1);

	cv::Rect roi(0, 0, 3, 3);
	U.copyTo(R(roi));
	
	T.at<double>(0, 3) = means.at<double>(0, 0);
	T.at<double>(1, 3) = means.at<double>(0, 1);
	T.at<double>(2, 3) = means.at<double>(0, 2);

	// in original paper there is no sqrt()
	S.at<double>(0, 0) = sqrt(A.at<double>(0, 0));
	S.at<double>(1, 1) = sqrt(A.at<double>(1, 0));
	S.at<double>(2, 2) = sqrt(A.at<double>(2, 0));
}
void GetSRT(cv::Mat input, cv::Mat& T, cv::Mat& R, cv::Mat& S)
{
	cv::Mat cov, means;
	cv::calcCovarMatrix(input.reshape(1, input.cols * input.rows), cov, means, CV_COVAR_NORMAL | CV_COVAR_ROWS, CV_64F);
	cv::Mat U, A, VT;
	cv::SVD::compute(cov, A, U, VT);
	T = cv::Mat::eye(4, 4, CV_64FC1);
	R = cv::Mat::eye(4, 4, CV_64FC1);
	S = cv::Mat::eye(4, 4, CV_64FC1);
	cv::Rect roi(0, 0, 3, 3);
	cv::invert(U, R(roi));

	T.at<double>(0, 3) = -means.at<double>(0, 0);
	T.at<double>(1, 3) = -means.at<double>(0, 1);
	T.at<double>(2, 3) = -means.at<double>(0, 2);

	S.at<double>(0, 0) = 1/sqrt(A.at<double>(0, 0));
	S.at<double>(1, 1) = 1/sqrt(A.at<double>(1, 0));
	S.at<double>(2, 2) = 1/sqrt(A.at<double>(2, 0));
}
cv::Mat AddChannel(cv::Mat mat)
{
	cv::Mat img = cv::Mat::ones(mat.size(), CV_64FC1);
	std::vector<cv::Mat> channels;
	cv::split(mat, channels);
	channels.push_back(img);
	cv::merge(channels, img);
	return img; 
}
cv::Mat RemoveChannel(cv::Mat mat)
{
	std::vector<cv::Mat> channels;
	cv::split(mat, channels);
	channels.resize(3);
	cv::Mat img;
	cv::merge(channels, img);
	return img;
}
cv::Mat ScalarMultiple(cv::Mat img, cv::Scalar value)
{
	std::vector<cv::Mat> img_channels;
	cv::split(img, img_channels);
	for(unsigned i = 0; i < img_channels.size(); i++)
		img_channels[i] *= value[i];
	cv::Mat res;
	cv::merge(img_channels, res);
	return res;
}