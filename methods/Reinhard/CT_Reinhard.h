#pragma once
#include "../../Transfer.h"

#define REINHARD_MEAN	0
#define REINHARD_STDD	1

// Reinhard's method
cv::Mat Reinhard(img_trans& source, std::map<unsigned, img_trans*>& layers);
cv::Mat BGRtoLab(cv::Mat input);
cv::Mat LabtoBGR(cv::Mat input);

class CTP_Reinhard: public CTParams
{
public:
	void SetParams(cv::Mat img);
	cv::Mat GetParam(int param);
protected:
	cv::Mat mean;
	cv::Mat stdd;
};