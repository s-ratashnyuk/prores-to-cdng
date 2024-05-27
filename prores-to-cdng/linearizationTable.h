//
//  Created by Stanislav Ratashnyuk on 22/05/2024.
//

#ifndef linearizationTable_h
#define linearizationTable_h

uint16_t linearizeValue(uint16_t value) {
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

uint16_t * getLinearizationTable() {
    static uint16_t result[65535];
    
    for(int i = 1; i <= 65535; i++) {
        result[i] = linearizeValue(i);
    }
    
    return result;
}

#endif /* linearizationTable_h */
