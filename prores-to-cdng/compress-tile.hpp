//
//  Created by Stanislav Ratashnyuk on 22/05/2024.
//

#ifndef compress_tile_hpp
#define compress_tile_hpp

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "turbojpeg.h"

long compressTile(uint16_t * srcDataBuf, unsigned char ** outBuf, int tileWidth, int tileHeight);

#endif /* compress_tile_hpp */
