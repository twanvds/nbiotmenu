//
// Created by Twan van der Schoot on 11-12-17.
//

#ifndef MENU_DEMO_UTIL_H
#define MENU_DEMO_UTIL_H

typedef unsigned int uint;


inline uint modinc(uint v, uint modulus){
    uint a = v+1;
    return (a<modulus)?a:0;
}

inline uint moddec(uint v, uint modulus){
    return (0<v)?v-1:(modulus-1);
}

#endif //MENU_DEMO_UTIL_H


