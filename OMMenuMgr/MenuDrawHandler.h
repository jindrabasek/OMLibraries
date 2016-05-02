/*
 * MenuDrawHandler.h
 *
 *  Created on: 30. 1. 2016
 *      Author: jindra
 */

#ifndef LIBRARIES_OMMENUMGR_MENUDRAWHANDLER_H_
#define LIBRARIES_OMMENUMGR_MENUDRAWHANDLER_H_

#include <stdint.h>

class MenuDrawHandler {
public:
    virtual ~MenuDrawHandler() {
    }

    virtual void draw(char* p_text, uint8_t p_row, uint8_t p_col, uint8_t len) {
    }
};

#endif /* LIBRARIES_OMMENUMGR_MENUDRAWHANDLER_H_ */
