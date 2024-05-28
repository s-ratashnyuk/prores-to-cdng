//
//  dng-request-params.h
//  prores-to-cdng
//
//  Created by Stanislav Ratashnyuk on 28/05/2024.
//

#ifndef dng_request_params_h
#define dng_request_params_h

#include "camera-definitions/camera-definition.h"

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
    uint32_t width;
    uint32_t height;
    uint32_t blackLevel;
    uint32_t whiteLevel;
    cameraDefinition cameraProfile;
};

#endif /* dng_request_params_h */
