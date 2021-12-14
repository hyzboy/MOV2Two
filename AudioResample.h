#ifndef __AUDIORESAMPLE__
#define __AUDIORESAMPLE__
/*
*  使用过滤器对音频重新采样 
*/

extern "C"
{
#include "libavformat\avformat.h"
#include <libavfilter\buffersrc.h>
#include <libavfilter\buffersink.h>
#include <libavutil\opt.h>
//#include "libavutil/audio_fifo.h"
};

class CAudioResample
{
public:
	CAudioResample();
	~CAudioResample();

public:
	bool InitFilter(AVCodecContext* dec_ctx, AVCodecContext* en_ctx);

	int AudioResample_Frame(AVFrame* srcFrame, int flags, AVFrame* destFrame);

private:
	AVFilterContext* buffersink_ctx = NULL;
	AVFilterContext* buffersrc_ctx = NULL;

	AVFilterGraph* filter_graph = NULL;
};
#endif
