/*
 * MenuAction.h
 *
 *  Created on: 30. 1. 2016
 *      Author: jindra
 */

#ifndef LIBRARIES_OMMENUMGR_MENUACTION_H_
#define LIBRARIES_OMMENUMGR_MENUACTION_H_

// Do not define virtual destructor on purpose - class
// and its children is not expected to need destructors,
// it saves a lot of SRAM otherwise occupied by VTABLE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"

class MenuAction {
public:
    virtual void doAction() {
    }
};

#pragma GCC diagnostic pop

#endif /* LIBRARIES_OMMENUMGR_MENUACTION_H_ */
