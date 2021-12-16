#include <iostream>
#include"VideoEncoder.h"
#include"VideoDecoder.h"
#include"FrameRecviver.h"
#include<stdlib.h>
#include"libyuv/scale_argb.h"
#include "AudioDecoder.h"
#include "AudioEncoder.h"

#ifdef USE_JNI
    #include<windows.h>
    #include<jni.h>
#endif//USE_JNI

constexpr uint32_t ALIGN_PIXELS=8;

const uint32_t GetAlignValue(const uint32_t value)
{
    constexpr uint32_t tmp=~(ALIGN_PIXELS-1);

    return (value+ALIGN_PIXELS-1)&tmp;
}

class RGBA2Two:public RGBAFrameRecviver
{
    uint8 *two_image=nullptr;
    uint8 *new_image=nullptr;
    uint8 *rgb_image=nullptr;

    uint max_height;

    bool new_size=false;

    uint new_width;
    uint new_height;

    VideoEncoder *two_encoder;
    VideoEncoder *rgb_encoder;

public:

    RGBA2Two(VideoEncoder *two,VideoEncoder *rgb,const uint mh)
    {
        two_encoder=two;
        rgb_encoder=rgb;
        max_height=mh;
    }

    ~RGBA2Two()
    {
        rgb_encoder->Finish();
        two_encoder->Finish();

        delete[] rgb_image;
        delete[] new_image;
        delete[] two_image;
    }

    void Convert(uint8 *tar,const uint8 *src,const uint width,const uint height)
    {
        uint8 *cp=tar;
        uint8 *ap=tar+width*4;
        const uint8 *sp=src;

        for(uint row=0;row<height;row++)
        {
            for(uint col=0;col<width;col++)
            {
                cp[0]=sp[0];
                cp[1]=sp[1];
                cp[2]=sp[2];
                cp[3]=255;

                cp+=4;sp+=3;

                *ap=*sp;    ++ap;
                *ap=*sp;    ++ap;
                *ap=*sp;    ++ap;
                *ap=255;    ++ap;++sp;
            }
            
            cp+=width*4;
            ap+=width*4;
        }
    }

    void OnFrame(const uint8 *data) override
    {
        if(!two_image)
        {
            two_image=new uint8[GetWidth()*GetHeight()*8];

            if(max_height>0
             &&GetHeight()>max_height)
            {
                const double scale=double(max_height)/double(GetHeight());

                new_width   =GetAlignValue(double(GetWidth())*scale);
                new_height  =GetAlignValue(max_height);

                new_size=true;
            }
            else
            {
                new_width   =GetAlignValue(GetWidth());
                new_height  =GetAlignValue(GetHeight());

                if(new_width!=GetWidth()
                 ||new_height!=GetHeight())
                    new_size=true;
            }
            
            std::cout<<"Movie Origin size: "<<GetWidth()<<"x"<<GetHeight()<<std::endl;

            if(new_size)
            {
                std::cout<<"Movie Scaled size: "<<new_width<<"x"<<new_height<<std::endl;
                new_image=new uint8[new_width*new_height*4*2];
            }

            two_encoder->Set(new_width*2,new_height,frame_rate,time_base);
            two_encoder->Init();

            rgb_image=new uint8[new_width*new_height*4];

            rgb_encoder->Set(new_width,new_height,frame_rate,time_base);
            rgb_encoder->Init();
        }

        Convert(two_image,data,GetWidth(),GetHeight());

        if(new_size)
        {
            libyuv::ARGBScale(two_image,GetWidth()*8,   GetWidth()*2,   GetHeight(),
                              new_image,new_width*8,    new_width*2,    new_height,libyuv::FilterMode::kFilterBox);

            two_encoder->Put(new_image);

            libyuv::ARGBScale(  data,       GetWidth()*4,   GetWidth(), GetHeight(),
                                rgb_image,  new_width*4,    new_width,  new_height,libyuv::FilterMode::kFilterBox);

            rgb_encoder->Put(rgb_image);
        }
        else
        {
            two_encoder->Put(two_image);
            rgb_encoder->Put(data);
        }

        //SaveToTGA(filename,(void *)two_image,GetWidth()*2,GetHeight(),24,true);
    }
};

