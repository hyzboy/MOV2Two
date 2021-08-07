#include <iostream>
#include"VideoEncoder.h"
#include"VideoDecoder.h"
#include"FrameRecviver.h"
#include<stdlib.h>

EncodeOutput *CreateEncodeOutput(const char *filename);

class RGBA2Two:public RGBAFrameRecviver
{
    uint8 *two_image=nullptr;

    VideoEncoder *encoder;

public:

    RGBA2Two(VideoEncoder *ve)
    {
        encoder=ve;
    }

    ~RGBA2Two()
    {
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
                *ap=255;    ++sp;
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

            encoder->Set(GetWidth(),GetHeight(),frame_rate);

            encoder->Init();
        }

        Convert(two_image,data,GetWidth(),GetHeight());

        encoder->Put(two_image);

        //SaveToTGA(filename,(void *)two_image,GetWidth()*2,GetHeight(),24,true);
    }
};

int main(int argc,char **argv)
{
    std::cout << "MOV 2 two\n";

    if(argc<4)
    {
        std::cout<<"Format: mov2two [input] [output] [bit rate]\n";
        std::cout<<"Example: mov2two input.mov output.mp4 1048576\n\n";
        return 0;
    }

    long bit_rate=atol(argv[3]);

    std::cout<<"input: "<<argv[1]<<std::endl;
    std::cout<<"output: "<<argv[2]<<std::endl;
    std::cout<<"bit_rate: "<<bit_rate<<std::endl;
    
    EncodeOutput *eo=CreateEncodeOutput(argv[2]);
    VideoEncoder *ve=CreateVideoEncoder(eo,bit_rate);
    FrameRecviver *fr=new RGBA2Two(ve);
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
    delete eo;
    return 0;
}
