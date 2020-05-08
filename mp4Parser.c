#define _CRT_SECURE_NO_WARNINGS     //Prevent Compile Error occured from fopen secure warning
#include <stdio.h>
#include <string.h>

#define OS32 sizeof(void *) != 4 ? 0 : 1
#define OS64 sizeof(void *) != 8 ? 0 : 1

// Defines the box type
#define	_FTYP_	0x66747970
#define _MDAT_	0x6D646174
#define	_FREE_	0x66726565
#define	_MOOV_	0x6D6F6F76
#define		_MVHD_	0x6D766864
#define		_TRAK_	0x7472616B

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
    unsigned int timeScale;
    union
    {
        unsigned int val32;
        unsigned long long val64;
    }duration;      // Seconds = duration / timeScale (s)
    unsigned int next_track_id;         //indicates a value to use for the track ID of the next track to be added to this presentation.(ISO/IEC 14496-12)
}mvhdBox;

// Container for all the metadata
typedef struct
{
    mvhdBox mvhdAtom;
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

                    // Parsing mvhd box name
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

                            printf("    ====================\n");
                        break;
                    }
                    fseek(fp, nextBoxLoc + moovInnerLoc, SEEK_SET);
                }

                printf("====================\n");
            break;

            default:

            break;
        }

        // Move to Next box
        nextBoxLoc += atomSize;
        fseek(fp, nextBoxLoc, SEEK_SET);
        printf("nextBoxLoc(%d)\n", nextBoxLoc);
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