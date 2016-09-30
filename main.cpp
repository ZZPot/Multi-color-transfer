#include "Transfer.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>

#pragma warning(disable: 4244)

using namespace cv;

#define TESTING

#define VIDEO
#ifndef VIDEO
	#define SOURCE_PIC "source/1.jpg"
#else
	// SET YOUR VIDEO
	#define SOURCE_VIDEO "../Gojira - To Sirius.mp4" 
#endif
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
	cv::String tb_name;
	refresh_param* ref_param;
};

std::vector<std::string> images =	{	
									"images/1.jpg",
								//	"images/2.jpg", 
									"images/3.jpg", 
								//	"images/4.jpg", 
								//	"images/5.jpg"
									};
cv::String wnd_result = "Result";
cv::String wnd_original = "Original";

void CreateWindowIT(img_trans& it, tb_param** param, int width, int height, cv::String& wnd_name);
void GetWindowsSize(int* width, int* height, unsigned count = 3);
void InitRefParam(refresh_param& ref_param);
void InitTBParams(refresh_param& ref_param, std::vector<tb_param>& tb_params);
void RefreshResult(refresh_param* param);

int main()
{
	refresh_param ref_param;
	InitRefParam(ref_param);
	std::vector<tb_param> tb_layers; // hope it won't relocate / do something with it later
	InitTBParams(ref_param, tb_layers);

	// Creating windows
	int wnd_w, wnd_h;
	GetWindowsSize(&wnd_w, &wnd_h, WND_ROW);
	unsigned iter = 3;
#ifdef TESTING
	iter = 1;
#endif
	tb_param* params[3];
	for(unsigned i = 0; i < ref_param.layers.size(); i+=iter)
	{
#ifdef TESTING
		params[0] = &tb_layers[i];
#else
		params[0] = &tb_layers[i];
		params[1] = &tb_layers[i + 1];
		params[2] = &tb_layers[i + 2];
#endif
		CreateWindowIT(ref_param.layers[i], params, wnd_w, wnd_h, ref_param.layers[i].file_name);
		moveWindow(ref_param.layers[i].file_name,
			(wnd_w + FRAMES_WIDTH) * (i % WND_ROW),
			(wnd_h + TOOLBAR_HEIGHT) * (i / WND_ROW));
	}
#ifdef TESTING
	tb_param tb_source[1] = {&ref_param.source, 0, "W", &ref_param};	
	params[0] = &tb_source[0];
#else
	tb_param tb_source[3] = {
				{&ref_param.source, 0, "W1", &ref_param},
				{&ref_param.source, 1, "W2", &ref_param},
				{&ref_param.source, 2, "W3", &ref_param}
				};
	params[0] = &tb_source[0];
	params[1] = &tb_source[1];
	params[2] = &tb_source[2];
#endif
#ifdef VIDEO
	VideoCapture video;
	video.open(SOURCE_VIDEO);
	if(!video.isOpened())
		return 1;
	Mat frame;
	video >> frame; // get 1st frame
	ref_param.source.SetImg(frame);
	ref_param.source.SetWeight();
	CreateWindowIT(ref_param.source, params, wnd_w, wnd_h, wnd_original);
	int wnd_top = (ref_param.layers.size() + WND_ROW - 1) / WND_ROW  * (wnd_h + TOOLBAR_HEIGHT);
	moveWindow(wnd_original, 0, wnd_top);

	namedWindow(wnd_result);
	moveWindow(wnd_result, wnd_w + FRAMES_WIDTH, wnd_top);

	while(1)
	{
		video >> frame;
		imshow(wnd_original, frame);
		ref_param.source.SetImg(frame);
		
		ref_param.source.img = BGRtoLab(ref_param.source.img);
		RefreshResult(&ref_param);
		if(waitKey(10) >= 0) break;
	}
	video.release();
#else
	CreateWindowIT(ref_param.source, params, wnd_w, wnd_h, wnd_original);
	int wnd_top = (ref_param.layers.size() + WND_ROW - 1) / WND_ROW  * (wnd_h + TOOLBAR_HEIGHT);
	moveWindow(wnd_original, 0, wnd_top);
	namedWindow(wnd_result);
	moveWindow(wnd_result, wnd_w + FRAMES_WIDTH, wnd_top);
	// convert source to lab
	ref_param.source.img = BGRtoLab(ref_param.source.img);
	waitKey(0);
#endif
	return 0;
}
void RefreshResult(refresh_param* param)
{
	imshow(param->res_wnd_name, CTW(param->source, param->layers));
}
void OnTrackBarChanged(int new_pos, void* param)
{
	tb_param* tbp = (tb_param*)param;
#ifdef TESTING
	tbp->it->SetWeight(new_pos); // for testing
#else
	tbp->it->channel_w[tbp->channel] = new_pos; 
#endif
	RefreshResult(tbp->ref_param);
}
void CreateWindowIT(img_trans& it, tb_param** param, int width, int height, cv::String& wnd_name)
{
	namedWindow(wnd_name, WINDOW_KEEPRATIO);
	resizeWindow(wnd_name, width, height); // should be resized before creating trackbar
	imshow(wnd_name, it.img);
	int init_val = it.channel_w[0];
	createTrackbar(param[0]->tb_name, wnd_name, &init_val, MAX_WEIGHT_VAL, OnTrackBarChanged, param[0]); 
#ifndef TESTING
	init_val = it.channel_w[1];
	createTrackbar(param[1]->tb_name, wnd_name, &init_val, MAX_WEIGHT_VAL, OnTrackBarChanged, param[1]);
	init_val = it.channel_w[2];
	createTrackbar(param[2]->tb_name, wnd_name, &init_val, MAX_WEIGHT_VAL, OnTrackBarChanged, param[2]);
#endif
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
	ref_param.res_wnd_name = wnd_result;
#ifdef VIDEO
	
#else
	ref_param.source.SetImg(SOURCE_PIC);
	ref_param.source.SetWeight();
#endif
	ref_param.layers[0].SetWeight(MAX_WEIGHT_VAL);
}
void InitTBParams(refresh_param& ref_param, std::vector<tb_param>& tb_params)
{
	for(unsigned i = 0; i < ref_param.layers.size(); i++)
	{
#ifdef TESTING
		tb_param temp = {&ref_param.layers[i], 0, "W", &ref_param};
		tb_params.push_back(temp);
#else
		tb_param temp[3] = {
							{&ref_param.layers[i], 0, "W1", &ref_param},
							{&ref_param.layers[i], 1, "W2", &ref_param},
							{&ref_param.layers[i], 2, "W3", &ref_param}
							};
		tb_params.push_back(temp[0]);
		tb_params.push_back(temp[1]);
		tb_params.push_back(temp[2]);
#endif
	}
}