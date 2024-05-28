//
//  Created by Stanislav Ratashnyuk on 30/04/2024.
//

#include "dng_bridge.hpp"

int workers = 0;

DngBridge::DngBridge() {
    panasonic::getDefinitionForPanasonic(&cameraProfile);
}

void DngBridge::request_dng(void *data, int dataSize, std::string fileName, dng_request_params drp) {
    
    char* dataForWorker = new char[dataSize];

    std::memcpy(dataForWorker, data, dataSize);
    
    while(workers >= 8) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    drp.cameraProfile = cameraProfile;

    auto f = [this, fileName, dataForWorker, dataSize, drp]() {
    
        makeDngFromCFA(dataForWorker, dataSize, fileName, drp);
                 
        delete[] dataForWorker;
        
        workers--;
    };
    
    std::thread t1(f);
    t1.detach();
    
    workers++;
}

void DngBridge::wait_until_complete() {
    while(workers > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(0));
    }
}
