#include"FrameRecviver.h"
#include<iostream>

void RGBAFrameRecviver::OnFrame(const AVFrame *frame)
{
    if(!rgba)
    {
        width=frame->width;
        height=frame->height;

        std::cout<<"size: "<<width<<"x"<<height<<std::endl
                 <<"format: "<<frame->format<<std::endl;

        rgba=new uint8[width*height*4];
    }

    if(frame->format==AV_PIX_FMT_RGBA)
    {
        OnFrame((uint8 *)(frame->data[0]));
    }
}
