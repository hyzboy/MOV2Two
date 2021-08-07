#include<io.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<stdio.h>
#include"VideoEncoder.h"

class VideoFileOutput:public EncodeOutput
{
    int fp;

public:

    VideoFileOutput(int f)
    {
        fp=f;        
    }

    ~VideoFileOutput()
    {
        _close(fp);
    }

    bool Write(const void *data,const uint size) override
    {
        return(_write(fp,data,size)==size);
    }
};//class VideoFileOutput:public EncodeOutput

EncodeOutput *CreateEncodeOutput(const char *filename)
{
    int fp;

    _sopen_s(&fp,filename,_O_BINARY|_O_WRONLY|_O_CREAT|_O_TRUNC,_SH_DENYNO,S_IREAD|_S_IWRITE);

    if(fp==-1)
        return(nullptr);

    return(new VideoFileOutput(fp));
}