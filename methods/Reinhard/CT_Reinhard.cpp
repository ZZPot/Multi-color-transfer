#include "CT_Reinhard.h"
#include <opencv2/imgproc.hpp>
#include <stdio.h>
/*Reinhard's method*/

#pragma region CONVERT MATRICES
cv::Mat RGB_to_LMS = (cv::Mat_<float>(3,3) <<	0.3811f, 0.5783f, 0.0402f,
												0.1967f, 0.7244f, 0.0782f,
												0.0241f, 0.1288f, 0.8444f);


cv::Mat LMS_to_RGB = (cv::Mat_<float>(3,3) <<	4.4679f, -3.5873f, 0.1193f,
												-1.2186f, 2.3809f, -0.1624f,
												0.0497f, -0.2439f, 1.2045f);
float _x = 1/sqrt(3), _y = 1/sqrt(6), _z = 1/sqrt(2);
cv::Mat LMS_to_lab = (cv::Mat_<float>(3,3) <<	_x, _x, _x,
												_y, _y, -2*_y,
												_z, -_z, 0);
cv::Mat lab_to_LMS = (cv::Mat_<float>(3,3) <<	_x, _y, _z,
												_x, _y, -_z,
												_x, -2*_y, 0);
#pragma endregion

#define PER_CHANNEL
//#define ESCAPE_ZEROS_RGB

cv::Mat Reinhard(img_trans& source, std::map<unsigned, img_trans*>& layers)
{
	/* 
		Once converted it remains in
		that color space until other
		method convert it in it's own.
		It saves time when color 
		transfer applied many times.
	*/
	source.ConvertTo(CS_Lalphabeta);
	cv::Mat res;

	cv::Scalar divider = CalcDivider(source, layers);
#ifdef PER_CHANNEL
	std::vector<cv::Mat> res_channels, lab_channels;
	cv::split(source.img, res_channels);
	cv::split(source.img, lab_channels);
	cv::Mat source_mean = source.params[METHOD_REINHARD]->GetParam(REINHARD_MEAN);
	// init
	cv::Scalar src_stdd(0); 
	cv::Scalar src_mean(0); 
	for(unsigned i = 0; i < IT_CHANNELS; i++)
	{
		res_channels[i] *= source.channel_w[i] * divider[i];
		lab_channels[i] -= source.params[METHOD_REINHARD]->GetParam(REINHARD_MEAN).at<double>(i); // same for all layers
		src_stdd[i] = source.params[METHOD_REINHARD]->GetParam(REINHARD_STDD).at<double>(i); // to not to call it every time in loop
	}

	// add
	for(auto& layer :  layers)
	{	
		for(unsigned i = 0; i < IT_CHANNELS; i++)
		{
			double layer_weight = layer.second->channel_w[i] * divider[i];
			double stdd_koef =	layer.second->params[METHOD_REINHARD]->GetParam(REINHARD_STDD).at<double>(i) / src_stdd[i];
			res_channels[i] += (lab_channels[i] * stdd_koef + layer.second->params[METHOD_REINHARD]->GetParam(REINHARD_MEAN).at<double>(i)) * layer_weight;
		}
	}
	cv::merge(res_channels, res);
#else

	cv::multiply(source.img, source.channel_w.mul(divider), res);
	cv::Scalar	mean_src(0, 0, 0), stdd_src(0, 0, 0);
	for(unsigned i = 0; i < IT_CHANNELS; i++)
	{
		mean_src[i] = source.params[METHOD_REINHARD]->GetParam(REINHARD_MEAN).at<double>(i);
		stdd_src[i] = source.params[METHOD_REINHARD]->GetParam(REINHARD_STDD).at<double>(i);
	}
	cv::Mat minus_mean = source.img - mean_src;
	for(auto& layer :  layers)
	{
		cv::Mat temp;
		cv::Scalar	mean_layer(0, 0, 0), koef(0, 0, 0);
		for(unsigned i = 0; i < IT_CHANNELS; i++)
		{
			mean_layer[i] = layer.second->params[METHOD_REINHARD]->GetParam(REINHARD_MEAN).at<double>(i);
			koef[i] = layer.second->params[METHOD_REINHARD]->GetParam(REINHARD_STDD).at<double>(i) / stdd_src[i];
		}
		cv::multiply(minus_mean, koef, temp);
		temp += mean_layer;

		cv::multiply(temp, layer.second->channel_w.mul(divider), temp);
		res += temp;
	}
#endif
	return LabtoBGR(res);
}
cv::Mat BGRtoLab(cv::Mat input)
{
	cv::Mat img_RGB;
	cv::cvtColor(input, img_RGB, cv::COLOR_BGR2RGB);
#ifdef ESCAPE_ZEROS_RGB
	cv::Mat min_mat = cv::Mat::Mat(img_RGB.size(), CV_8UC3, cv::Scalar(1, 1, 1));
	cv::max(img_RGB, min_mat, img_RGB);
#endif
	img_RGB.convertTo(img_RGB, CV_32FC1, 1/255.f);
	cv::Mat img_lms;
	
	cv::transform(img_RGB, img_lms, RGB_to_LMS);
#ifndef ESCAPE_ZEROS_RGB
	cv::Mat min_mat = cv::Mat::Mat(img_RGB.size(), CV_32FC3, cv::Scalar(0.0001, 0.0001, 0.0001));
	cv::max(img_lms, min_mat, img_lms);
#endif
	cv::log(img_lms,img_lms);
	img_lms /= log(10);
	cv::Mat img_lab;
	cv::transform(img_lms, img_lab, LMS_to_lab);
	return img_lab;
}
cv::Mat LabtoBGR(cv::Mat input)
{
	cv::Mat img_lms;
	cv::transform(input, img_lms, lab_to_LMS);
	cv::exp(img_lms,img_lms);
	cv::pow(img_lms,log(10),img_lms);
	cv::Mat img_RGB;
	cv::transform(img_lms, img_RGB, LMS_to_RGB);
	img_RGB.convertTo(img_RGB, CV_8UC1, 255.f);
	cv::Mat img_BGR;
	cv::cvtColor(img_RGB, img_BGR, cv::COLOR_RGB2BGR);
	return img_BGR;
}
void CTP_Reinhard::SetParams(cv::Mat img)
{
	cv::meanStdDev(BGRtoLab(img), mean, stdd);
}
cv::Mat CTP_Reinhard::GetParam(int param)
{
	// no number checking here
	switch(param)
	{
	case REINHARD_MEAN:
		return mean;
	case REINHARD_STDD:
		return stdd;
	}
	return cv::Mat();
}