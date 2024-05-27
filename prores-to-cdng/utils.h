//
//  Created by Stanislav Ratashnyuk on 22/05/2024.
//

#ifndef utils_h
#define utils_h

long readBinaryFile(std::string fileName, void * &outData) {
    FILE * inFile;
    
    inFile = fopen(fileName.c_str(), "rb");
    
    long size = 0;
    fseek(inFile, 0, SEEK_END);
    size = ftell(inFile);
    fseek(inFile, 0, SEEK_SET);
    
    outData = malloc(size);
    fread(outData, size, 1, inFile);
    
    fclose(inFile);
    
    return size;
}

uint16_t swapBytes(uint16_t src) {
    auto hibyte = (src & 0xff00) >> 8;
    auto lobyte = (src & 0xff);
    
    return lobyte << 8 | hibyte;
}

#endif /* utils_h */
