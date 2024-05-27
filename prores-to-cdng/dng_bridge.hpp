//
//  Created by Stanislav Ratashnyuk on 30/04/2024.
//

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <thread>
#include <chrono>

#include "make_dng_from_cfa.h"

typedef uint32_t uint32;

#define qLogJXL true

struct dng_request_params {
    std::string fileName;
    std::string manufacturer;
    std::string model;
    std::string fNumber;
    std::string irisfNumber;
    std::string shutterSpeed;
    std::string shutterAngle;
    std::string iso;
    std::string lensModel;
    std::string lensAttributes;
    uint32 width;
    uint32 height;
    uint32 blackLevel;
    uint32 whiteLevel;
};

class DngBridge {
public:
    DngBridge();
    void request_dng(void *fData, int fDataSize, std::string fileName, std::string manufacturer, std::string model, std::string fNumber, std::string irisfNumber, std::string shutterSpeed, std::string shutterAngle, std::string iso, std::string lensModel, std::string lensAttributes, uint32 width, uint32 height, uint32 blackLevel, uint32 whiteLevel);
    void make_dng(void *fData, std::string fileName, std::string manufacturer, std::string model, std::string fNumber, std::string irisfNumber, std::string shutterSpeed, std::string shutterAngle, std::string iso, std::string lensModel, std::string lensAttributes, uint32 width, uint32 height, uint32 blackLevel, uint32 whiteLevel);
    void wait_until_complete();
};
