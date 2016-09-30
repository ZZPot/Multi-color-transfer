#pragma once
#include <string>
#include <vector>
#include <opencv2/core.hpp>


#define IT_CHANNELS	3

struct img_trans
{
	img_trans(std::string fname);
	img_trans(cv::Mat image);
	img_trans();
	void SetImg(std::string fname);
	void SetImg(cv::Mat image);
	void SetWeight(double weight = 0);
	std::string file_name;
	cv::Mat img; // source img in lab color space // others in BGR
	std::vector<double> channel_w; // please, between 0 and 100.
	std::vector<double> mean;
	std::vector<double> stdd;
};

cv::Mat CTW(img_trans& input, std::vector<img_trans> layers); // main work

void GetImageParams(cv::Mat& img, std::vector<double>& mean, std::vector<double>& stdd); // get mean and stdd

cv::Mat BGRtoLab(cv::Mat input);
cv::Mat LabtoBGR(cv::Mat input);
