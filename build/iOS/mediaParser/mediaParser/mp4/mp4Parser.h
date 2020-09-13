//
//  mp4Parser.hpp
//  mediaParser
//
//  Created by HanGyo Jeong on 2020/09/13.
//  Copyright © 2020 HanGyoJeong. All rights reserved.
//

#ifndef mp4Parser_hpp
#define mp4Parser_hpp

#define _CRT_SECURE_NO_WARNINGS     // Prevent Compile Error occured from fopen secure warning
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define OS32 sizeof(void*) != 4 ? 0 : 1
#define OS64 sizeof(void*) != 8 ? 0 : 1

// Defines the box type
#define _FTYP_  0x66747970
#define _MDAT_  0x6D646174
#define _FREE_  0x66726565
#define _MOOV_  0x6D6F6F76
#define     _MVHD_  0x6D766864
#define     _TRAK_  0x7472616B
#define         _TKHD_  0x746B6864
#define         _TREF_  0x74726566
#define         _EDTS_  0x65647473
#define         _MDIA_  0x6D646961
#define             _MDHD_  0x6D646864
#define             _HDLR_  0x68646C72
#define             _MINF_  0x6D696E66
#define                 _VMHD_  0x766D6864
#define                 _SMHD_  0x736D6864
#define                 _DINF_  0x6D696E66
#define                     _DREF_  0x64726566
#define                         _URL_  0x75726C20
#define                 _STBL_  0x7374626C
#define                     _STSD_  0x73747364
#define                         _PASP_  0x70617370
#define                         _MP4V_  0x6D703476
#define                         _MP4A_  0x6D703461
#define                             _ESDS_  0x65736473
#define                         _AVC1_  0x61766331
#define                             _AVCC_  0x61766343
#define                             _BTRT_  0x62747274
#define                     _STTS_  0x73747473
#define                     _CTTS_  0x63747473
#define                     _STSC_  0x73747363
#define                     _STSZ_  0x7374737A
#define                     _STZ2_  0x73747A32
#define                     _STCO_  0x7374636F
#define                     _CO64_  0x636F3634
#define                     _STSS_  0x73747373
#define                     _SBGP_  0x73626770
#define                     _SGPD_  0x73677064
#define                     _SUBS_  0x73756273

// CP = 'Core Parser'
typedef unsigned char           CPINT8;         // 8bits
typedef unsigned short          CPINT16;        // 16bits
typedef unsigned int            CPINT32;        // 32bits
typedef unsigned long long      CPINT64;        // 64bits

/*
FTYP box Structure
*/
 typedef struct {
    CPINT32 major_brands;
    CPINT32 minor_version;
    CPINT32 compatible_brands;
} ftypBox;

/*
MVHD box Structure
*/
 typedef struct {
     CPINT32 size;
     CPINT16 version;
     union {
         CPINT32 val32;
         CPINT64 val64;
     }create_time;
     union {
         CPINT32 val32;
         CPINT64 val64;
     }modification_time;
     CPINT32 timeScale;         // Integer that specifies the time-scale for the entire presentation was modified
     union {
         CPINT32 val32;
         CPINT64 val64;
     }duration;                 // Seconds = duration / timeScale (s)
     CPINT32 next_track_id;
} mvhdBox;

/*
TKHD box Structure
 
 - Track Header, Overall information about the track
 - Exactly one track header box is contained in a track
*/
typedef struct {
    CPINT32 size;
    CPINT8 version;
    union {
        CPINT32 val32;
        CPINT64 val64;
    }create_time;
    union {
        CPINT32 val32;
        CPINT64 val64;
    }modification_time;
    CPINT32 track_id;
    CPINT32 duration;           // Track's play second(based on movie timescale)
    CPINT16 alternate_group;
    CPINT16 volume;             // Full volume is 1.0(0x0100) and is the normal value
    CPINT32 matrix[9];          // Provides a transformation matrix for the video, ex] { 0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000 }
    CPINT32 width;
    CPINT32 height;
} tkhdBox;

/*
MDHD box Structure
 
 - Media Heaader, overall information about the media
*/
typedef struct {
    CPINT32 size;
    CPINT8 version;
    union {
        CPINT32 val32;
        CPINT64 val64;
    }create_time;
    union {
        CPINT32 val32;
        CPINT64 val64;
    }modification_time;
    CPINT32 timeScale;
    union {
        CPINT32 val32;
        CPINT64 val64;
    }duration;
} mdhdBox;

/*
HDLR box Structure
 
 - Handler type when present in a media box
 'vide': Video track
 'soun': Audio track
 'hint': Hint track
 'meta': Timed Metadata track
*/
typedef struct {
    CPINT32 handler_type;
    CPINT8* name;               // UTF-8 character which gives a human-readable name for the track type
} hdlrBox;

/*
URL box Structure
*/
typedef struct {
    CPINT8* name;
} urlBox;

