//
//  Created by Stanislav Ratashnyuk on 30/04/2024.
//

#include "dng_bridge.hpp"

int workers = 0;

DngBridge::DngBridge() {

}

void DngBridge::request_dng(void *data, int dataSize, std::string fileName, std::string manufacturer, std::string model, std::string fNumber, std::string irisfNumber, std::string shutterSpeed, std::string shutterAngle, std::string iso, std::string lensModel, std::string lensAttributes, uint32 width, uint32 height, uint32 blackLevel, uint32 whiteLevel) {
    
    char* dataForWorker = new char[dataSize];

    std::memcpy(dataForWorker, data, dataSize);
    
    while(workers >= 8) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    auto f = [this, fileName, dataForWorker, dataSize, manufacturer, model, fNumber, irisfNumber, shutterSpeed, shutterAngle, iso, lensModel, lensAttributes, width, height, blackLevel, whiteLevel]() {
    
        makeDngFromCFA(dataForWorker, dataSize, fileName);
                 
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
