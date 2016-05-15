/*
 * MenuDrawHandler.h
 *
 *  Created on: 30. 1. 2016
 *      Author: jindra
 */

#ifndef LIBRARIES_OMMENUMGR_MENUDRAWHANDLER_H_
#define LIBRARIES_OMMENUMGR_MENUDRAWHANDLER_H_

#include <stdint.h>

// Do not define virtual destructor on purpose - class
// and its children is not expected to need destructors,
// it saves a lot of SRAM otherwise occupied by VTABLE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"

class MenuDrawHandler {
public:
    virtual void draw(char* p_text, uint8_t p_row, uint8_t p_col, uint8_t len) {
    }
};

#pragma GCC diagnostic pop

#endif /* LIBRARIES_OMMENUMGR_MENUDRAWHANDLER_H_ */
