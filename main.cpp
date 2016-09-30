#include "Transfer.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#pragma warning(disable: 4244)

using namespace cv;

#define WND_RESULT "Result"
#define MAX_WEIGHT_VAL	10

std::vector<std::string> images = {	"images/1.jpg",
									"images/2.jpg", 
								//	"images/3.jpg", 
								//	"images/4.jpg", 
								//	"images/5.jpg"
									};

// parameter that contains all refresh_function arguments
struct refresh_param
{
	std::string res_wnd_name;
	img_trans	source;
	std::vector<img_trans> layers;
};

// trackbar param
struct tb_param
{
	img_trans* it;
	unsigned channel;
	std::vector<cv::String> tb_name;
	refresh_param* ref_param;
};

void CreateWindowIT(img_trans& it, tb_param* param, std::string wnd_name = "");

int main()
{
	refresh_param ref_param;
	for(auto image : images)
	{
		ref_param.layers.push_back(img_trans(image));
	}
	ref_param.source.SetImg("source/1.jpg");
	ref_param.res_wnd_name = WND_RESULT;

	ref_param.source.SetWeight(1);
	ref_param.layers[0].SetWeight(100);
//	ref_param.layers[1].SetWeight(100);
//	ref_param.layers[2].SetWeight(50);

	/*
		Creating windows here
	*/
	std::vector<tb_param> params;
	for(unsigned i = 0; i < ref_param.layers.size(); i++)
	{
		tb_param temp = {&ref_param.layers[i], 0, {"W", "W", "W"}, &ref_param};
		params.push_back(temp);
	}
	for(unsigned i = 0; i < ref_param.layers.size(); i++)
	{
		CreateWindowIT(ref_param.layers[i], &params[i]);
	}
	ref_param.source.img = BGRtoLab(ref_param.source.img); // convert to lab for speed up

	waitKey(0);
	return 0;
}
void RefreshResult(refresh_param* param)
{
	imshow(param->res_wnd_name, CTW(param->source, param->layers));
}
void OnTrackBarChanged(int new_pos, void* param)
{
	tb_param* tbp = (tb_param*)param;
	tbp->it->channel_w[tbp->channel] = new_pos; 
	tbp->it->SetWeight(new_pos); // for testing
	RefreshResult(tbp->ref_param);
}
void CreateWindowIT(img_trans& it, tb_param* param, std::string wnd_name)
{
	if(!wnd_name.length())
		wnd_name = it.file_name;
	namedWindow(wnd_name, WINDOW_AUTOSIZE);
	imshow(wnd_name, it.img);
	int init_val = it.channel_w[0];
	createTrackbar(param->tb_name[0], wnd_name, &init_val, MAX_WEIGHT_VAL, OnTrackBarChanged, param); // for testing
}