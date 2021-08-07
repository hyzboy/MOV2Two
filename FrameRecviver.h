#pragma once

#include"DataType.h"

extern "C"
{
    #include<libavcodec/avcodec.h>
}

class FrameRecviver
{
protected:

    uint width=0;
    uint height=0;

public:

    virtual ~FrameRecviver()=default;

    virtual void OnFrame(const AVFrame *frame)=0;

public:

    const uint GetWidth()const{return width;}
    const uint GetHeight()const{return height;}
};//

class RGBAFrameRecviver:public FrameRecviver
{
private:

    uint8 *rgba=nullptr;

    void OnFrame(const AVFrame *frame);

public:

    virtual ~RGBAFrameRecviver()
    {
        delete[] rgba;
    }

    virtual void OnFrame(const uint8 *)=0;
};//class RGBAFrameRecviver:public FrameRecviver
