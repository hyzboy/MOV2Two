#include <iostream>
#include"VideoEncoder.h"
#include"VideoDecoder.h"
#include"FrameRecviver.h"
#include<stdlib.h>
#include"libyuv/scale_argb.h"

class RGBA2Two:public RGBAFrameRecviver
{
    uint8 *two_image=nullptr;
    uint8 *new_image=nullptr;
    uint8 *rgb_image=nullptr;

    uint max_height;

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

                new_width=uint(double(GetWidth())*scale);
                new_height=max_height;

                new_image=new uint8[new_width*new_height*4*2];
            }
            else
            {
                new_width   =GetWidth();
                new_height  =GetHeight();
            }

            two_encoder->Set(new_width*2,new_height,frame_rate,time_base);
            two_encoder->Init();

            rgb_image=new uint8[new_width*new_height*4];

            rgb_encoder->Set(new_width,new_height,frame_rate,time_base);
            rgb_encoder->Init();
        }

        Convert(two_image,data,GetWidth(),GetHeight());

        if(new_height!=GetHeight())
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

int main(int argc,char **argv)
{
    std::cout << "MOV 2 two\n";

    if(argc<5)
    {
        std::cout<<"Format: mov2two [input] [output two] [output rgb] [bit rate] [max height]\n";
        std::cout<<"Example: mov2two input.mov output_two.mp4 output_rgb.mp4 1048576 480\n\n";
        return 0;
    }

    long bit_rate=atol(argv[4]);
    long max_height;

    std::cout<<"            input: "<<argv[1]<<std::endl;
    std::cout<<"output(rgb+alpha): "<<argv[2]<<std::endl;
    std::cout<<"      output(rgb): "<<argv[3]<<std::endl;
    std::cout<<"         bit_rate: "<<bit_rate<<std::endl;

    if(argc<5)
        max_height=0;
    else
        max_height=atol(argv[5]);

    std::cout<<"max height: "<<max_height<<std::endl;
    
    VideoEncoder *ve_two=CreateVideoEncoder(argv[2],bit_rate);
    VideoEncoder *ve_rgb=CreateVideoEncoder(argv[3],bit_rate);
    FrameRecviver *fr=new RGBA2Two(ve_two,ve_rgb,max_height);
    VideoDecoder *vd=CreateVideoDecoder(argv[1],fr);

    vd->Start();

    uint32 frame_count=0;

    while(vd->NextFrame())
        ++frame_count;
        
    std::cout<<std::endl;
    std::cout<<"Movie Encoder Finished!"<<std::endl;
    std::cout<<"Total frame: "<<frame_count<<std::endl;

    delete vd;
    delete fr;
    delete ve_two;
    delete ve_rgb;
    return 0;
}
