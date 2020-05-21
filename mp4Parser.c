#define _CRT_SECURE_NO_WARNINGS     //Prevent Compile Error occured from fopen secure warning
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define OS32 sizeof(void *) != 4 ? 0 : 1
#define OS64 sizeof(void *) != 8 ? 0 : 1

// Defines the box type
#define	_FTYP_	0x66747970
#define _MDAT_	0x6D646174
#define	_FREE_	0x66726565
#define	_MOOV_	0x6D6F6F76
#define		_MVHD_	0x6D766864
#define		_TRAK_	0x7472616B
#define			_TKHD_	0x746B6864
#define			_TREF_	0x74726566
#define			_EDTS_	0x65647473
#define			_MDIA_	0x6D646961
#define				_MDHD_	0x6D646864
#define				_HDLR_	0x68646C72
#define				_MINF_	0x6D696E66
#define					_VMHD_	0x766D6864
#define					_SMHD_	0x736D6864
#define					_DINF_	0x64696E66
#define						_DREF_	0x64726566
#define						    _URL_	0x75726C20
#define					_STBL_	0x7374626C
#define						_STSD_	0x73747364
#define							_pasp_	0x70617370		// Pixel Aspect Ratio Box
#define							_MP4V_	0x6D703476
#define							_MP4A_	0x6D703461
#define                             _ESDS_  0x65736473
#define							_AVC1_	0x61766331
#define								_AVCC_	0x61766343
#define								_BTRT_	0x62747274
#define						_STTS_	0x73747473
#define						_CTTS_	0x63747473
#define						_STSC_	0x73747363
#define						_STSZ_	0x7374737A
#define						_STCO_	0x7374636F
#define						_CO64_	0x636F3634
#define						_STSS_	0x73747373
// #define						_STSH_	0x73747368
// #define						_STDP_	0x73746470
// #define						_SDTP_	0x73647470
#define						_SBGP_	0x73626770
#define						_SGPD_	0x73677064
#define						_SUBS_	0x73756273

typedef struct
{
    unsigned int major_brand;           //32bits
    unsigned int minor_version;         //32bits
    unsigned int compatible_brands;     //32bits
}ftypBox;

// Movie header, overall declarations
typedef struct
{
    unsigned int size;                  //32bits
    unsigned short version;             //16bits
    union
    {
        unsigned int val32;             //32bits
        unsigned long long val64;       //64bits
    }create_time;
    union
    {
        unsigned int val32;
        unsigned long long val64;
    }modification_time;
    unsigned int timeScale;             // Integer that specifies the time-scale for the entire presentation was modified
    union
    {
        unsigned int val32;
        unsigned long long val64;
    }duration;      // Seconds = duration / timeScale (s)
    unsigned int next_track_id;         //indicates a value to use for the track ID of the next track to be added to this presentation.(ISO/IEC 14496-12)
}mvhdBox;

// Track Header, overall information about the track
/*
Exactly one track header box is contained in a track
*/
typedef struct
{
    unsigned int size;
    unsigned char version;              // 8bits
    union
    {
        unsigned int val32;
        unsigned long long val64;
    }create_time;
    union 
    {
        unsigned int val32;
        unsigned long long val64;
    }modification_time;
    unsigned int track_id;
    unsigned int duration;      //Track's play second(based on movie timescale)
    unsigned short alternate_group;     //16bit
    unsigned short volume;          // Full volume is 1.0(0x0100) and is the normal value
    unsigned int matrix[9];         // Provides a transformation matrix for the video { 0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000 }
    unsigned int width;             // Specify the track's visual presentation size
    unsigned int height;
}tkhdBox;

// media header, overall information about tthe media
typedef struct
{
    unsigned int size;
    unsigned char version;
    union
    {
        unsigned int val32;
        unsigned long long val64;
    }create_time;
    union 
    {
        unsigned int val32;
        unsigned long long val64;
    }modification_time;
    unsigned int timeScale;
    union
    {
        unsigned int val32;
        unsigned long long val64;
    }duration;
}mdhdBox;

/*
handler_type when present in a media box
'vide': Video track
'soun': Audio track
'hint': Hint track
'meta': Timed Metadata track
*/
typedef struct 
{
    unsigned int handler_type;
    unsigned char* name;        //UTF-8 character which gives a human-readable name for the track type
}hdlrBox;

typedef struct
{
    unsigned char* name;
}urlBox;

typedef struct
{
    unsigned int entry_count;

    urlBox urlAtom;
}drefBox;
// data information box, container
typedef struct
{
    drefBox drefAtom;
}dinfBox;

typedef struct
{
    unsigned short data_reference_index;
    unsigned short width;               //Maximum width
    unsigned short height;              //Maximum height
}stsd_mp4v_SampleEntry;

