/*
 * MenuExitHandler.h
 *
 *  Created on: 30. 1. 2016
 *      Author: jindra
 */

#ifndef LIBRARIES_OMMENUMGR_MENUEXITHANDLER_H_
#define LIBRARIES_OMMENUMGR_MENUEXITHANDLER_H_

class MenuExitHandler {
public:
    virtual ~MenuExitHandler() {
    }

    virtual void exitMenu(bool fullExit) {
    }
    virtual void exitMenuPostCallback() {
    }
};

#endif /* LIBRARIES_OMMENUMGR_MENUEXITHANDLER_H_ */
