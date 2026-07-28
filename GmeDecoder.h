#ifndef GMEDECODER_H
#define GMEDECODER_H
#include <gme/gme.h>

#define SAMPLES_PER_FRAME 2048
#define SAMPLE_RATE 44100

struct game_info_t{
    const char* system;
    const char* game;
    const char* author;
    const char* copyright;
};

class GmeDecoder{
    Music_Emu* emu;
    gme_err_t err;
    gme_info_t* info;
    //char _filename[256];
    short _pcm[SAMPLES_PER_FRAME];
    //int songIndex;
    int totalSongs;
public:
    GmeDecoder();
    bool LoadFile(char* filename);
    int GetTrackCount();
    bool SetTrack(int index);
    short* OutputData();
    game_info_t GetGameInfo();
    int GetPosition() const;
    int GetTrackLength() const;
    void Close();
    ~GmeDecoder();
};
#endif // GMEDECODER_H
