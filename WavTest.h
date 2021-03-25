#ifndef ANDROID_WAVTEST_H
#define ANDROID_WAVTEST_H

#include <utils/Errors.h>
#include <utils/String8.h>
#include <cutils/bitops.h>
#include <sys/resource.h>

using namespace android;

class WavFormart
{
public:
    WavFormart(char* filename);
    int ReadData(void** data, int framecount);

    int getSampleRate() const {return mSamplesPerSec;}
    int getChannels() const {return mChannels;}
    int getBitsPerSample() const {return mBitsPerSample;}
    uint32_t getBitsPerSec() const {return mAvg_bytesPerSec;}
    uint32_t getDataSize() const {return mDataSize;}

protected:
    virtual ~WavFormart();

private:
    char* mFileNmame;
    int mFd;

    uint16_t  mCompressionCode;
    uint16_t  mChannels;
    uint32_t  mSamplesPerSec;
    uint32_t  mAvg_bytesPerSec;
    uint16_t  mBlockAlign;
    uint16_t  mBitsPerSample;

    uint32_t  mFileSize;
    uint32_t  mFormartSize;
    uint32_t  mFactSize;
    uint32_t  mDataSize;
    uint32_t  mDataOffset;
    
    char mDateBuffer[8*1024*1024];
    int mDateReadSeek;

    status_t mInitCheck;
    status_t init();
};

#endif
