#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <chrono>

#define TINY_DNG_WRITER_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINY_DNG_NO_EXCEPTION
#define TINY_DNG_LOADER_DEBUG

// enable to measure performance
#undef MEASUREPERF

#include "tiny_dng_writer.h"

#include "turbojpeg.h"

#include "linearizationTable.h"
#include "utils.h"
#include "compress-tile.hpp"

#include "make_dng_from_cfa.h"


int tileWidth = 5888;
int tileLength = 3312;
int tilesCount = 1;

uint16_t * linearizationTable = getLinearizationTable();

std::vector<uint16_t> getTile(uint16_t * inData, int x0, int y0, int x1, int y1) {
    long tileSize = x1 * y1;
    
    std::vector<uint16_t> tile;
    tile.reserve(tileSize);
    
    int rowSize = tileWidth;
    
    int count = 0;
    
    for(int i = y0; i < y1; i++) {
        for(int j = x0; j < x1; j++) {
            int curPos = i * rowSize + j;
            uint16_t curValue = inData[curPos];
            tile[count] = linearizationTable[curValue];
            
            count++;
        }
    }
    
    return tile;
}

long getTiles(uint16_t * inData, long inDataSize, std::vector<unsigned char> * tilesMemoryBlock, std::vector<long> * tilesSizes) {
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    
    int compressedBytesWriteCount = 0;
    
    #ifdef MEASUREPERF
    auto measure1 = high_resolution_clock::now();
    #endif
    
    for (int i = 0; i < tilesCount; i++) {
        
        auto tile = getTile(inData, i * tileWidth, 0, tileWidth + i * tileWidth, tileLength);
        
        unsigned char * compressedTile = NULL;
        
        #ifdef MEASUREPERF
        auto measure2 = high_resolution_clock::now();
        #endif
        
        long compressedTileSize = compressTile(tile.data(), &compressedTile, tileWidth, tileLength);
        
        #ifdef MEASUREPERF
        auto measure3 = high_resolution_clock::now();
        #endif
        
        tilesMemoryBlock->resize(tilesMemoryBlock->size() + compressedTileSize);
        
        for (int i = 0; i < compressedTileSize; i++) {
            (*tilesMemoryBlock)[compressedBytesWriteCount] = compressedTile[i];
            
            compressedBytesWriteCount++;
        }
        
        delete [] compressedTile;

        #ifdef MEASUREPERF
        auto compressTime = duration_cast<milliseconds>(measure3 - measure2);
        std::cout << "compress time: " << compressTime.count() << std::endl;
        #endif
        
        tilesSizes->push_back(compressedTileSize);
    }
    
    #ifdef MEASUREPERF
    auto measure4 = high_resolution_clock::now();
    auto allProcessTime = duration_cast<milliseconds>(measure4 - measure1);
    std::cout << "all time: " << allProcessTime.count() << std::endl;
    #endif
        
    return tilesSizes->size();
}

void makeDngFromCFA(void * inBuf, long size, std::string fileName) {
    tinydngwriter::DNGImage di;
    di.SetSubfileType(false, false, false);
    di.SetBigEndian(false);
    di.SetImageWidth(5888);
    di.SetImageLength(3312);
    di.SetCompression(7);
    di.SetUniqueCameraModel("Panasonic DC-S5M2");
    
    di.SetPhotometric(tinydngwriter::PHOTOMETRIC_CFA);
    di.SetDNGVersion(1, 1, 0, 0);
    
    di.SetOrientation(1);
    di.SetCalibrationIlluminant1(17);
    di.SetCalibrationIlluminant2(21);
    
    di.SetXResolution(72);
    di.SetYResolution(72);
    di.SetResolutionUnit(tinydngwriter::RESUNIT_INCH);
    
    di.SetSamplesPerPixel(1);
    
    unsigned short bps[1] = { 12 };
    di.SetBitsPerSample(1, bps);
    
    di.SetBlackLevelRepeatDim(1, 1);
        
    double cm1[9] = {0.94519, -0.24292, -0.0788574, -0.494141, 1.12805, 0.0595703, -0.00927734, 0.0153809, 0.220337};
    di.SetColorMatrix1(3, cm1);
    
    double cm2[9] = {0.493408, -0.0946045, -0.0467529, -0.400269, 0.959595, 0.132446, 0.013916, -0.0236816, 0.426147};
    di.SetColorMatrix2(3, cm2);
    
    double asn[3] = {0.494629, 1, 0.568237};
    di.SetAsShotNeutral(3, asn);
    
    unsigned char cfaP[4] = {00, 0x1, 0x1, 0x2};
    di.SetCFAPattern(4, cfaP);
    
    unsigned short blL[1] = { 256 };
    di.SetBlackLevel(1, blL);
    
    double whL[1] = { 48179 };
    di.SetWhiteLevelRational(1, whL);
    
    di.SetCFARepeatPatternDim(2, 2);
    
    di.SetCustomFieldShort(tinydngwriter::TIFFTAG_CFALAYOUT, 1);
    
    std::vector<unsigned char> tilesMemoryBlock;
    std::vector<long> tilesSizes;
        
    getTiles((uint16_t *) inBuf, size, &tilesMemoryBlock, &tilesSizes);

    di.SetTilesData(tileWidth, tileLength, &tilesMemoryBlock, &tilesSizes);
    
    tinydngwriter::DNGWriter dng_writer(false);
    
    dng_writer.AddImage(&di);
    
    std::string err2;
    dng_writer.WriteToFile(fileName.c_str(), &err2);
    
    std::cout << err2;
}
