#define LOG_NDEBUG 0
#define LOG_TAG "wavtest"

#include <utils/Errors.h>
#include <utils/String8.h>
#include <cutils/bitops.h>
#include <utils/Log.h>
#include <sys/resource.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#include "WavTest.h"

using namespace android;

WavFormart::WavFormart(char* filename)
    :mFileNmame(filename)
{
    mInitCheck = init();
}

WavFormart::~WavFormart()
{
    close(mFd);
}

int WavFormart::ReadData(void** data, int framecount)
{
    int sizetoread;

    sizetoread = framecount * mChannels * mBitsPerSample / 8;
    *data = mDateBuffer;
	
    if (lseek(mFd, mDateReadSeek, SEEK_SET) != mDateReadSeek){
        ALOGE("ReadData seek Erro");
    }

    mDateReadSeek += sizetoread;
    return read(mFd, mDateBuffer, sizetoread);
}

status_t WavFormart::init()
{
    char buffer[256];
    uint32_t offset = 0;

    ALOGD("WavFormart open file %s", mFileNmame);

    mFd = open(mFileNmame, O_RDWR | O_CREAT, S_IRWXU | S_IRWXO | S_IRWXG);

    if (mFd <= 0)
    {
        ALOGE("open error %s No = %d", mFileNmame, errno);
        return NO_INIT;
    } 
    else
    {
        ALOGD("WavFormart open file %s fd %d", mFileNmame, mFd);
    }

    if (read(mFd, buffer, 12) < 12)
    {
        ALOGE("File size less than Riff Chunk");
        return NO_INIT;
    }

    if (memcmp(buffer, "RIFF", 4))
    {
        ALOGE("File size less than Riff Chunk");
        return NO_INIT;
    }

    if (memcmp(buffer+8, "WAVE", 4))
    {
        ALOGE("File size less than Riff Chunk");
        return NO_INIT;
    }

    mFileSize = *((uint32_t *)(buffer + 4)) + 8;
    ALOGD("The size of file %s is %d", mFileNmame, mFileSize);
    offset += 12;

    while (mFileSize + offset >= 8)
    {
        if (read(mFd, buffer, 8) < 8)
        {
            ALOGE("File size less");
            return NO_INIT;
        }
        offset += 8;

        if (!memcmp(buffer, "fmt ", 4)) 
        {
            ALOGD("get fmt");

            mFormartSize = *((uint32_t *)(buffer + 4)) + 8;
            ALOGD("get the size of Fmt Chunk %d", mFormartSize);

            if ((unsigned int)(read(mFd, buffer, mFormartSize-8)) < mFormartSize-8)
            {
                ALOGE("File size less for Fmt Chunk");
                return NO_INIT;
            }
            offset += mFormartSize - 8;

            mCompressionCode = *((uint16_t *)(buffer));
            mChannels = *((uint16_t *)(buffer + 2));
            mSamplesPerSec = *((uint32_t *)(buffer + 4));
            mAvg_bytesPerSec = *((uint32_t *)(buffer + 8));;
            mBlockAlign = *((uint16_t *)(buffer + 12));
            mBitsPerSample = *((uint16_t *)(buffer + 14));
            ALOGD("get the Compression Code %d", mCompressionCode);
            ALOGD("get the Channels %d", mChannels);
            ALOGD("get the Samples Rate  %d", mSamplesPerSec);
            ALOGD("get the Bytes Per Sec %d", mAvg_bytesPerSec);
            ALOGD("get the mBlockAlign %d", mBlockAlign);
            ALOGD("get the mBitsPerSample %d", mBitsPerSample);

        }
        else if (!memcmp(buffer, "fact", 4))
        {
            ALOGD("get fact");

            mFactSize = *((uint32_t *)(buffer + 4)) + 8;
            ALOGD("get the size of Fact Chunk %d", mDataSize);
        }
        else if (!memcmp(buffer, "data", 4))
        {
            ALOGD("get data");

            mDataSize = *((uint32_t *)(buffer + 4));
            ALOGD("get the size of Data %d", mDataSize);

            ALOGD("Check the size ----");

            ALOGD("check the FileSize %d", mFileSize);
            ALOGD("check the Formart Size %d", mFormartSize);
            ALOGD("check the Fact Size %d", mFactSize);
            ALOGD("check the size of Data %d", mDataSize);
            ALOGD("check the offset %d", offset);
            ALOGD("check the all head size %d", 12 + mFormartSize + mFactSize + 8);
             
            if (offset != 12 + mFormartSize + mFactSize + 8)
            {
                ALOGE("The head size check file");
                return NO_INIT;
            }
            mDataOffset = offset;
			mDateReadSeek = offset;

            return OK;
        }
    }

   return OK;
}



