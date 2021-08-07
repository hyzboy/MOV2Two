﻿extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
}

#include<iostream>
#include"VideoDecoder.h"
#include"FrameRecviver.h"

namespace
{
    struct CodecName_by_ID
    {
        AVCodecID id;
        char name[32];
    };

    constexpr CodecName_by_ID codec_name_by_id[]=
    {
        {AV_CODEC_ID_MJPEG      ,"mjpeg_cuvid"},
        {AV_CODEC_ID_MPEG1VIDEO ,"mpeg1_cuvid"},
        {AV_CODEC_ID_MPEG2VIDEO ,"mpeg2_cuvid"},
        {AV_CODEC_ID_MPEG4      ,"mpeg4_cuvid"},
        {AV_CODEC_ID_H264       ,"h264_cuvid"},
        {AV_CODEC_ID_HEVC       ,"hevc_cuvid"},
        {AV_CODEC_ID_VC1        ,"vc1_cuvid"},
        {AV_CODEC_ID_VP8        ,"vp8_cuvid"},
        {AV_CODEC_ID_VP9        ,"vp9_cuvid"},
        {AV_CODEC_ID_AV1        ,"av1_cuvid"},

        {AV_CODEC_ID_H264       ,"h264_amf"},
        {AV_CODEC_ID_HEVC       ,"hevc_amf"},

        {AV_CODEC_ID_MJPEG      ,"mjpeg_qsv"},
        {AV_CODEC_ID_MPEG2VIDEO ,"mpeg2_qsv"},
        {AV_CODEC_ID_H264       ,"h264_qsv"},
        {AV_CODEC_ID_HEVC       ,"hevc_qsv"},
        {AV_CODEC_ID_VC1        ,"vc1_qsv"},
        {AV_CODEC_ID_VP8        ,"vp8_qsv"},
        {AV_CODEC_ID_VP9        ,"vp9_qsv"},
        {AV_CODEC_ID_AV1        ,"av1_qsv"},

        {AV_CODEC_ID_NONE       ,""}
    };

    AVCodec *GetAVCodecDecoder(AVCodecID id)
    {
        AVCodec *codec=nullptr;

        for(auto &c:codec_name_by_id)
        {

            if(c.id==id)
            {
                codec=avcodec_find_decoder_by_name(c.name);

                if(codec)
                {
                    std::cout<<"use decoder: "<<c.name<<std::endl;
                    return codec;
                }
            }
        }        

        return avcodec_find_decoder(id);
    }
}//namespace

class FFmpegVideoDecoder:public VideoDecoder
{
    AVFormatContext *   ctx;

    AVCodecParameters * video_cp;

    AVStream *          video_stream;
    int                 video_stream_index;

    AVCodec *           video_codec;
    AVCodecContext *    video_ctx;

    FrameRecviver *     frame_recviver;
    AVFrame *           frame;

    AVPacket            packet;


public:

    FFmpegVideoDecoder(FrameRecviver *fr)
    {
        ctx=nullptr;
        frame_recviver=fr;

        video_cp=nullptr;
        video_stream=nullptr;
        video_stream_index=-1;

        video_codec=nullptr;
        video_ctx=nullptr;

        frame=nullptr;
    }

    ~FFmpegVideoDecoder()
    {
        if(frame)
            av_free(frame);

        if(video_ctx)
            avcodec_free_context(&video_ctx);

        if(ctx)
            avformat_close_input(&ctx);
    }

    bool Init(const char *filename)
    {
        ctx=avformat_alloc_context();

        if(avformat_open_input(&ctx,filename,nullptr,nullptr)!=0)
            return(false);

        if(avformat_find_stream_info(ctx,nullptr)<0)
            return(false);

        av_dump_format(ctx,0,filename,0);

        video_cp=nullptr;
        video_stream=nullptr;
        video_stream_index=-1;

        for(int i=0;i<ctx->nb_streams;i++)
        {
            if(ctx->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO)
            {
                video_stream=ctx->streams[i];
                video_stream_index=i;
                video_cp=video_stream->codecpar;
                break;
            }
        }

        if(!video_cp)
            return(false);

        video_codec=GetAVCodecDecoder(video_cp->codec_id);

        if(!video_codec)
        {
            printf("Can't find decoder:%d\n",video_cp->codec_id);
            return(false);
        }

        video_ctx=avcodec_alloc_context3(video_codec);

        avcodec_parameters_to_context(video_ctx,video_cp);

        if(avcodec_open2(video_ctx,video_codec,nullptr)<0)
        {
            printf("Couldn't open video stream");
            return(false);
        }

        width=video_cp->width;
        height=video_cp->height;

        frame=av_frame_alloc();

        fps=double(video_stream->avg_frame_rate.num)/double(video_stream->avg_frame_rate.den);
        frame_time=double(video_stream->avg_frame_rate.den)/double(video_stream->avg_frame_rate.num);

        return(true);
    }

    void Start()
    {
        if(!video_ctx)return;
        if(!video_stream)return;

        av_seek_frame(ctx,video_stream_index,0,AVSEEK_FLAG_FRAME);
    }

    bool NextFrame()
    {
        int ret;

        while(av_read_frame(ctx,&packet)>=0)
        {
            if(packet.stream_index==video_stream_index)
            {
                if(avcodec_send_packet(video_ctx,&packet)>=0)
                {
                    ret=avcodec_receive_frame(video_ctx,frame);

                    if(ret==AVERROR(EAGAIN)||ret==AVERROR_EOF)
                    {
                        av_packet_unref(&packet);
                        continue;
                    }

                    if(ret<0)
                    {
                        av_packet_unref(&packet);
                        return(false);
                    }

                    frame_recviver->OnFrame(frame);

                    av_packet_unref(&packet);
                    return(true);
                }
            }

            av_packet_unref(&packet);
        }

        return(false);
    }

    void Abort()
    {

    }
};//class FFmpegDecoder:public Decoder

VideoDecoder *CreateVideoDecoder(const char *filename,FrameRecviver *fr)
{
    FFmpegVideoDecoder *dec=new FFmpegVideoDecoder(fr);

    if(dec->Init(filename)==false)
    {
        delete dec;
        return(nullptr);
    }

    return(dec);
}
