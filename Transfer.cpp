#define _CRT_RAND_S
#include "Transfer.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <stdio.h>
#include <stdlib.h>
#include "methods/Reinhard/CT_Reinhard.h"
#include "methods/Xiao/CT_Xiao.h"

#pragma warning(disable: 4244 4267 4996)

std::string tb_names[3] = {"W1", "W2", "W3"};

img_trans::img_trans(std::string fname): channel_w(0, 0, 0)
{
	SetImg(fname);
}
img_trans::img_trans(cv::Mat image): channel_w(0, 0, 0)
{
	SetImg(image);
}
img_trans::img_trans(): channel_w(0, 0, 0)
{
	current_cs = CS_UNDEFINED;
}
void img_trans::SetImg(std::string fname)
{
	name = fname;
	img = cv::imread(fname);
	if(!img.channels())
		return;
	current_cs = CS_BGR;
	for(auto& param: params)
	{
		delete param.second;
	}
	params.clear();
}
void img_trans::SetImg(cv::Mat image)
{
	img = image.clone();
	current_cs = CS_BGR;
	for(auto& param: params)
	{
		delete param.second;
	}
	params.clear();
}
void img_trans::SetWeight(double weight)
{
	channel_w[0] = weight;
	channel_w[1] = weight;
	channel_w[2] = weight;
}
color_space img_trans::ConvertTo(color_space cs)
{
	if(cs == current_cs)
		return current_cs;
	if((cs == color_space::CS_UNDEFINED) || (current_cs == CS_UNDEFINED))
		return CS_UNDEFINED;
	switch(cs)
	{
	case CS_RGB:
		switch (current_cs)
		{
		case CS_BGR:
			cv::cvtColor(img, img, CV_BGR2RGB);
			break;
		case CS_Lalphabeta:
			cv::cvtColor(img, LabtoBGR(img), CV_BGR2RGB);
			break;
		default:
			return CS_UNDEFINED;
		}
		break;
	case CS_BGR:
		switch (current_cs)
		{
		case CS_RGB:
			cv::cvtColor(img, img, CV_RGB2BGR);
			break;
		case CS_Lalphabeta:
			img = LabtoBGR(img);
			break;
		default:
			return CS_UNDEFINED;
		}
		break;
	case CS_Lalphabeta:
		switch (current_cs)
		{
		case CS_RGB:
			cv::cvtColor(img, img, CV_RGB2BGR);
			img = BGRtoLab(img);
			break;
		case CS_BGR:
			img = BGRtoLab(img);
			break;
		default:
			return CS_UNDEFINED;
		}
		break;
	default:
		return CS_UNDEFINED;
	}
	color_space old_cs = current_cs;
	current_cs = cs;
	return old_cs;
}
void img_trans::AddParams(transfer_method method) // created by new
{
	if(params.find(method) != params.end())
		return;
	params[method] = GetCTP(method);
	ConvertTo(CS_BGR);
	params[method]->SetParams(img);
}
cv::Mat CTW(img_trans& source, std::map<unsigned, img_trans*>& layers, transfer_method method)
{
	/*
		Sum of all layers and input wights
		per same channel prefer to be 100.
		It's not limit, it can be 100 in input
		and sum 100 in all layers, but result
		percentage then will be 50% for input 
		and 50% for all layers (respectivly
		to their weights). Negative == 0
	*/
	// Please, return in BGR
	switch (method)
	{
	case METHOD_NONE:
		source.ConvertTo(CS_BGR);
		return source.img.clone();
	case METHOD_REINHARD:
		return Reinhard(source, layers);
	case METHOD_XIAO:
		return Xiao(source, layers);
	}
	return cv::Mat();
}
ColorMachine::ColorMachine(std::string name)
{
	if(name.size())
	{
		_name = "_";
		_name += name;
	}
	else
		_name = CreateRandomName(WND_RAND_NUM, "_"); // result will be like "_123"
	_wnd_original = WND_ORIGINAL + _name; // "Original_name"
	_wnd_result = WND_RESULT + _name; // "Result_name"
	_show = false;
	_current_method = METHOD_NONE;
	_source.SetWeight(1);
	for(unsigned i = 0; i < TB_COUNT; i++)
	{
		_source.tb_params[i].it = &_source;
		_source.tb_params[i].channel = i;
		_source.tb_params[i].tb_name = tb_names[i];
		_source.tb_params[i].cmachine = this;
	}
	_next_id = 1;
}
ColorMachine::~ColorMachine()
{
	for(auto layer: _layers)
	{
		ShowWindows(false);
		delete layer.second;
	}
}
unsigned ColorMachine::AddLayer(cv::Mat layer, std::string name)
{
	img_trans* temp = new img_trans(layer);
	if(name.size())
	{
		name = WND_LAYER + _name + "_";
		name = CreateRandomName(WND_RAND_NUM, name); // "Layer_name_456"
	}
	else
		name += _name; // "SpecialImage_name"
	temp->name = name;
	for(unsigned i = 0; i < TB_COUNT; i++)
	{
		temp->tb_params[i].it = temp;
		temp->tb_params[i].channel = i;
		temp->tb_params[i].tb_name = tb_names[i];
		temp->tb_params[i].cmachine = this;
	}
	_layers[_next_id] = temp;
	if(_show)
	{
		int wnd_w, wnd_h;
		GetWindowsSize(&wnd_w, &wnd_h, WND_ROW);
		CreateWindowIT(temp, wnd_w, wnd_h, temp->name);
			moveWindow(temp->name,
				(wnd_w + FRAMES_WIDTH) * (_layers.size() % WND_ROW),
				(wnd_h + TOOLBAR_HEIGHT) * (_layers.size() / WND_ROW));
	}
	return _next_id++;
}
unsigned ColorMachine::AddLayer(std::string file_name)
{
	return AddLayer(cv::imread(file_name), file_name);
}
img_trans* ColorMachine::GetLayer(unsigned layer_id)
{
	auto found = _layers.find(layer_id);
	if(found != _layers.end())
		return found->second;
	return nullptr;
}
void ColorMachine::DeleteLayer(unsigned layer_id)
{
	auto found = _layers.find(layer_id);
	if(found != _layers.end())
	{
		if(_show)
			cv::destroyWindow(found->second->name);
		delete found->second;
		_layers.erase(found);
	}
}
void ColorMachine::SetSource(cv::Mat source)
{
	_source.SetImg(source);
	_source.params.clear();
	if(_show)
		imshow(_wnd_original, _source.img);
}
void ColorMachine::SetSource(std::string file_name)
{
	SetSource(cv::imread(file_name));
}
void ColorMachine::Prepare(transfer_method method)
{
	_source.AddParams(method);
	for(auto layer: _layers)
	{
		layer.second->AddParams(method);
	}
}
transfer_method ColorMachine::SetMethod(transfer_method new_method)
{
	transfer_method old = _current_method;
	_current_method = new_method;
	return old;
}
transfer_method ColorMachine::GetMethod()
{
	return _current_method;
}
cv::Mat ColorMachine::TransferColor(transfer_method method)
{
	if(method == METHOD_NONE)
		method = _current_method; // to use METHOD_NONE call ColorMachine::SetMethod(METHOD_NONE)
	cv::Mat temp = CTW(_source, _layers, method);
	if(_show)
		imshow(_wnd_result, temp);
	return temp;
}
void ColorMachine::ShowWindows(bool show)
{
	if(_show == show)
		return;
	if(show)
	{
		int wnd_w, wnd_h;
		GetWindowsSize(&wnd_w, &wnd_h, WND_ROW);
		unsigned row_count = 0;
		for(auto layer: _layers)
		{
			CreateWindowIT(layer.second, wnd_w, wnd_h, layer.second->name);
			moveWindow(layer.second->name,
				(wnd_w + FRAMES_WIDTH) * (row_count % WND_ROW),
				(wnd_h + TOOLBAR_HEIGHT) * (row_count / WND_ROW));
			row_count++;
		}
		CreateWindowIT(&_source, wnd_w, wnd_h, _wnd_original);
		int wnd_top = (row_count + WND_ROW - 1) / WND_ROW  * (wnd_h + TOOLBAR_HEIGHT);
		moveWindow(_wnd_original, 0, wnd_top);
		namedWindow(_wnd_result);
		moveWindow(_wnd_result, wnd_w + FRAMES_WIDTH, wnd_top);
	}
	else
	{
		for(auto& layer: _layers)
			destroyWindow(layer.second->name);
		destroyWindow(_wnd_original);
		destroyWindow(_wnd_result);
	}
	_show = show;
}
std::string CreateRandomName(unsigned num_chars, std::string prefix, std::string postfix)
{
	std::string new_name = prefix;
	if(num_chars > RANDOM_NAME_MAX_CHARS)
		num_chars = RANDOM_NAME_MAX_CHARS;
	unsigned u_rand;
	char rand_chunk[RANDOM_CHUNK_SIZE + 1];
	for(; num_chars; num_chars -= cv::min<unsigned>(num_chars, RANDOM_CHUNK_SIZE))
	{
		rand_s(&u_rand);
		itoa(u_rand % (unsigned)pow(10, cv::min<unsigned>(num_chars, RANDOM_CHUNK_SIZE)), rand_chunk, 10);
		new_name += rand_chunk;
	}
	new_name += postfix;
	return new_name;
}
void CreateWindowIT(img_trans* it, int width, int height, cv::String& wnd_name)
{
	namedWindow(wnd_name, cv::WINDOW_KEEPRATIO);
	resizeWindow(wnd_name, width, height); // should be resized before creating trackbar
	it->ConvertTo(CS_BGR);
	imshow(wnd_name, it->img);
	for(unsigned i = 0; i < TB_COUNT; i++)
	{
		int init_val = it->channel_w[i];
		createTrackbar(it->tb_params[i].tb_name, wnd_name, &init_val, MAX_WEIGHT_VAL, OnTrackBarChanged, &it->tb_params[i]); 
	}
}
void GetWindowsSize(int* width, int* height, unsigned count)
{
	// get screen size and stuff
	*width = 300;
	*height = 200;
	*width += FRAMES_WIDTH;
	*height += TOOLBAR_HEIGHT;
}
void OnTrackBarChanged(int new_pos, void* param)
{
	tb_param* tbp = (tb_param*)param;
#ifdef TESTING
	tbp->it->SetWeight(new_pos); // for testing
#else
	tbp->it->channel_w[tbp->channel] = new_pos; 
#endif
	tbp->cmachine->TransferColor();
}
CTParams* GetCTP(transfer_method method) // can't make with template
{
	switch(method)
	{
	case METHOD_REINHARD:
		return new CTP_Reinhard;
	case METHOD_XIAO:
		return new CTP_Xiao;
	}
	return nullptr;
}
cv::Scalar CalcDivider(img_trans& source, std::map<unsigned, img_trans*>& layers)
{
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
	return divider;
}