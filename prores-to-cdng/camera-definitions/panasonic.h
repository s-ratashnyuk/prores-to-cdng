//
//  panasonic.h
//  prores-to-cdng
//
//  Created by Stanislav Ratashnyuk on 28/05/2024.
//

#ifndef panasonic_h
#define panasonic_h

#include "camera-definition.h"

namespace panasonic {
    uint16_t p_linearizeValue(uint16_t value) {
        double a = 0.0039;
        double b = -0.0078;
        double c = -0.4634 - value;
        
        double disr = b * b - a * c * 4;
        
        double frup = b * -1 + sqrt(disr);
        double frdn = a * 2;
        
        double x = frup / frdn;
        uint16_t xi = abs(ceil(x));
        
        return xi;
    }

    void p_makeLinearizationTable(uint16_t result[]) {
        for(int i = 1; i <= 65535; i++) {
            result[i-1] = p_linearizeValue(i);
        }
    }

    void getDefinitionForPanasonic(cameraDefinition * cd_panasonic) {
        
        cd_panasonic->profileName = "panasonic";
        
        int c = 0;
        for(auto value: {0.94519, -0.24292, -0.0788574, -0.494141, 1.12805, 0.0595703, -0.00927734, 0.0153809, 0.220337}) {
            cd_panasonic->colorMatrix1[c] = value;
            c++;
        }
        
        c = 0;
        for(auto value: {0.493408, -0.0946045, -0.0467529, -0.400269, 0.959595, 0.132446, 0.013916, -0.0236816, 0.426147}) {
            cd_panasonic->colorMatrix2[c] = value;
            c++;
        }
        
        c = 0;
        for(double value: {0.494629, 1.0, 0.568237}) {
            cd_panasonic->asShotNeutral[c] = value;
            c++;
        }
                
         p_makeLinearizationTable(cd_panasonic->linearizationTable);
    };
}




#endif /* panasonic_h */
