#include "CT_Reinhard.h"
#include <opencv2/imgproc.hpp>
#include "Transfer.h"

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

	cv::Scalar divider(0, 0, 0);
	
	for(auto& layer :  layers)
	{
		for(unsigned i = 0; i < IT_CHANNELS; i++)
		{
			if(layer.second->channel_w[i] < 0)
				continue;
			divider[i] += layer.second->channel_w[i];
		}
	}
	for(unsigned i = 0; i < IT_CHANNELS; i++)
	{
		if(source.channel_w[i] < 0)
			continue;
		divider[i] += source.channel_w[i];
		divider[i] = 1 / divider[i];
	}

#ifdef PER_CHANNEL
	std::vector<cv::Mat> res_channels, lab_channels;
	cv::split(source.img, res_channels);
	cv::split(source.img, lab_channels);
	// init
	for(unsigned i = 0; i < IT_CHANNELS; i++)
		res_channels[i] *= source.channel_w[i] * divider[i];
	// add
	for(auto& layer :  layers)
	{	for(unsigned i = 0; i < IT_CHANNELS; i++)
		{
			double layer_weight = layer.second->channel_w[i] * divider[i];
			double stdd_koef =	layer.second->params[METHOD_REINHARD]->GetParam(REINHARD_STDD, i) / 
								source.params[METHOD_REINHARD]->GetParam(REINHARD_STDD, i);
			res_channels[i] += 
				((lab_channels[i] - source.params[METHOD_REINHARD]->GetParam(REINHARD_MEAN, i))
				* stdd_koef + layer.second->params[METHOD_REINHARD]->GetParam(REINHARD_MEAN, i)) * layer_weight;
		}
	}
	cv::merge(res_channels, res);
#else

	cv::multiply(source.img, source.channel_w.mul(divider), res);
	cv::Scalar	mean_src(0, 0, 0), stdd_src(0, 0, 0);
	for(unsigned i = 0; i < IT_CHANNELS; i++)
	{
		mean_src[i] = source.params[METHOD_REINHARD]->GetParam(REINHARD_MEAN, i);
		stdd_src[i] = source.params[METHOD_REINHARD]->GetParam(REINHARD_STDD, i);
	}
	cv::Mat minus_mean = source.img - mean_src;
	for(auto& layer :  layers)
	{
		cv::Mat temp;
		cv::Scalar	mean_layer(0, 0, 0), koef(0, 0, 0);
		for(unsigned i = 0; i < IT_CHANNELS; i++)
		{
			mean_layer[i] = layer.second->params[METHOD_REINHARD]->GetParam(REINHARD_MEAN, i);
			koef[i] = layer.second->params[METHOD_REINHARD]->GetParam(REINHARD_STDD, i) / stdd_src[i];
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
	cv::cvtColor(input, img_RGB, CV_BGR2RGB);
	cv::Mat min_mat = cv::Mat::Mat(img_RGB.size(), CV_8UC3, cv::Scalar(1, 1, 1));
	cv::max(img_RGB, min_mat, img_RGB);
	img_RGB.convertTo(img_RGB, CV_32FC1, 1/255.f);
	cv::Mat img_lms;
	cv::transform(img_RGB, img_lms, RGB_to_LMS);
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
	cv::cvtColor(img_RGB, img_BGR, CV_RGB2BGR);
	return img_BGR;
}
void GetMeanStdd(cv::Mat img, std::vector<double>& mean, std::vector<double>& stdd)
{
	cv::Mat _mean, _stdd;
	cv::meanStdDev(img, _mean, _stdd);
	mean.resize(IT_CHANNELS);
	stdd.resize(IT_CHANNELS);
	for(unsigned i = 0; i < IT_CHANNELS; i++)
	{
		mean[i] = _mean.at<double>(i);
		stdd[i] = _stdd.at<double>(i);
	}
}
void CTP_Reinhard::SetParams(cv::Mat img)
{
	GetMeanStdd(BGRtoLab(img), mean, stdd);
}
double CTP_Reinhard::GetParam(int param, int number)
{
	// no number checking here
	switch(param)
	{
	case REINHARD_MEAN:
		return mean[number];
	case REINHARD_STDD:
		return stdd[number];
	}
	return 0;
}