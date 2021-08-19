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

    uint max_height;

    uint new_width;
    uint new_height;

    VideoEncoder *encoder;

public:

    RGBA2Two(VideoEncoder *ve,const uint mh)
    {
        encoder=ve;
        max_height=mh;
    }

    ~RGBA2Two()
    {
        encoder->Finish();

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

    void Scale()
    {
        libyuv::ARGBScale(two_image,GetWidth()*8,GetWidth()*2,GetHeight(),
                          new_image,new_width*4,new_width,new_height,libyuv::FilterMode::kFilterBox);
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

                new_width=uint(double(GetWidth()*2.0f)*scale);
                new_height=max_height;

                new_image=new uint8[new_width*new_height*4];
            }
            else
            {
                new_width   =GetWidth()*2;
                new_height  =GetHeight();
            }

            encoder->Set(new_width,new_height,frame_rate);

            encoder->Init();
        }

        Convert(two_image,data,GetWidth(),GetHeight());

        if(new_height!=GetHeight())
        {
            Scale();
            encoder->Put(new_image);
        }
        else
        {
            encoder->Put(two_image);
        }

        //SaveToTGA(filename,(void *)two_image,GetWidth()*2,GetHeight(),24,true);
    }
};

int main(int argc,char **argv)
{
    std::cout << "MOV 2 two\n";

    if(argc<4)
    {
        std::cout<<"Format: mov2two [input] [output] [bit rate] [max height]\n";
        std::cout<<"Example: mov2two input.mov output.mp4 1048576 480\n\n";
        return 0;
    }

    long bit_rate=atol(argv[3]);
    long max_height;

    std::cout<<"input: "<<argv[1]<<std::endl;
    std::cout<<"output: "<<argv[2]<<std::endl;
    std::cout<<"bit_rate: "<<bit_rate<<std::endl;

    if(argc<5)
        max_height=0;
    else
        max_height=atol(argv[4]);

    std::cout<<"max height: "<<max_height<<std::endl;
    
    VideoEncoder *ve=CreateVideoEncoder(argv[2],bit_rate);
    FrameRecviver *fr=new RGBA2Two(ve,max_height);
    VideoDecoder *vd=CreateVideoDecoder(argv[1],fr);

    vd->Start();

    uint32 frame_count=0;

    while(vd->NextFrame())
    {
        ++frame_count;
    }

    std::cout<<"frame: "<<frame_count<<std::endl;

    delete vd;
    delete fr;
    delete ve;
    return 0;
}
