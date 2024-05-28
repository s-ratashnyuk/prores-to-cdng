//
//  camera-definition.h
//  prores-to-cdng
//
//  Created by Stanislav Ratashnyuk on 28/05/2024.
//

#ifndef camera_definition_h
#define camera_definition_h

struct cameraDefinition {
    std::string profileName;
    double colorMatrix1[9];
    double colorMatrix2[9];
    double asShotNeutral[9];
    uint16_t linearizationTable[65535];
};

#endif /* camera_definition_h */
