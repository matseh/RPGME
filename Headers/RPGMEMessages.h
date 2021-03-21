#ifndef RPGMEMESSAGES_H
#define RPGMEMESSAGES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    RPGMEMessageIDLoadFileRequest       = 1,
    RPGMEMessageIDLoadFileResponse      = 2,
    RPGMEMessageIDReadAudioDataRequest  = 3,
    RPGMEMessageIDReadAudioDataResponse = 4,
    RPGMEMessageIDGetSongInfoRequest    = 5,
    RPGMEMessageIDGetSongInfoResponse   = 6,
    RPGMEMessageIDPlaySongRequest       = 7,
    RPGMEMessageIDPlaySongResponse      = 8
}   RPGMEMessageID;

typedef struct
{
    char filePath[256];
    uint32_t sampleRate;
} RPGMELoadFileRequestContent;

typedef struct
{
    uint32_t success;
    uint32_t songCount;
} RPGMELoadFileResponseContent;

typedef struct
{
    uint32_t sampleCount;
} RPGMEReadAudioDataRequestContent;

typedef struct
{
    uint32_t success;
    uint32_t sampleCount;
    /* Followed by actual waveform data. */
} RPGMEReadAudioDataResponseContent;

typedef struct
{
    uint32_t subsong;
} RPGMEGetSongInfoRequestContent;

typedef struct
{
    uint32_t success;
    uint32_t duration; /* Song duration in milliseconds. */
    /* System string. */
    /* Game string. */
    /* Song string. */
    /* Author string. */
    /* Copyright string. */
    /* Comment string. */
    /* Dumper string. */

} RPGMEGetSongInfoResponseContent;

typedef struct
{
    uint32_t subsong;
} RPGMEPlaySongRequestContent;

typedef struct
{
    uint32_t success;
} RPGMEPlaySongResponseContent;

#ifdef __cplusplus
}
#endif

#endif