typedef struct
{
    unsigned short data_reference_index;
    unsigned short timeScale;
}stsd_mp4a_SampleEntry;

typedef struct
{
    unsigned short data_reference_index;
    unsigned short width;           // Maximum width, in pixels of the stream
    unsigned short height;          // Maximum height, in pixels of the stream
    unsigned int horizResolution;   // 0x00480000 <- 72dpi
    unsigned int vertiResolution;   // 0x00480000 <- 72dpi
}stsd_avc1_SampleEntry;

typedef struct
{
    unsigned int naluFieldLength;

    unsigned int spsNalUnitLength;
    unsigned int ppsNalUnitLength;
    
    unsigned char* spsNalUnit;
    unsigned char* ppsNalUnit;
}stsd_avcc_SampleEntry;

typedef struct
{

}stsd_esds_SampleEntry;

// decoding time-to-sample
/*
Generally, DTS & PTS was taken by stts box. 
but if dts & pts was different(in case of using B frame), DTS taken by stts box, PTS taken by ctts box
DT(n+1) = DT(n) + STTS(n)
For example
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
typedef struct
{
    unsigned int entry_count;
    unsigned int* sample_count;
    unsigned int* sample_delta;
}sttsBox;


// sample table box, container for the time/space map
/*
Mdat is the media data atom which contain video & audio frames. It is separated into tracks
Each track has multiple chunks and each chunks has multiple samples
track
    chunk #1
        sample #1
        sample #2
    chunk #2
The number of sample in the chunk is defined in stsc atom(sample to chunk box) and the chunk offset is defined in stco atom(chunk offset box)

sample?
    - All the data associated with a single timestamp
    - No two samples within a track can share the same timestamp
    - A sample is individual frame of video or a compressed section of audio. In hint tracks, a sample defines the formationo of one or more streaming packets
*/
typedef struct
{
    sttsBox sttsAtom;
    stsd_avc1_SampleEntry avc1SampleEntry;
    stsd_avcc_SampleEntry avccSampleEntry;
    stsd_esds_SampleEntry esdsSampleEntry;
    stsd_mp4a_SampleEntry mp4aSampleEntry;
    stsd_mp4v_SampleEntry mp4vSampleEntry;
}stblBox;

// Media Information Container
typedef struct
{
    dinfBox dinfAtom;
    stblBox stblAtom;
}minfBox;


// Container for the media infoormation in a track
typedef struct
{
    mdhdBox mdhdAtom;
    hdlrBox hdlrAtom;
    minfBox minfAtom;
}mdiaBox;

typedef struct
{
    tkhdBox tkhdAtom;
    mdiaBox mdiaAtom;
}trakBox;

// Container for all the metadata
typedef struct
{
    int trakIndex;

    mvhdBox mvhdAtom;
    trakBox* trakAtom;
}moovBox;

