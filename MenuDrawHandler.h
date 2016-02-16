/*
 * MenuDrawHandler.h
 *
 *  Created on: 30. 1. 2016
 *      Author: jindra
 */

#ifndef LIBRARIES_OMMENUMGR_MENUDRAWHANDLER_H_
#define LIBRARIES_OMMENUMGR_MENUDRAWHANDLER_H_

#include <Arduino.h>

class MenuDrawHandler {
public:
	virtual ~MenuDrawHandler(){}

	virtual void draw(char* p_text, int p_row, int p_col, int len){	Serial.print(F("DISPLAY!!!!!"));}
};

#endif /* LIBRARIES_OMMENUMGR_MENUDRAWHANDLER_H_ */
