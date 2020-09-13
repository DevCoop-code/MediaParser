//
//  mp4Parser.cpp
//  mediaParser
//
//  Created by HanGyo Jeong on 2020/09/13.
//  Copyright © 2020 HanGyoJeong. All rights reserved.
//

#include "mp4Parser.h"

void read4bytes(void* buffer, FILE* fp) {
    unsigned char temp[4];
    fgets(temp, 4 + 1, fp);
    
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
