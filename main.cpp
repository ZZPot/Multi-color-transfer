#include "Transfer.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#pragma warning(disable: 4244)

using namespace cv;


#define MAX_WEIGHT_VAL	10
#define WND_ROW			4
#define FRAMES_WIDTH	9
#define TOOLBAR_HEIGHT	40

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

std::vector<std::string> images =	{	
									"images/1.jpg",
								//	"images/2.jpg", 
								//	"images/3.jpg", 
									"images/4.jpg", 
								//	"images/5.jpg"
									};
cv::String wnd_result = "Result";
cv::String wnd_original = "Original";

void CreateWindowIT(img_trans& it, tb_param* param, int width, int height, cv::String& wnd_name);
void GetWindowsSize(int* width, int* height, unsigned count = 3);
void InitRefParam(refresh_param& ref_param);
void InitTBParams(refresh_param& ref_param, std::vector<tb_param>& tb_params);

int main()
{
	refresh_param ref_param;
	InitRefParam(ref_param);
	std::vector<tb_param> tb_params;
	InitTBParams(ref_param, tb_params);

	// Creating windows
	int wnd_w, wnd_h;
	GetWindowsSize(&wnd_w, &wnd_h, WND_ROW);
	for(unsigned i = 0; i < ref_param.layers.size(); i++)
	{
		CreateWindowIT(ref_param.layers[i], &tb_params[i], wnd_w, wnd_h, ref_param.layers[i].file_name);
		moveWindow(ref_param.layers[i].file_name,
			(wnd_w + FRAMES_WIDTH) * (i % WND_ROW),
			(wnd_h + TOOLBAR_HEIGHT) * (i / WND_ROW));
	}
	tb_param tb_source = {&ref_param.source, 0, {"W", "W", "W"}, &ref_param};
	CreateWindowIT(ref_param.source, &tb_source, wnd_w, wnd_h, wnd_original);
	int wnd_top = (ref_param.layers.size() + WND_ROW - 1) / WND_ROW  * (wnd_h + TOOLBAR_HEIGHT);
	moveWindow(wnd_original, 0, wnd_top);
	namedWindow(wnd_result);
	moveWindow(wnd_result, wnd_w + FRAMES_WIDTH, wnd_top);

	// convert source to lab
	ref_param.source.img = BGRtoLab(ref_param.source.img);

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
void CreateWindowIT(img_trans& it, tb_param* param, int width, int height, cv::String& wnd_name)
{
	namedWindow(wnd_name, WINDOW_KEEPRATIO);
	resizeWindow(wnd_name, width, height);
	imshow(wnd_name, it.img);
	int init_val = it.channel_w[0];
	createTrackbar(param->tb_name[0], wnd_name, &init_val, MAX_WEIGHT_VAL, OnTrackBarChanged, param); // for testing
}
void GetWindowsSize(int* width, int* height, unsigned count)
{
	*width = 400;
	*height = 200;
	*width += FRAMES_WIDTH;
	*height += TOOLBAR_HEIGHT;
}
void InitRefParam(refresh_param& ref_param)
{
	for(auto image : images)
	{
		ref_param.layers.push_back(img_trans(image));
	}
	ref_param.source.SetImg("source/1.jpg");
	ref_param.res_wnd_name = wnd_result;
	ref_param.layers[0].SetWeight(MAX_WEIGHT_VAL);
}
void InitTBParams(refresh_param& ref_param, std::vector<tb_param>& tb_params)
{
	for(unsigned i = 0; i < ref_param.layers.size(); i++)
	{
		tb_param temp = {&ref_param.layers[i], 0, {"W", "W", "W"}, &ref_param};
		tb_params.push_back(temp);
	}
}