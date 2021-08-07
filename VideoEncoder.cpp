extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/opt.h>
}

#include"VideoEncoder.h"
#include<iostream>
#include"libyuv.h"

namespace
{
    struct EncodecName_by_ID
    {
        AVCodecID id;
        char name[32];
    };

    constexpr EncodecName_by_ID encodec_name_by_id[]=
    {
        {AV_CODEC_ID_H264       ,"h264_nvenc"},
        {AV_CODEC_ID_H264       ,"nvenc"},
        {AV_CODEC_ID_H264       ,"nvenc_h264"},

        {AV_CODEC_ID_HEVC       ,"hevc_nvenc"},
        {AV_CODEC_ID_HEVC       ,"nvenc_hevc"},

        {AV_CODEC_ID_H264       ,"h264_amf"},
        {AV_CODEC_ID_HEVC       ,"hevc_amf"},

        {AV_CODEC_ID_MJPEG      ,"mjpeg_qsv"},
        {AV_CODEC_ID_MPEG2VIDEO ,"mpeg2_qsv"},
        {AV_CODEC_ID_H264       ,"h264_qsv"},
        {AV_CODEC_ID_HEVC       ,"hevc_qsv"},
        {AV_CODEC_ID_VP9        ,"vp9_qsv"},

        {AV_CODEC_ID_NONE       ,""}
    };

    AVCodec *GetAVEncodec(AVCodecID id)
    {
        AVCodec *codec=nullptr;

        for(auto &c:encodec_name_by_id)
        {

            if(c.id==id)
            {
                codec=avcodec_find_encoder_by_name(c.name);

                if(codec)
                {
                    std::cout<<"use encoder: "<<c.name<<std::endl;
                    return codec;
                }
            }
        }        

        return avcodec_find_encoder(id);
    }
}//namespace

class FFMPEGVideoEncoder:public VideoEncoder
{
    AVCodec *codec;
    AVCodecContext *codec_ctx;
    AVFrame *frame;
    AVPacket *packet;

    uint pts;

public:

    FFMPEGVideoEncoder(AVCodec *eco,EncodeOutput *eo,const uint br):VideoEncoder(eo,br)
    {
        codec=eco;

        frame=av_frame_alloc();
        codec_ctx=avcodec_alloc_context3(codec);
        packet=av_packet_alloc();

        pts=0;
    }

    ~FFMPEGVideoEncoder()
    {
        if(packet)
            av_packet_free(&packet);

        if(frame)
            av_frame_free(&frame);

        if(codec_ctx)
            avcodec_free_context(&codec_ctx);
    }

    void Set(const uint w,const uint h,const AVRational &fr) override
    {
        VideoEncoder::Set(w,h,fr);

        AVRational tb;

        tb.num=fr.den;
        tb.den=fr.num;

        codec_ctx->bit_rate     =bit_rate;
        codec_ctx->width        =w;
        codec_ctx->height       =h;
        codec_ctx->time_base    =tb;
        codec_ctx->framerate    =fr;
        codec_ctx->gop_size     =10;
        codec_ctx->max_b_frames =1;
        codec_ctx->pix_fmt      =AV_PIX_FMT_NV12;
    }

    bool Init() override
    {
        if(avcodec_open2(codec_ctx,codec,nullptr)<0)
        {
            std::cerr<<"avcodec open failed!"<<std::endl;
            return(false);
        }

        av_opt_set(codec_ctx->priv_data,"preset","slow",0);

        frame->format=codec_ctx->pix_fmt;
        frame->width=codec_ctx->width;
        frame->height=codec_ctx->height;
    
        int ret = av_frame_get_buffer(frame, 0);

        if(ret<0)
            return(false);

        return(true);
    }

    bool encode()
    {
        int ret=avcodec_send_frame(codec_ctx,frame);
        if(ret<0)
            return(false);

        while(ret>=0)
        {
            ret=avcodec_receive_packet(codec_ctx,packet);

            if(ret==AVERROR(EAGAIN)||ret==AVERROR_EOF)
                return(true);
            else
            if(ret<0)
                return(false);

            std::cout<<"write "<<packet->size<<" bytes."<<std::endl;

            output->Write(packet->data,packet->size);
            av_packet_unref(packet);
        }

        return(true);
    }

    bool Put(const uint8 *data) override
    {
        int ret=av_frame_make_writable(frame);

        if(ret<0)
            return(false);

        libyuv::ARGBToNV12( data,codec_ctx->width*4,
                            frame->data[0],frame->linesize[0],
                            frame->data[1],frame->linesize[1],
                            codec_ctx->width,codec_ctx->height);

        frame->pts=pts;
        ++pts;

        std::cout<<"convert "<<pts<<" frame."<<std::endl;

        return encode();
    }

    bool Finish() override
    {
        return encode();
    }
};//class FFMPEGVideoEncoder:public VideoEncoder

VideoEncoder *CreateVideoEncoder(EncodeOutput *eo,const uint bit_rate)
{
    constexpr AVCodecID codec_list[]
    {
        AV_CODEC_ID_H264,
        AV_CODEC_ID_HEVC,
        AV_CODEC_ID_VP9
    };

    AVCodec *codec=nullptr;
    
    for(AVCodecID id:codec_list)
    {
        codec=GetAVEncodec(AV_CODEC_ID_H264);        
        if(codec)break;
    }

    if(!codec)
    {
        std::cerr<<"We didn't find a encoder[H264,HEVC,VP9]"<<std::endl;
        return nullptr;
    }

    return(new FFMPEGVideoEncoder(codec,eo,bit_rate));
}