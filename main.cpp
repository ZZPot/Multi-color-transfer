#include "Transfer.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>

#pragma warning(disable: 4244)

using namespace cv;

#define VIDEO
#ifndef VIDEO
	#define SOURCE_PIC "source/1.jpg"
#else
	// SET YOUR VIDEO
	#define SOURCE_VIDEO "../Time-lapse.flv"
#endif

std::vector<std::string> images = {
									"images/1.jpg",
								//	"images/2.jpg",
								//	"images/3.jpg",
									"images/4.jpg",
								//	"images/5.jpg"
									};
int main()
{
	ColorMachine cmachine("1");
	for(auto image: images)
		cmachine.AddLayer(image);
	cmachine.SetMethod(METHOD_REINHARD);
	#ifdef VIDEO
		VideoCapture video;
		video.open(SOURCE_VIDEO);
		if(!video.isOpened())
			return 1;
		Mat frame;
		while(1)
		{
			video >> frame;
			resize(frame, frame, Size(0, 0), 0.3f, 0.3f); // video was too big
			cmachine.SetSource(frame);
			cmachine.ShowWindows(true); // will work only first time
			cmachine.Prepare(METHOD_REINHARD); // first time for all, then only for source
			cmachine.TransferColor();
			if(waitKey(5) >= 0) 
				break;
		}
		video.release();
	#else
		cmachine.SetSource(SOURCE_PIC);
		cmachine.ShowWindows(true);
		cmachine.Prepare(METHOD_REINHARD);
		cmachine.TransferColor();
		waitKey(0);
	#endif
	return 0;
}