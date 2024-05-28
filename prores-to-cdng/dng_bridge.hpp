//
//  Created by Stanislav Ratashnyuk on 30/04/2024.
//

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <thread>
#include <chrono>

#include "make_dng_from_cfa.h"
#include "dng-request-params.h"

#include "camera-definitions/panasonic.h"

typedef uint32_t uint32;

#define qLogJXL true


class DngBridge {
private:
    cameraDefinition cameraProfile;
public:
    DngBridge();
    void request_dng(void *fData, int fDataSize, std::string fileName, dng_request_params drp);
    void make_dng(void *fData, std::string fileName, dng_request_params drp);
    void wait_until_complete();
};
