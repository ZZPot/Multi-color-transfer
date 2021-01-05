#pragma once
#include <string>
#include <vector>
#include <opencv2/core.hpp>
#include <map>
#include "common.h"

// Must define NOMINMAX

#define IT_CHANNELS	3

#define WND_LAYER		_T("Layer")
#define WND_RAND_NUM	3
#define WND_ORIGINAL	_T("Original")
#define WND_RESULT		_T("Result")

#define MAX_WEIGHT_VAL	10
#define WND_ROW			4
#define FRAMES_WIDTH	9
#define TOOLBAR_HEIGHT	30

#define TESTING

#ifdef TESTING
	#define TB_COUNT	1
#else
	#define TB_COUNT	3
#endif

enum color_space
{
	CS_UNDEFINED = 0,
	CS_RGB,
	CS_BGR,
	CS_Lalphabeta
};
enum transfer_method
{
	METHOD_NONE	= 0,
	METHOD_REINHARD,
	METHOD_XIAO,
	METHOD_MAX
};

class CTParams
{
public:
	virtual ~CTParams(){}
	virtual void SetParams(cv::Mat img) = 0;
	virtual cv::Mat GetParam(int param) = 0;
};

struct img_trans;
class ColorMachine;

// trackbar param
struct tb_param
{
	img_trans* it;
	unsigned channel;
	cv::String tb_name;
	ColorMachine* cmachine;
};

struct img_trans
{
	img_trans(std::tstring fname);
	img_trans(cv::Mat image);
	img_trans();
	void SetImg(std::tstring fname);
	void SetImg(cv::Mat image);
	void SetWeight(double weight = 0);
	color_space ConvertTo(color_space cs);
	void AddParams(transfer_method method);

	std::tstring name;
	cv::Mat img;
	cv::Scalar channel_w;
	color_space	current_cs;
	std::map<transfer_method, CTParams*> params; 
	tb_param tb_params[TB_COUNT]; 
};

class ColorMachine
{
public:
	ColorMachine(std::tstring name = _T(""));
	virtual ~ColorMachine();
	unsigned AddLayer(cv::Mat layer, std::tstring name = _T(""));
	unsigned AddLayer(std::tstring file_name);
	img_trans* GetLayer(unsigned layer_id);
	void DeleteLayer(unsigned layer_id);
	void SetSource(cv::Mat source);
	void SetSource(std::tstring file_name);
	void Prepare(transfer_method method);
	transfer_method SetMethod(transfer_method new_method);
	transfer_method GetMethod();
	cv::Mat TransferColor(transfer_method method = METHOD_NONE);
	void ShowWindows(bool show);
protected:
	img_trans _source;
	std::map<unsigned, img_trans*> _layers;
	unsigned _next_id;
	cv::String _wnd_result;
	cv::String _wnd_original;
	std::tstring _name;
	bool _show;
	transfer_method _current_method;
};
cv::Mat CTW(img_trans& source, std::map<unsigned, img_trans*>& layers, transfer_method method = METHOD_NONE);
void CreateWindowIT(img_trans* it, int width, int height, std::tstring wnd_name);
void GetWindowsSize(int* width, int* height, unsigned count = 3);
void OnTrackBarChanged(int new_pos, void* param);
CTParams* GetCTP(transfer_method method);
cv::Scalar CalcDivider(img_trans& source, std::map<unsigned, img_trans*>& layers);