uint32_t ConvertMovie(const char *src,const char *two,const char *rgb,const uint32_t max_height,const uint32_t bit_rate,const bool use_hardware)
{    
    VideoEncoder *ve_two=CreateVideoEncoder(two,bit_rate,use_hardware);
    VideoEncoder *ve_rgb=CreateVideoEncoder(rgb,bit_rate,use_hardware);
    FrameRecviver *fr=new RGBA2Two(ve_two,ve_rgb,max_height);
    VideoDecoder *vd=CreateVideoDecoder(src,fr,use_hardware);

    //音频
    AudioDecoder* audio_decoder = nullptr;
    if (vd->GetAudioIndex() > -1)
    {
        audio_decoder = new AudioDecoder();
        if (!audio_decoder->init(vd->GetFrmCtx(), vd->GetAudioIndex()))
        {
            return -1;
        }

        AudioEncoder* audio_encoder = new AudioEncoder();
        if (!audio_encoder->Init())
        {
            return -1;
        }

        if (!audio_decoder->CreateAudioProcesser(audio_encoder->GetEncoderCtx()))
        {
            return -1;
        }

        audio_decoder->SetEncoder(audio_encoder);

        ve_two->AddAudioStream(audio_encoder->GetEncoderCtx(), vd->GetAudioTimeBase());
        ve_rgb->AddAudioStream(audio_encoder->GetEncoderCtx(), vd->GetAudioTimeBase());

        audio_encoder->SetVideoEncoder(ve_two, ve_rgb);
    }
    vd->SetAudioDecoder(audio_decoder);
    vd->Start();

    uint32_t frame_count=0;

    while(vd->NextFrame())
        ++frame_count;

    delete vd;
    delete fr;
    delete ve_two;
    delete ve_rgb;

    return frame_count;
}

bool Convert(const char *src,const char *two,const char *rgb,const uint32_t bit_rate,const uint32_t max_height)
{
    std::cout<<"            input: "<<src<<std::endl;
    std::cout<<"output(rgb+alpha): "<<two<<std::endl;
    std::cout<<"      output(rgb): "<<rgb<<std::endl;
    std::cout<<"         bit_rate: "<<bit_rate<<std::endl;
    std::cout<<"       max height: "<<max_height<<std::endl;

//    return true;

    uint32_t frame_count=ConvertMovie(src,two,rgb,max_height,bit_rate,true);

    if(frame_count==0)
    {
        std::cerr<<"first decoder/encoder failed, try use software decoder/encoder"<<std::endl;

        frame_count=ConvertMovie(src,two,rgb,max_height,bit_rate,false);
    }
        
    std::cout<<std::endl;

    if(frame_count>0)
    {
        std::cout<<"Movie Encoder Finished!"<<std::endl;
        std::cout<<"Total frame: "<<frame_count<<std::endl;

        return true;
    }
    else
    {
        std::cout<<"Movie Encoder Failed!"<<std::endl;

        return false;
    }
}

#ifdef USE_JNI

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

const std::string GetJavaString(JNIEnv *env,jstring jstr)
{
    int length=env->GetStringUTFLength(jstr);
    const char *str=env->GetStringUTFChars(jstr,nullptr);

    return std::string(str,length);
}

extern "C" JNIEXPORT jint JNICALL Java_com_hyzboy_mov2two_convert(JNIEnv *env,jobject obj,jstring jsrc,jstring jtwo,jstring jrgb,jint bit_rate,jint max_height)
{
    jboolean jb=false;

    const std::string src=GetJavaString(env,jsrc);
    const std::string two=GetJavaString(env,jtwo);
    const std::string rgb=GetJavaString(env,jrgb);

    return Convert(src.c_str(),two.c_str(),rgb.c_str(),bit_rate,max_height);
}
#else
int main(int argc,char **argv)
{
    std::cout << "MOV 2 two\n";

    if(argc<6)
    {
        std::cout<<"Format: mov2two [input] [output two] [output rgb] [bit rate] [max height]\n";
        std::cout<<"Example: mov2two input.mov output_two.mp4 output_rgb.mp4 1048576 480\n\n";
        return 0;
    }

    long bit_rate=atol(argv[4]);
    long max_height;

    if(argc<5)
        max_height=0;
    else
        max_height=atol(argv[5]);

    Convert(argv[1],argv[2],argv[3],bit_rate,max_height);

    /*long bit_rate = 2000000;
    long max_height=1080;
    const char* infile = "d:\\mov\\39.mov";
    const char* outfile1 = "d:\\mov\\31.mp4";
    const char* outfile2 = "d:\\mov\\31_1.mp4";
   
    Convert(infile, outfile1, outfile2, bit_rate, max_height);*/

    return 0;
}
#endif//USE_JNI
