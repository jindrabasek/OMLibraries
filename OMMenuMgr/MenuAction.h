/*
 * MenuAction.h
 *
 *  Created on: 30. 1. 2016
 *      Author: jindra
 */

#ifndef LIBRARIES_OMMENUMGR_MENUACTION_H_
#define LIBRARIES_OMMENUMGR_MENUACTION_H_

class MenuAction {
public:
    virtual ~MenuAction() {
    }

    virtual void doAction() {
    }
};

#endif /* LIBRARIES_OMMENUMGR_MENUACTION_H_ */
