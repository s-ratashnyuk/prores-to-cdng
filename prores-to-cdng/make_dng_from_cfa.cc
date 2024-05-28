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

#include "compress-tile.hpp"

#include "make_dng_from_cfa.h"

int tilesCount = 1;

std::vector<uint16_t> getTile(uint16_t * inData, int x0, int y0, int x1, int y1, int imageWidth, uint16_t * linearizationTable) {
    long tileSize = x1 * y1;
    
    std::vector<uint16_t> tile;
    tile.reserve(tileSize);
    
    int rowSize = imageWidth;
    
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

long getTiles(uint16_t * inData, long inDataSize, std::vector<unsigned char> * tilesMemoryBlock, std::vector<long> * tilesSizes, int tileWidth, int tileLength, uint16_t * linearizationTable) {
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    
    int compressedBytesWriteCount = 0;
    
    #ifdef MEASUREPERF
    auto measure1 = high_resolution_clock::now();
    #endif
    
    for (int i = 0; i < tilesCount; i++) {
        
        auto tile = getTile(inData, i * tileWidth, 0, tileWidth + i * tileWidth, tileLength, tileWidth, linearizationTable);
        
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

void makeDngFromCFA(void * inBuf, long size, std::string fileName, dng_request_params drp) {
    tinydngwriter::DNGImage di;
    di.SetSubfileType(false, false, false);
    di.SetBigEndian(false);
    di.SetImageWidth(drp.width);
    di.SetImageLength(drp.height);
    di.SetCompression(7);
    
    std::string uniqueCameraModel = drp.manufacturer + "  " + drp.model;
    di.SetUniqueCameraModel(uniqueCameraModel);
    
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
        
    double * cm1 = drp.cameraProfile.colorMatrix1;
    di.SetColorMatrix1(3, cm1);
    
    double * cm2 = drp.cameraProfile.colorMatrix2;
    di.SetColorMatrix2(3, cm2);
    
    double * asn = drp.cameraProfile.asShotNeutral;
    di.SetAsShotNeutral(3, asn);
    
    unsigned char cfaP[4] = {00, 0x1, 0x1, 0x2};
    di.SetCFAPattern(4, cfaP);
    
    unsigned short blL[1] = { (unsigned short)drp.blackLevel };
    di.SetBlackLevel(1, blL);
    
    double whL[1] = { (double)drp.whiteLevel };
    di.SetWhiteLevelRational(1, whL);
    
    di.SetCFARepeatPatternDim(2, 2);
    
    di.SetCustomFieldShort(tinydngwriter::TIFFTAG_CFALAYOUT, 1);
    
    std::vector<unsigned char> tilesMemoryBlock;
    std::vector<long> tilesSizes;
        
    
    int tileWidth = drp.width;
    int tileLength = drp.height;
    getTiles((uint16_t *) inBuf, size, &tilesMemoryBlock, &tilesSizes, tileWidth, tileLength, drp.cameraProfile.linearizationTable);

    di.SetTilesData(tileWidth, tileLength, &tilesMemoryBlock, &tilesSizes);
    
    tinydngwriter::DNGWriter dng_writer(false);
    
    dng_writer.AddImage(&di);
    
    std::string err2;
    dng_writer.WriteToFile(fileName.c_str(), &err2);
    
    std::cout << err2;
}