void read4(void* buffer, FILE* fp);
void swap(unsigned char* left, unsigned char* right);
/*
Simple Command line to make executable file
1. gcc -o mp4Parser mp4Parser.c
2. chmod a+x mp4Parser
3. ./mp4Parser

To implement JavaScript FFI(Foreign Fetch Interface)
https://www.sysleaf.com/nodejs-ffi/
*/
int main()
{
    unsigned char atomSizeBuf[4];
    unsigned char atomNameBuf[4];
    int boxInnerLoc = 0;
    int nextBoxLoc = 0;
    int atomSize = 0;
    int mp4Size = 0;
    float mediaTotalDurationSeconds = 0;

    ftypBox ftypAtom;
    moovBox moovAtom;

    //Initialize moovBox Property
    moovAtom.trakIndex = 0;

    FILE *fp = fopen("IU_friday.mp4", "r");

    fseek(fp, 0, SEEK_END);
    mp4Size = ftell(fp);

    printf("Total Size of mp4(%d)\n", mp4Size);

    //Seek to First
    fseek(fp, 0, SEEK_SET);

    while (nextBoxLoc < mp4Size /*&& 0*/)
    {
        // Parsing Box Size
        fgets(atomSizeBuf, 4 + 1, fp); 
        unsigned int atSize = (atomSizeBuf[0]<<24) | (atomSizeBuf[1]<<16) | (atomSizeBuf[2]<<8) | atomSizeBuf[3];
        printf("atom Size Dec:(%d), Hex:(%x) \n",atSize, atSize);
        atomSize = atSize;
        
        // Parsing Box Name
        fgets(atomNameBuf, 4 + 1, fp);
        unsigned int attype = (atomNameBuf[0]<<24) | (atomNameBuf[1]<<16) | (atomNameBuf[2]<<8) | atomNameBuf[3];
        printf("atom Name:(%s), Hex:(%x)\n", atomNameBuf, attype);

        switch(attype)
        {
            case _FTYP_:
                printf("=====[ftyp box]=====\n");

                //Parsing FTYP Atom 'Major brand' property
                read4(&(ftypAtom.major_brand), fp);
                printf("ftyp major brand(%x) \n", ftypAtom.major_brand);

                //Parsing FTYP Atom 'minor version' property
                read4(&(ftypAtom.minor_version), fp);
                printf("ftyp minor version(%x)\n", ftypAtom.minor_version);

                printf("====================\n");
            break;

            case _FREE_:
                printf("=====[free box]=====\n");

                printf("====================\n");
            break;

            case _MDAT_:
                printf("=====[mdat box]=====\n");

                printf("====================\n");
            break;

            case _MOOV_:        //Maybe make do while or while, moov size > searching size
                printf("=====[moov box]=====\n");
                //Parsing the Size of mvhd box
                // read4(&(moovAtom.mvhdAtom.size),fp);
                // printf("mvhd atom size(%x)\n", moovAtom.mvhdAtom.size);

                int moovInnerLoc = 8;   //8: moov size 32bits(4bytes) and moov tag 32bit(4bytes)
                while(moovInnerLoc < atomSize)
                {
                    // Parsing the box size
                    unsigned int mooovInnerBoxSize = 0;
                    read4(&(mooovInnerBoxSize), fp);
                    moovInnerLoc += mooovInnerBoxSize;
                    printf("moovInnerBoxSize:(%x)\n", mooovInnerBoxSize);

                    // Parsing box name
                    fgets(atomNameBuf, 4 + 1, fp);
                    printf("atom Name(%s)\n", atomNameBuf);
                    unsigned int attype = (atomNameBuf[0]<<24) | (atomNameBuf[1]<<16) | (atomNameBuf[2]<<8) | atomNameBuf[3];
                    switch(attype)
                    {
                        case _MVHD_:
                            printf("    =====[mvhd box]=====\n");
                            // Parsing mvhd version info
                            unsigned char mvhdAtomVersionTemp[2];
                            fgets(mvhdAtomVersionTemp, 2 + 1, fp);
                            moovAtom.mvhdAtom.version = (mvhdAtomVersionTemp[0]<<8) | mvhdAtomVersionTemp[1];
                            // printf("mvhd version info(%x)\n", moovAtom.mvhdAtom.version);

                            // Skip the mvhd flag info
                            fseek(fp, 2, SEEK_CUR);

                            //Create, Modification time, timescale, duration is 32bits
                            fseek(fp, 4 + 4, SEEK_CUR);     //Skip the creation & modification time, if you want to parse time you should separate depends on version type
                            unsigned char tempBuf[4];

                            // Parsing mvhd timescale info
                            fgets(tempBuf, 4 + 1, fp);
                            moovAtom.mvhdAtom.timeScale = (tempBuf[0]<<24) | (tempBuf[1]<<16) | (tempBuf[2]<<8) | (tempBuf[3]);
                            printf("    timeScale(%x)\n", moovAtom.mvhdAtom.timeScale);

                            // Parsing mvhd duration info
                            fgets(tempBuf, 4 + 1, fp);
                            if(moovAtom.mvhdAtom.version == 0)
                            {
                                moovAtom.mvhdAtom.duration.val32 = (tempBuf[0]<<24) | (tempBuf[1]<<16) | (tempBuf[2]<<8) | (tempBuf[3]);
                                printf("    duration(%x)\n", moovAtom.mvhdAtom.duration.val32);
                            }
                            else if(moovAtom.mvhdAtom.version == 1)
                            {
                                moovAtom.mvhdAtom.duration.val64 = (tempBuf[0]<<24) | (tempBuf[1]<<16) | (tempBuf[2]<<8) | (tempBuf[3]);
                                printf("    duration(%x)\n", moovAtom.mvhdAtom.duration.val64);
                            }
                            
                            //Calculate the media total duration time(seconds)
                            mediaTotalDurationSeconds = (double)moovAtom.mvhdAtom.duration.val32 / (double)moovAtom.mvhdAtom.timeScale;
                            printf("    MediaTotal Duration(%f)\n", mediaTotalDurationSeconds);

                            fseek(fp, 76, SEEK_CUR);    //Skip the all Reserved value
                            
                            // Parsing Next Track ID Info
                            fgets(tempBuf, 4 + 1, fp);
                            moovAtom.mvhdAtom.next_track_id = (tempBuf[0]<<24) | (tempBuf[1]<<16) | (tempBuf[2]<<8) | (tempBuf[3]);
                            printf("    next Track ID Dec:(%d), Hex:(%x)\n", moovAtom.mvhdAtom.next_track_id, moovAtom.mvhdAtom.next_track_id);
                            
                            printf("    ====================\n");
                        break;

                        case _TRAK_:
                            printf("    =====[trak box]=====\n");

                            int trakLoc = 8;   //8: trak size 32bits(4bytes) and trak tag 32bit(4bytes) 

                            int trackNum = moovAtom.trakIndex;
                            while(trakLoc < mooovInnerBoxSize)
                            {
                                // Parsing the box size
                                unsigned int trakInnerBoxSize = 0;
                                read4(&(trakInnerBoxSize), fp);
                                trakLoc += trakInnerBoxSize;

                                // Parsing the box name
                                fgets(atomNameBuf, 4 + 1, fp);
                                printf("        atom Name(%s)\n", atomNameBuf);
                                unsigned int attype = (atomNameBuf[0]<<24) | (atomNameBuf[1]<<16) | (atomNameBuf[2]<<8) | atomNameBuf[3];
                                
                                unsigned char tempBuf16[2];
                                unsigned char tempBuf32[4];
                                unsigned char tempBuf64[8];
                                switch(attype)
                                {
                                    case _TKHD_:
                                        printf("        =====[tkhd box]=====\n");
                                        // fseek(fp, trakInnerBoxSize - 8, SEEK_CUR);  //8 means box size and tag size
                                        unsigned char version[1];
                                        fgets(version,  1 + 1, fp);
                                        moovAtom.trakAtom[trackNum].tkhdAtom.version = version[0];
                                        
                                        fseek(fp, 3, SEEK_CUR); //Skip the Flags
                                        switch(moovAtom.trakAtom[trackNum].tkhdAtom.version)
                                        {
                                            case 0:
                                                fgets(tempBuf32, 4 + 1, fp);
                                                moovAtom.trakAtom[trackNum].tkhdAtom.create_time.val32 = (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);

                                                fgets(tempBuf32, 4 + 1, fp);
                                                moovAtom.trakAtom[trackNum].tkhdAtom.modification_time.val32 = (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);
                                            break;
                                            
                                            case 1:
                                                fseek(fp, 16, SEEK_CUR);    //Skip the creation, modification time lately it should be filled out
                                            break;

                                            default:

                                            break;
                                        }
                                        fgets(tempBuf32, 4 + 1, fp);
                                        moovAtom.trakAtom[trackNum].tkhdAtom.track_id = (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);
                                        
                                        fseek(fp, 4, SEEK_CUR);     //Skip the Reserved 4bytes
                                        
                                        fgets(tempBuf32, 4 + 1, fp);
                                        moovAtom.trakAtom[trackNum].tkhdAtom.duration = (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);     //Can calculate media Duration : mediaTotalDurationSeconds = (double)moovAtom.trakAtom.tkhdAtom.duration / (double)moovAtom.mvhdAtom.timeScale;

                                        fseek(fp, 12, SEEK_CUR);    //Skip the Reserved, layer, alternate_group property
                                        
                                        //Parsing the volume property - 2bytes
                                        fgets(tempBuf16, 2 + 1, fp);
                                        moovAtom.trakAtom[trackNum].tkhdAtom.volume = (tempBuf16[0] << 8) | (tempBuf16[1] << 8);

                                        fseek(fp, 2, SEEK_CUR);     //Skip the Reserved

                                        fseek(fp, 36, SEEK_CUR);    //Skip the Reserved

                                        //Parsing Width(Available in video track)
                                        fgets(tempBuf32, 4 + 1, fp);
                                        moovAtom.trakAtom[trackNum].tkhdAtom.width = (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);

                                        //Parsing Height(Available in video track)
                                        fgets(tempBuf32, 4 + 1, fp);
                                        moovAtom.trakAtom[trackNum].tkhdAtom.height = (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);
                                    break;
                                    case _TREF_:
                                        printf("        =====[tref box]=====\n");
                                        fseek(fp, trakInnerBoxSize - 8, SEEK_CUR);  //8 means box size and tag size
                                    break;

                                    case _EDTS_:
                                        printf("        =====[edts box]=====\n");
                                        fseek(fp, trakInnerBoxSize - 8, SEEK_CUR);  //8 means box size and tag size
                                    break;

                                    case _MDIA_:
                                        printf("        =====[mdia box]=====\n");
                                        // fseek(fp, trakInnerBoxSize - 8, SEEK_CUR);  //8 means box size and tag size

                                        int mdiaLoc = 8;    //MDIA box size and Tag
                                        while(mdiaLoc < trakInnerBoxSize)
                                        {
                                            //Parsing the box size
                                            fgets(tempBuf32, 4 + 1, fp);
                                            unsigned int mdiaInnerBoxSize = (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);
                                            mdiaLoc += mdiaInnerBoxSize;

                                            //Parsing the box tag
                                            fgets(tempBuf32, 4 + 1, fp);
                                            unsigned int attype = (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);

                                            switch (attype)
                                            {
                                            case _MDHD_:
                                                printf("            =====[mdhd box]=====\n");
                                                // fseek(fp, mdiaInnerBoxSize - 8, SEEK_CUR);
                                                unsigned char version[1];
                                                fgets(version, 1 + 1, fp);
                                                moovAtom.trakAtom[trackNum].mdiaAtom.mdhdAtom.version = version[0];

                                                fseek(fp, 3, SEEK_CUR);     //Skip flags

                                                switch (moovAtom.trakAtom[trackNum].mdiaAtom.mdhdAtom.version)
                                                {
                                                case 0:
                                                    //Parsing creation time
                                                    fgets(tempBuf32, 4 + 1, fp);
                                                    moovAtom.trakAtom[trackNum].mdiaAtom.mdhdAtom.create_time.val32 = (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);

                                                    //Parsing modification time
                                                    fgets(tempBuf32, 4 + 1, fp);
                                                    moovAtom.trakAtom[trackNum].mdiaAtom.mdhdAtom.modification_time.val32 = (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);
                                                    break;
                                                case 1:
                                                    //Parsing creation time
                                                    fseek(fp, 16, SEEK_CUR);    //Lately need to implement this
                                                    break;
                                                default:

                                                    break;
                                                }

                                                fgets(tempBuf32, 4 + 1, fp);
                                                moovAtom.trakAtom[trackNum].mdiaAtom.mdhdAtom.timeScale = (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);

                                                if(moovAtom.trakAtom[trackNum].mdiaAtom.mdhdAtom.version == 0)
                                                {
                                                    fgets(tempBuf32, 4 + 1, fp);
                                                    moovAtom.trakAtom[trackNum].mdiaAtom.mdhdAtom.duration.val32 = (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);
                                                }
                                                else
                                                {
                                                    fseek(fp, 8, SEEK_CUR);     //Lately need to implement this
                                                }
                                                
                                                fseek(fp, 4, SEEK_CUR);     //Skip the pad, language, pre_defined
                                                break;

                                            case _HDLR_:
                                                printf("            =====[hdlr box]=====\n");
                                                // fseek(fp, mdiaInnerBoxSize - 8, SEEK_CUR);
                                               
                                                fseek(fp, 8, SEEK_CUR);     // Skip version, flags and reserved
                                                
                                                fgets(tempBuf32, 4 + 1, fp);
                                                moovAtom.trakAtom[trackNum].mdiaAtom.hdlrAtom.handler_type = (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);
                                                printf("            handler type:(%x)\n", moovAtom.trakAtom[trackNum].mdiaAtom.hdlrAtom.handler_type);

                                                fseek(fp, 4 * 3, SEEK_CUR);     // Skip the reserved

                                                printf("            hdlr box size(%d)\n", mdiaInnerBoxSize);
                                                int nameSize = mdiaInnerBoxSize - (12 + 4 + 8 + 8);
                                                printf("            hdlr track name size(%d)\n", sizeof(char *) * nameSize);
                                                moovAtom.trakAtom[trackNum].mdiaAtom.hdlrAtom.name = (unsigned char*)malloc(sizeof(char *) * nameSize);
                                                
                                                fgets(moovAtom.trakAtom[trackNum].mdiaAtom.hdlrAtom.name, nameSize + 1, fp);
                                                printf("            track name:(%s)\n", moovAtom.trakAtom[trackNum].mdiaAtom.hdlrAtom.name);
                                                break;

                                            case _MINF_:
                                                printf("            =====[minf box]=====\n");

                                                int minfLoc = 8;
                                                while(minfLoc < mdiaInnerBoxSize)
                                                {
                                                    //Parsing the box size
                                                    fgets(tempBuf32, 4 + 1, fp);
                                                    unsigned int minfInnerBoxSize = (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);
                                                    minfLoc += minfInnerBoxSize;

                                                    //Parsing the box tag
                                                    fgets(tempBuf32, 4 + 1, fp);
                                                    unsigned int attype = (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);

                                                    switch (attype)
                                                    {
                                                    case _VMHD_:
                                                        printf("                =====[vmhd box]=====\n");
                                                        fseek(fp, minfInnerBoxSize - 8, SEEK_CUR);
                                                        break;
                                                    case _SMHD_:
                                                        printf("                =====[smhd box]=====\n");
                                                        fseek(fp, minfInnerBoxSize - 8, SEEK_CUR);
                                                        break;
                                                    case _DINF_:
                                                        printf("                =====[dinf box]=====\n");
                                                        // fseek(fp, minfInnerBoxSize - 8, SEEK_CUR);
                                                        int dinfLoc = 8;
                                                        int entry_count = 0;
                                                        while(dinfLoc < minfInnerBoxSize)
                                                        {
                                                            //Parsing the box size
                                                            fgets(tempBuf32, 4 + 1, fp);
                                                            unsigned int dinfInnerBoxSize = (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);
                                                            dinfLoc += dinfInnerBoxSize;

                                                            //Parsing the box tag
                                                            fgets(tempBuf32, 4 + 1, fp);
                                                            unsigned int attype = (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);

                                                            if(attype == _DREF_)
                                                            {
                                                                printf("                    =====[dref box]=====\n");
                                                                fseek(fp, 4, SEEK_CUR);     //Skip version, flag 

                                                                fgets(tempBuf32, 4 + 1, fp);
                                                                moovAtom.trakAtom[trackNum].mdiaAtom.minfAtom.dinfAtom.drefAtom.entry_count = (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);
                                                                entry_count = moovAtom.trakAtom[trackNum].mdiaAtom.minfAtom.dinfAtom.drefAtom.entry_count;
                                                                
                                                                if(entry_count > 0)
                                                                {
                                                                    entry_count--;

                                                                    //Parsing box size
                                                                    fgets(tempBuf32, 4 + 1, fp);
                                                                    unsigned int boxSize = (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);

                                                                    //Parsing box tag
                                                                    fgets(tempBuf32, 4 + 1, fp);
                                                                    unsigned int attype = (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);
                                                                    if(attype == _URL_)
                                                                    {
                                                                        printf("                        =====[url box]=====\n");
                                                                        fseek(fp, 4, SEEK_CUR);     //skip version, flag

                                                                        int locationSize = boxSize - 4 - 8;
                                                                        printf("                        URL location Size(%d)\n", locationSize);
                                                                        char* location = (char *)malloc(sizeof(char*) * locationSize);
                                                                        if(locationSize != 0)
                                                                        {
                                                                            fgets(location, locationSize + 1, fp);
                                                                        }
                                                                        moovAtom.trakAtom[trackNum].mdiaAtom.minfAtom.dinfAtom.drefAtom.urlAtom.name = location;
                                                                    }
                                                                    else
                                                                    {
                                                                        fseek(fp, boxSize - 8, SEEK_CUR);
                                                                    }
                                                                }
                                                            }else
                                                            {
                                                                printf("                    =====[[%x] box]=====\n", attype);
                                                                fseek(fp, dinfInnerBoxSize - 8, SEEK_CUR);
                                                            }
                                                        }
                                                        break;
                                                    case _STBL_:
                                                        printf("                =====[stbl box]=====\n");

                                                        int stblLoc = 8;
                                                        while(stblLoc < minfInnerBoxSize)
                                                        {
                                                            // Parsing the box size
                                                            fgets(tempBuf32, 4 + 1, fp);
                                                            unsigned int stblInnerBoxSize = (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);
                                                            stblLoc += stblInnerBoxSize;

                                                            // Parsing the box tag
                                                            fgets(tempBuf32, 4 + 1, fp);
                                                            unsigned int attype = (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);

                                                            switch(attype)
                                                            {
                                                            case _STSD_:
                                                                printf("                    =====[stsd box]=====\n");
                                                                fseek(fp, 4, SEEK_CUR);     //Skip Version & Flag

                                                                fgets(tempBuf32, 4 + 1, fp);
                                                                unsigned int entryCount = (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);
                                                                while(entryCount > 0)
                                                                {
                                                                    entryCount--;

                                                                    //Parsing the SampleEntry Size
                                                                    fgets(tempBuf32, 4 + 1, fp);
                                                                    unsigned int sampleEntryBoxSize = (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);

                                                                    //Parsing the box tag
                                                                    fgets(tempBuf32, 4 + 1, fp);
                                                                    unsigned int attype = (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);

                                                                    switch (attype)
                                                                    {
                                                                    case _MP4V_:
                                                                        fseek(fp, sampleEntryBoxSize - 8, SEEK_CUR);
                                                                        
                                                                        break;

                                                                    case _MP4A_:
                                                                        // fseek(fp, sampleEntryBoxSize, SEEK_CUR);
                                                                        printf("                        =====[mp4a box]=====\n");

                                                                        fseek(fp, 6, SEEK_CUR); //reserved

                                                                        // Parsing data reference index
                                                                        fgets(tempBuf16, 2 + 1, fp);
                                                                        moovAtom.trakAtom[trackNum].mdiaAtom.minfAtom.stblAtom.mp4aSampleEntry.data_reference_index = (tempBuf16[1] << 8) | tempBuf16[0];

                                                                        fseek(fp, 16, SEEK_CUR);    //Skip reserved

                                                                        // Parsing TimeScale
                                                                        fgets(tempBuf16, 2 + 1, fp);
                                                                        moovAtom.trakAtom[trackNum].mdiaAtom.minfAtom.stblAtom.mp4aSampleEntry.timeScale = (tempBuf16[1] << 8) | tempBuf16[0];

                                                                        fseek(fp, 2, SEEK_CUR);     //Skip reserved

                                                                        // Parsing esds box size
                                                                        fgets(tempBuf32, 4 + 1, fp);
                                                                        unsigned int esdsBoxSize = (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);

                                                                        // Parsing esds box tag
                                                                        fgets(tempBuf32, 4 + 1, fp);
                                                                        unsigned int esdsTag = (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);
                                                                        if(esdsTag == _ESDS_)
                                                                        {
                                                                            printf("                            =====[esds box]=====\n");
                                                                            
                                                                            fseek(fp, esdsBoxSize - 8, SEEK_CUR);

                                                                        }
                                                                        else
                                                                        {
                                                                            fseek(fp, esdsBoxSize - 8, SEEK_CUR);
                                                                        }

                                                                        break;

                                                                    case _AVC1_:
                                                                        printf("                        =====[avc1 box]=====\n");

                                                                        fseek(fp, 6, SEEK_CUR);     //Skip reserved

                                                                        // Parsing data reference index
                                                                        fgets(tempBuf16, 2 + 1, fp);
                                                                        moovAtom.trakAtom[trackNum].mdiaAtom.minfAtom.stblAtom.avc1SampleEntry.data_reference_index = (tempBuf16[1] << 8) | tempBuf16[0];

                                                                        fseek(fp, 16, SEEK_CUR);    //Skip reserved

                                                                        // Parsing width & height
                                                                        fgets(tempBuf16, 2 + 1, fp);
                                                                        moovAtom.trakAtom[trackNum].mdiaAtom.minfAtom.stblAtom.avc1SampleEntry.width = (tempBuf16[1] << 8) | tempBuf16[0];
                                                                        fgets(tempBuf16, 2 + 1, fp);
                                                                        moovAtom.trakAtom[trackNum].mdiaAtom.minfAtom.stblAtom.avc1SampleEntry.height = (tempBuf16[1] << 8) | tempBuf16[0];

                                                                        // Parsing horize/vertical resolution
                                                                        fgets(tempBuf32, 4 + 1, fp);
                                                                        moovAtom.trakAtom[trackNum].mdiaAtom.minfAtom.stblAtom.avc1SampleEntry.horizResolution = (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);
                                                                        fgets(tempBuf32, 4 + 1, fp);
                                                                        moovAtom.trakAtom[trackNum].mdiaAtom.minfAtom.stblAtom.avc1SampleEntry.vertiResolution = (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);

                                                                        fseek(fp, 42, SEEK_CUR);    //skip reserved, frame_count, compressorname, depth, pre_defined

                                                                        // Parsing avc1 children box size
                                                                        fgets(tempBuf32, 4 + 1, fp);
                                                                        unsigned int avc1InnerSize = (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);

                                                                        // Parsing avc1 children box tag
                                                                        fgets(tempBuf32, 4 + 1, fp);
                                                                        unsigned int attype = (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);

                                                                        if(attype == _AVCC_)
                                                                        {
                                                                            printf("                            =====[avcc box]=====\n");
                                                                            fseek(fp, avc1InnerSize - 8, SEEK_CUR);
                                                                        }
                                                                        else
                                                                        {
                                                                            fseek(fp, avc1InnerSize - 8, SEEK_CUR);
                                                                        }
                                                                        break;
                                                                    
                                                                    default:
                                                                        fseek(fp, sampleEntryBoxSize - 8, SEEK_CUR);
                                                                        break;
                                                                    }
                                                                }


                                                                // fseek(fp, stblInnerBoxSize - 8, SEEK_CUR);
                                                                break;

                                                            case _STTS_:
                                                                printf("                    =====[stts box]=====\n");
                                                                fseek(fp, 4, SEEK_CUR);     // Skip the version, flags information
                                                                fgets(tempBuf32, 4 + 1, fp);    // Get the Entry Count
                                                                unsigned int entry_count =  (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);
                                                                unsigned int* sampleCountArray = malloc(sizeof(unsigned int) * entry_count);
                                                                unsigned int* sampleDeltaArray = malloc(sizeof(unsigned int) * entry_count);

                                                                unsigned int i = 0;
                                                                for(i; i < entry_count; i++)
                                                                {
                                                                    fgets(tempBuf32, 4 + 1, fp);    //Get sample count info.
                                                                    unsigned int sampleCountInfo = (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);
                                                                    sampleCountArray[i] = sampleCountInfo;

                                                                    fgets(tempBuf32, 4 + 1, fp);    //Get sample duration info.
                                                                    unsigned int sampleDeltaInfo = (tempBuf32[0]<<24) | (tempBuf32[1]<<16) | (tempBuf32[2]<<8) | (tempBuf32[3]);
                                                                    sampleDeltaArray[i] = sampleDeltaInfo;
                                                                }
                                                                
                                                                moovAtom.trakAtom[trackNum].mdiaAtom.minfAtom.stblAtom.sttsAtom.sample_count = sampleDeltaArray;
                                                                moovAtom.trakAtom[trackNum].mdiaAtom.minfAtom.stblAtom.sttsAtom.sample_delta = sampleDeltaArray;

                                                                printf("                    %x\n", moovAtom.trakAtom[trackNum].mdiaAtom.minfAtom.stblAtom.sttsAtom.sample_delta[2]);

                                                                // fseek(fp, stblInnerBoxSize - 8, SEEK_CUR);
                                                                break;

                                                            case _CTTS_:
                                                                printf("                    =====[ctts box]=====\n");
                                                                fseek(fp, stblInnerBoxSize - 8, SEEK_CUR);
                                                                break;

                                                            case _STSC_:
                                                                printf("                    =====[stsc box]=====\n");
                                                                fseek(fp, stblInnerBoxSize - 8, SEEK_CUR);
                                                                break;

                                                            case _STSZ_:
                                                                printf("                    =====[stsz box]=====\n");
                                                                fseek(fp, stblInnerBoxSize - 8, SEEK_CUR);
                                                                break;

                                                            case _STCO_:
                                                                printf("                    =====[stco box]=====\n");
                                                                fseek(fp, stblInnerBoxSize - 8, SEEK_CUR);
                                                                break;

                                                            case _CO64_:
                                                                printf("                    =====[co64 box]=====\n");
                                                                fseek(fp, stblInnerBoxSize - 8, SEEK_CUR);
                                                                break;

                                                            case _STSS_:
                                                                printf("                    =====[stss box]=====\n");
                                                                fseek(fp, stblInnerBoxSize - 8, SEEK_CUR);
                                                                break;

                                                            case _SBGP_:
                                                                printf("                    =====[sbgp box]=====\n");
                                                                fseek(fp, stblInnerBoxSize - 8, SEEK_CUR);
                                                                break;

                                                            case _SGPD_:
                                                                printf("                    =====[sgpd box]=====\n");
                                                                fseek(fp, stblInnerBoxSize - 8, SEEK_CUR);
                                                                break;

                                                            case _SUBS_:
                                                                printf("                    =====[subs box]=====\n");
                                                                fseek(fp, stblInnerBoxSize - 8, SEEK_CUR);
                                                                break;

                                                            default:
                                                                fseek(fp, stblInnerBoxSize - 8, SEEK_CUR);
                                                                break;
                                                            }
                                                        }
                                                        break;
                                                    default:
                                                        fseek(fp, minfInnerBoxSize - 8, SEEK_CUR);
                                                        break;
                                                    }
                                                }
                                                break;
                                            
                                            default:
                                                fseek(fp, mdiaInnerBoxSize - 8, SEEK_CUR);
                                                break;
                                            }
                                        }
                                        
                                    break;

                                    default:
                                        fseek(fp, trakInnerBoxSize - 8, SEEK_CUR);
                                    break;       
                                }
                            }
                            moovAtom.trakIndex++;
                            printf("    ====================\n");
                        break;
                    }
                    fseek(fp, nextBoxLoc + moovInnerLoc, SEEK_SET);     //moovInnerLoc = Size of 1st moovChildren (mvhd, trak, mvex, ipmc) 
                }

                printf("====================\n");
            break;

            default:

            break;
        }

        // Move to Next box
        nextBoxLoc += atomSize;
        fseek(fp, nextBoxLoc, SEEK_SET);
        // printf("nextBoxLoc(%d)\n", nextBoxLoc);
    }
    
    // Close the file
    fclose(fp);

    return 0;
}

/*
=====Functions=====
*/
void read4(void* buffer, FILE* fp)
{
    unsigned char temp[4];

    fgets(temp, 4 + 1, fp);

    // For Swaping 0 1 2 3 to 3 2 1 0
    swap(&temp[0], &temp[3]);
    swap(&temp[1], &temp[2]);
    
    memcpy(buffer, temp, 4);
}

void swap(unsigned char* left, unsigned char* right)
{
    unsigned char temp = *right;
    *right = *left;
    *left = temp;
}

/*
[fseek] origin parameter
SEEK_SET : Beginning of file
SEEK_CUR : Current position of the file pointer
SEEK_END : End of file

[fgets] File Pointer의 위치를 이동 시킨다
*/