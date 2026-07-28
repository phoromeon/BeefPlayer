#include "GmeDecoder.h"
#include <QDebug>

GmeDecoder::GmeDecoder()
{
    emu = nullptr;
    info = nullptr;
    //songIndex = 0;
    totalSongs = 1;
}

bool GmeDecoder::LoadFile(char *filename)
{
    //Could not print err.
    Close();
    err = gme_open_file(filename, &emu, SAMPLE_RATE);
    if(!err)
    {
        totalSongs = gme_track_count(emu);
        err = gme_track_info(emu, &info, 0);
        return true;
    }
    else
    {
        qDebug() << err;
        return false;
    }
}

int GmeDecoder::GetTrackCount()
{
    return totalSongs;
}

bool GmeDecoder::SetTrack(int index)
{
    if(info)
    {
        gme_free_info(info);
        info = nullptr;
    }
    if(emu)
    {
        err = gme_start_track(emu, index);
        if(!err)
        {
            err = gme_track_info(emu, &info, index);
            return true;
        }
        else
        {
            qDebug() << err;
            return false;
        }
    }
    else
    {
        qDebug() << "Music Emulator hasn't been initialized.";
        return false;
    }
}

short *GmeDecoder::OutputData()
{
    if (!emu) return nullptr;

    err = gme_play(emu, SAMPLES_PER_FRAME, _pcm);
    if (err) {
        qDebug() << "gme_play error:" << err;
        return nullptr;  // 返回静音数据
    }
    return _pcm;
}
//
game_info_t GmeDecoder::GetGameInfo()
{
    game_info_t game_info;
    if(info)
    {
        game_info.game = info->game;
        game_info.system = info->system;
        game_info.author = info->author;
        game_info.copyright = info->copyright;
    }
    return game_info;
}

int GmeDecoder::GetPosition() const
{
    return emu ? gme_tell(emu) : 0;  // 返回当前采样数
}

int GmeDecoder::GetTrackLength() const
{
    return info ? info->length : 0;  // 返回总采样数
}


void GmeDecoder::Close()
{
    if(emu)
    {
        if(info)
        {
            gme_free_info(info);
            info = nullptr;
        }
        gme_delete(emu);
        emu = nullptr;
    }
}

GmeDecoder::~GmeDecoder()
{
    if(emu)
    {
        gme_delete(emu);
        if(info)
            gme_free_info(info);
    }
}
