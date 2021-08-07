#include <iostream>
#include"VideoDecoder.h"
#include"FrameRecviver.h"
#include<stdlib.h>

class RGBFR:public RGBAFrameRecviver
{
    uint count=0;

    char filename[32];

    uint8 *two_image=nullptr;

public:

    ~RGBFR()
    {
        delete[] two_image;
    }

    void RGBA2Two(uint8 *tar,const uint8 *src,const uint width,const uint height)
    {
        uint8 *cp=tar;
        uint8 *ap=tar+width*3;
        const uint8 *sp=src;

        for(uint row=0;row<height;row++)
        {
            for(uint col=0;col<width;col++)
            {
                cp[0]=sp[2];
                cp[1]=sp[1];
                cp[2]=sp[0];    cp+=3;sp+=3;
                *ap=*sp;    ++ap;
                *ap=*sp;    ++ap;
                *ap=*sp;    ++ap;++sp;
            }
            
            cp+=width*3;
            ap+=width*3;
        }
    }

    void OnFrame(const uint8 *data) override
    {
        sprintf_s(filename,32,"%d.tga",count);
        ++count;

        if(!two_image)
            two_image=new uint8[GetWidth()*GetHeight()*6];

        RGBA2Two(two_image,data,GetWidth(),GetHeight());

        SaveToTGA(filename,(void *)two_image,GetWidth()*2,GetHeight(),24,true);
    }
};

int main(int argc,char **argv)
{
    std::cout << "MOV 2 two\n";

    if(argc<2)
    {
        std::cout<<"Example: mov2two 1.mov\n\n";
        return 0;
    }

    std::cout<<"input: "<<argv[1]<<std::endl;

    FrameRecviver *fr=new RGBFR();

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
    return 0;
}
