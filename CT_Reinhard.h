#pragma once
#include "Transfer.h"

#define REINHARD_MEAN	0
#define REINHARD_STDD	1

void GetMeanStdd(cv::Mat img, std::vector<double>& mean, std::vector<double>& stdd); // get mean and stdd

// Reinhard's method
cv::Mat Reinhard(img_trans& source, std::map<unsigned, img_trans*>& layers);
cv::Mat BGRtoLab(cv::Mat input);
cv::Mat LabtoBGR(cv::Mat input);

class CTP_Reinhard: public CTParams
{
public:
	void SetParams(cv::Mat img);
	double GetParam(int param, int number);
	
protected:
	std::vector<double> mean;
	std::vector<double> stdd;
};