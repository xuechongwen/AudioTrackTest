#define LOG_NDEBUG 0
#define LOG_TAG "audiotracktest"

#include <stdlib.h>
#include <stdio.h>
#include <cutils/properties.h>
#include <media/AudioSystem.h>
#include <media/AudioTrack.h>
#include <math.h>

#include "WavTest.h"
#include <binder/MemoryDealer.h>
#include <binder/MemoryHeapBase.h>
#include <binder/MemoryBase.h>
#include <binder/ProcessState.h>
#include <utils/Log.h>
#include <fcntl.h>

using namespace android;

void printProcess(int current, int len)
{
    int i = 0;
    char process[101] = {};
    int  num = current * 100 / len;

    fprintf(stdout, "\r"); 

    if (num > 100)
        return;

    for (i = 0; i < 100; i++) {
        process[i] = i < num ? '>' : '_';
    }
    process[100] = '\0';

    fprintf(stdout, "%3d:%02d |%s| %3d:%02d ", current/60, current%60, process, len/60, len%60);
}

int main(int argc, char **argv)
{
    size_t framecount = 16;
    void *buffer = NULL;
    int sizefet = 0;
    int looptimes = 0;
    int sampleRate = 0;
    int channel = 0;
    int bitsPerSample = 0;
    int bitsPerSec = 0;
    int writen = 0;
    int dataSize = 0;

    if (argc != 2) {
        fprintf(stdout, "Usage: %s filename\n", argv[0]);
        return -1;
    }   

    WavFormart* WavFormart1 = new WavFormart(argv[1]);
    sampleRate = WavFormart1->getSampleRate();
    channel = WavFormart1->getChannels();
    bitsPerSample = WavFormart1->getBitsPerSample();

    bitsPerSec = WavFormart1->getBitsPerSec();
    dataSize = WavFormart1->getDataSize();

    fprintf(stdout, "main get the SampleRate %d\n", WavFormart1->getSampleRate());
    fprintf(stdout, "main get the Channels %d\n", WavFormart1->getChannels());
    fprintf(stdout, "main get the BitsPerSample %d\n", WavFormart1->getBitsPerSample());

    if (AudioTrack::getMinFrameCount(&framecount, AUDIO_STREAM_MUSIC, sampleRate) != NO_ERROR ) {
        fprintf(stdout, "Error: cannot compute output frame count\n");
        return -1;
    }

    sp<AudioTrack> track = new AudioTrack();
    if (track->set(AUDIO_STREAM_MUSIC,
                   sampleRate/*samplerate*/,
                   AUDIO_FORMAT_PCM_16_BIT,
                   AUDIO_CHANNEL_OUT_STEREO,
                   0,
                   AUDIO_OUTPUT_FLAG_DEEP_BUFFER,
                   NULL /*callback_t*/,
                   NULL /*user*/,
                   0 /*notificationFrames*/,
                   0 /*sharedBuffer*/,
                   false /*threadCanCallJava*/,
                   (audio_session_t)0 /*sessionId*/,
                   AudioTrack::TRANSFER_OBTAIN) != NO_ERROR) {
        fprintf(stdout, "Error: cannot initialize audio track device\n");
        return -1;
    }

    track->start();
    track->setVolume(1.0);

    while ((sizefet = WavFormart1->ReadData(&buffer, framecount)) > 0)
    {
        int sampleSize = bitsPerSample * channel / 8;
        int framGet = sizefet / sampleSize;
        int offset = 0;
        looptimes += 1;
        while (framGet > 0)
        {
            AudioTrack::Buffer abuffer;
            abuffer.frameCount = framGet;
            status_t status = track->obtainBuffer(&abuffer, 1);
            if (status == NO_ERROR) {
                memcpy(abuffer.i8, (char *)buffer + offset, abuffer.size);
                framGet -= abuffer.frameCount;
                offset = (framecount - framGet) * sampleSize;
                writen += abuffer.frameCount * sampleSize;
                track->releaseBuffer(&abuffer);
            } else if (status != TIMED_OUT && status != WOULD_BLOCK) {
                goto exit;
            }
            printProcess(writen/bitsPerSec, dataSize/bitsPerSec);
        }
    }

exit:
    track->stop();
    fprintf(stdout, "\n"); 

    return 0;
}
