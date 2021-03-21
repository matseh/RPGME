#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gme.h"
#include "RPIPCMessage.h"
#include "RPGMEMessages.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

typedef struct
{
    Music_Emu *emulator;
    RPIPCMessageRef request;
    RPIPCMessageRef response;
} RPGMEPlayer;

typedef RPGMEPlayer *RPGMEPlayerRef;

void RPGMEPlayerDelete(RPGMEPlayerRef gmePlayer)
{
    if(gmePlayer->emulator)
    {
        gme_delete(gmePlayer->emulator);
    }
}

RPGMEPlayerRef RPGMEPlayerCreate(void)
{
    RPGMEPlayerRef gmePlayer = calloc(1, sizeof(RPGMEPlayer));

    if(gmePlayer)
    {
        gmePlayer->request  = RPIPCMessageCreate();
        gmePlayer->response = RPIPCMessageCreate();

        if(!gmePlayer->request || !gmePlayer->response)
        {
            RPGMEPlayerDelete(gmePlayer);

            gmePlayer = NULL;
        }
    }

    return gmePlayer;
}

void RPGMEPlayerLoadFile(RPGMEPlayerRef gmePlayer,
                         RPGMELoadFileRequestContent *requestContent)
{
    gme_err_t gmeError = NULL;
    int songCount = 0;
    int success = FALSE;

    RPGMELoadFileResponseContent *responseContent = (RPGMELoadFileResponseContent *) RPIPCMessageContent(gmePlayer->response);

    if(gmePlayer->emulator)
    {
        gme_delete(gmePlayer->emulator);

        gmePlayer->emulator = NULL;
    }

    gmeError = gme_open_file(requestContent->filePath,
                             &gmePlayer->emulator,
                             requestContent->sampleRate);

    if(gmeError)
    {
        fprintf(stderr, "RPGME: Failed to load \"%s\". Error: %s\n",
                requestContent->filePath,
                gmeError);
    }
    else
    {
        songCount = gme_track_count(gmePlayer->emulator);

        if(songCount > 0)
        {
            success = TRUE;
        }
        else
        {
            songCount = 0;
        }
    }

    RPIPCMessageSetID(gmePlayer->response, RPGMEMessageIDLoadFileResponse);
    RPIPCMessageSetContentLength(gmePlayer->response, sizeof(RPGMELoadFileResponseContent));
 
    responseContent->success   = success;
    responseContent->songCount = songCount;

    RPIPCMessageSend(gmePlayer->response, stdout);
}

void RPGMEPlayerReadAudioData(RPGMEPlayerRef gmePlayer,
                              RPGMEReadAudioDataRequestContent *requestContent)
{

    RPGMEReadAudioDataResponseContent *responseContent = (RPGMEReadAudioDataResponseContent *) RPIPCMessageContent(gmePlayer->response);

    if(gme_track_ended(gmePlayer->emulator))
    {
        responseContent->sampleCount = 0;
    }
    else
    {
        gme_err_t gmeError = NULL;

        uint32_t sampleCount = requestContent->sampleCount;

        uint32_t maxSampleCount = (RPIPCMessageMaxContentLength(gmePlayer->response) - sizeof(RPGMEReadAudioDataResponseContent)) / sizeof(int16_t);

        if(sampleCount > maxSampleCount)
        {
            sampleCount = maxSampleCount;
        }

        gmeError = gme_play(gmePlayer->emulator,
                            sampleCount,
                            (short *) &responseContent[1]);

        if(gmeError)
        {
            fprintf(stderr, "RPGME: Failed to read audio data. Error: %s\n",
                    gmeError);

            sampleCount = 0;
        }

        responseContent->sampleCount = sampleCount;
    }

    RPIPCMessageSetID(gmePlayer->response, RPGMEMessageIDReadAudioDataResponse);
    RPIPCMessageSetContentLength(gmePlayer->response, sizeof(RPGMEReadAudioDataResponseContent) + (sizeof(int16_t) * responseContent->sampleCount));
    RPIPCMessageSend(gmePlayer->response, stdout);
}

