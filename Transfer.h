#pragma once
#include <string>
#include <vector>
#include <opencv2/core.hpp>
#include <map>

#define IT_CHANNELS	3
#define RANDOM_NAME_MAX_CHARS 30
#define RANDOM_CHUNK_SIZE	5

#define WND_LAYER		"Layer"
#define WND_RAND_NUM	3
#define WND_ORIGINAL	"Original"
#define WND_RESULT		"Result"

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
	METHOD_MAX
};

class CTParams
{
public:
	virtual ~CTParams(){}
	virtual void SetParams(cv::Mat img) = 0;
	virtual double GetParam(int param, int number) = 0;
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
	img_trans(std::string fname);
	img_trans(cv::Mat image);
	img_trans();
	void SetImg(std::string fname);
	void SetImg(cv::Mat image);
	void SetWeight(double weight = 0);
	color_space ConvertTo(color_space cs);
	void AddParams(transfer_method method);

	cv::String name;
	cv::Mat img;
	cv::Scalar channel_w;
	color_space	current_cs;
	std::map<transfer_method, CTParams*> params; 
	tb_param tb_params[TB_COUNT]; 
};

class ColorMachine
{
public:
	ColorMachine(std::string name = "");
	virtual ~ColorMachine();
	unsigned AddLayer(cv::Mat layer, std::string name = "");
	unsigned AddLayer(std::string file_name);
	img_trans* GetLayer(unsigned layer_id);
	void DeleteLayer(unsigned layer_id);
	void SetSource(cv::Mat source);
	void SetSource(std::string file_name);
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
	std::string _name;
	bool _show;
	transfer_method _current_method;
};
cv::Mat CTW(img_trans& source, std::map<unsigned, img_trans*>& layers, transfer_method method = METHOD_NONE);
std::string CreateRandomName(unsigned num_chars, std::string prefix = "", std::string postfix = "");
void CreateWindowIT(img_trans* it, int width, int height, cv::String& wnd_name);
void GetWindowsSize(int* width, int* height, unsigned count = 3);
void OnTrackBarChanged(int new_pos, void* param);
CTParams* GetCTP(transfer_method method);