/*
DREF box Structure
*/
typedef struct {
    CPINT32 entry_count;
    urlBox urlAtom;
} drefBox;

/*
DINF box Structure
 
 - Data Information Box, Container
*/
typedef struct {
    drefBox drefAtom;
} dinfBox;

/*
STSD MP4V SampleEntry Structure
*/
typedef struct {
    CPINT16 data_reference_index;
    CPINT16 width;                  // Maximum Width
    CPINT16 height;                 // Maximum Height
} stsd_mp4v_SampleEntry;

/*
STSD MP4A SampleEntry Structure
*/
typedef struct {
    CPINT16 data_reference_index;
    CPINT16 timeScale;
} stsd_mp4a_SampleEntry;

/*
STSD AVC1 SampleEntry Structure
*/
typedef struct {
    CPINT16 data_reference_index;
    CPINT16 width;                  // Maximum width, in pixels of the streaam
    CPINT16 height;                 // Maximum height, in pixels of the streaam
    CPINT32 horizResolution;        // 0x00480000 <- 72dpi
    CPINT32 vertiResolution;        // 0x00480000 <- 72dpi
} stsd_avc1_SampleEntry;

/*
STSD AVCC SampleEntry Structure
*/
typedef struct {
    CPINT32 naluFieldLength;
    
    CPINT32 spsNalUnitLength;
    CPINT32 ppsNalUnitLength;
    
    CPINT8* spsNalUnit;
    CPINT8* ppsNalUnit;
} stsd_avcc_SampleEntry;

/*
ESDS SampleEntry Structure
*/
typedef struct
{

} stsd_esds_SampleEntry;

/*
STTS box Structure
 
 - Decoding time-to-sample
 - Generally, DTS&PTS was taken by stts box, but if DTS&PTS was different(in case of using B-Frame),
 DTS was taken by stts box, PTS taken by ctts box
 - DT(n+1) = DT(n) + STTS(n)
 For example]
 ----------------------------
 Sample Count | Sample-Delta
 ----------------------------
       4      |       3
       2      |       1
       3      |       2
 Sample: 1~4 => Sample-Delta: 3
 Sample: 5~6 => Sample-Delta: 1
 Sample: 7~9 => Sample-Delta: 2
*/
typedef struct {
    CPINT32 entry_count;
    CPINT32* sample_count;
    CPINT32* sample_delta;
} sttsBox;

/*
STSZ, STZ2 box Structure
*/
typedef struct {
    CPINT32 sample_size;    // Specifying the default sample size(if every samples size is different, these value is 0)
    CPINT32 sample_count;   // The number of sample in a track
    CPINT32* entry_size;    // Each Samples size
} stszBox, stz2Box;

/*
STSC box Structure
*/
typedef struct {
    CPINT32 entry_count;
    CPINT8* stscTable;
    CPINT32* first_chunk;
    CPINT32* samples_per_chunk;
    CPINT32* samples_description_index;
} stscBox;

/*
STBL box Structure
 
 - Sample Table Box, Container for the time/space map
 
 MDAT is the media data ato which contain video&audio frames. It is separated into tracks
 Each track has multiple chunks and each chunks has multiple samples
 track
    chunk #1
        sample #1
        sample #2
    chunk #2
 The number of sample in the chunk is defined in stsc atom(sample to chunk box) and the chunk offset is defined in stco atom(chunk offset box)
 
 Sample?
 - All the Data Associated with a single timestamp
 - No two samples within a track can share the same timestamp
 - A Sample is individual frame of video or a compressed section of audio
 - In hint Tracks, a sample defines the formation of one or more streaming packets
*/
typedef struct {
    sttsBox sttsAtom;
    stszBox stszAtom;
    stz2Box stz2Atom;
    stscBox stscAtom;
    stsd_avc1_SampleEntry avc1SampleEntry;
    stsd_avcc_SampleEntry avccSampleEntry;
    stsd_esds_SampleEntry esdsSampleEntry;
    stsd_mp4a_SampleEntry mp4aSampleEntry;
    stsd_mp4v_SampleEntry mp4vSampleEntry;
} stblBox;

/*
MINF box Structure

 - Media Information Container
*/
typedef struct {
    dinfBox dinfAtom;
    stblBox stblAtom;
} minfBox;

/*
MDIA box Structure

 - Container for the media information in a track
*/
typedef struct {
    mdhdBox mdhdAtom;
    hdlrBox hdlrAtom;
    minfBox minfAtom;
} mdiaBox;

/*
TRAK box Structure
*/
typedef struct {
    tkhdBox tkhdAtom;
    mdiaBox mdiaAtom;
} trakBox;

/*
MOOV box Structure
*/
typedef struct {
    int trakIndex;
    
    mvhdBox mvhdAtom;
    trakBox* trakAtom;
} moovBox;

void read4bytes(void* buffer, FILE* fp);
void swap(unsigned char* left, unsigned char* right);
void init(const char* video_file_path);

#endif /* mp4Parser_hpp */