void RPGMEPlayerGetSongInfo(RPGMEPlayerRef gmePlayer,
                             RPGMEGetSongInfoRequestContent *requestContent)
{
    gme_err_t gmeError = NULL;
    gme_info_t *info = NULL;
    int success = TRUE;

    RPGMEGetSongInfoResponseContent *responseContent = (RPGMEGetSongInfoResponseContent *) RPIPCMessageContent(gmePlayer->response);

    RPIPCMessageSetID(gmePlayer->response, RPGMEMessageIDGetSongInfoResponse);
    RPIPCMessageSetContentLength(gmePlayer->response, sizeof(RPGMEGetSongInfoResponseContent));

    gmeError = gme_track_info(gmePlayer->emulator,
                              &info,
                              requestContent->subsong - 1);

    if(gmeError)
    {
        fprintf(stderr, "RPGME: Failed to get info for subsong %u. Error: %s\n",
                requestContent->subsong,
                gmeError);

        success = FALSE;
    }
    else
    {
        if(info->length > 0)
        {
            responseContent->duration = info->length;
        }
        else
        {
            responseContent->duration = 0;

            if(info->intro_length > 0)
            {
                responseContent->duration += info->intro_length;
            }

            if(info->loop_length > 0)
            {
                responseContent->duration += info->loop_length;
            }
        }

        success = RPIPCMessageWriteString(gmePlayer->response, info->system);
        
        if(success)
        {
            success = RPIPCMessageWriteString(gmePlayer->response, info->game);
        }

        if(success)
        {
            success = RPIPCMessageWriteString(gmePlayer->response, info->song);
        }
        
        if(success)
        {
            success = RPIPCMessageWriteString(gmePlayer->response, info->author);
        }
        
        if(success)
        {
            success = RPIPCMessageWriteString(gmePlayer->response, info->copyright);
        }
        
        if(success)
        {
            success = RPIPCMessageWriteString(gmePlayer->response, info->comment);
        }

        if(success)
        {
            success = RPIPCMessageWriteString(gmePlayer->response, info->dumper);
        }

        gme_free_info(info);
    }

    responseContent->success = success;

    RPIPCMessageSend(gmePlayer->response, stdout);
}

void RPGMEPlayerPlaySong(RPGMEPlayerRef gmePlayer,
                         RPGMEPlaySongRequestContent *requestContent)
{
    gme_err_t gmeError = NULL;
    int success = TRUE;

    gmeError = gme_start_track(gmePlayer->emulator,
                               requestContent->subsong - 1);

    if(gmeError)
    {
        fprintf(stderr, "RPGME: Failed to play subsong %u. Error: %s\n",
                requestContent->subsong,
                gmeError);

        success = FALSE;
    }

    RPGMEPlaySongResponseContent *responseContent = (RPGMEPlaySongResponseContent *) RPIPCMessageContent(gmePlayer->response);

    responseContent->success = success;

    RPIPCMessageSetID(gmePlayer->response, RPGMEMessageIDPlaySongResponse);
    RPIPCMessageSetContentLength(gmePlayer->response, sizeof(RPGMEPlaySongResponseContent));
    RPIPCMessageSend(gmePlayer->response, stdout);
}

void RPGMEPlayerRun(RPGMEPlayerRef gmePlayer)
{
    int success = TRUE;

    do
    {
        success = RPIPCMessageReceive(gmePlayer->request, stdin);

        if(success)
        {
            void *requestContent = RPIPCMessageContent(gmePlayer->request);

            switch(RPIPCMessageID(gmePlayer->request))
            {
                case RPGMEMessageIDLoadFileRequest:
                    RPGMEPlayerLoadFile(gmePlayer, (RPGMELoadFileRequestContent *) requestContent);
                    break;

                case RPGMEMessageIDReadAudioDataRequest:
                    RPGMEPlayerReadAudioData(gmePlayer, (RPGMEReadAudioDataRequestContent *) requestContent);
                    break;

                case RPGMEMessageIDGetSongInfoRequest:
                    RPGMEPlayerGetSongInfo(gmePlayer, (RPGMEGetSongInfoRequestContent *) requestContent);
                    break;

                case RPGMEMessageIDPlaySongRequest:
                    RPGMEPlayerPlaySong(gmePlayer, (RPGMEPlaySongRequestContent *) requestContent);
                    break;

                default:
                    fprintf(stderr, "RPGME: Unknown request ID: %u\n", RPIPCMessageID(gmePlayer->request));
                    success = FALSE;
                    break;
            }
        }

    } while(success);
}

int main(int argc, char const *argv[])
{
    RPGMEPlayerRef gmePlayer = RPGMEPlayerCreate();

    if(gmePlayer)
    {
        RPGMEPlayerRun(gmePlayer);

        RPGMEPlayerDelete(gmePlayer);
    }

    return -1;
}
