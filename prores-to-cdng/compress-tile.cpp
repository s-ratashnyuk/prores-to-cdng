//
//  Created by Stanislav Ratashnyuk on 22/05/2024.
//

#include "compress-tile.hpp"

long compressTile(uint16_t * srcDataBuf, unsigned char ** outBuf, int tileWidth, int tileHeight) {
    tjhandle tji = tjInitCompress();

    unsigned long outBufSize = 0;
    
    tj3Set(tji, TJPARAM_QUALITY, 90);
    tj3Set(tji, TJPARAM_SUBSAMP, TJSAMP_GRAY);
    tj3Set(tji, TJPARAM_OPTIMIZE, 1);
    tj3Set(tji, TJPARAM_PROGRESSIVE, 1);
    
    tj3Set(tji, TJPARAM_COLORSPACE, TJCS_GRAY);
    
    int pitch = tileWidth;
    
    int result = tj3Compress12(tji, (const short *)srcDataBuf, tileWidth, pitch, tileHeight, TJPF_GRAY, outBuf, &outBufSize);
    
    if (result != 0) {
        auto cmpErr = tj3GetErrorStr(tji);
        std::cout << cmpErr;
    }
   
    tj3Destroy(tji);

    return outBufSize;
}
