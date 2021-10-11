#pragma once
#include"DataType.h"
extern "C"
{
    #include<libavcodec/avcodec.h>
}

class EncodeOutput
{
public:

    virtual ~EncodeOutput()=default;

    virtual bool Write(const void *,const uint)=0;
};//class EncodeOutput

class VideoEncoder
{
protected:

    uint width;
    uint height;
    AVRational frame_rate;
    AVRational time_base;
    uint bit_rate;

    char filename[_MAX_PATH];

public:

    VideoEncoder(const char *fn,const uint br)
    {
        strcpy_s(filename,_MAX_PATH,fn);

        width=height=0;
        bit_rate=br;
    }

    virtual ~VideoEncoder()=default;

    virtual void Set(const uint w,const uint h,const AVRational &fr,const AVRational &tb)
    {
        width=w;
        height=h;
        frame_rate=fr;
        time_base=tb;
    }

    virtual bool Init()=0;

    virtual bool Put(const uint8 *)=0;

    virtual bool Finish()=0;
};//class VideoEncoder

VideoEncoder *CreateVideoEncoder(const char *filename,const uint bit_rate,const bool);