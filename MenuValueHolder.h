/*
 * MenuValueHolder.h
 *
 *  Created on: 17. 2. 2016
 *      Author: jindra
 */

#ifndef LIBRARIES_OMMENUMGR_MENUVALUEHOLDER_H_
#define LIBRARIES_OMMENUMGR_MENUVALUEHOLDER_H_

#include <stddef.h>

template <class T> class MenuValueHolder {
private:
	T * value;
public:
	MenuValueHolder() : value(NULL){}

	T * getValuePtr() {
		return value;
	}

	void setValuePtr(T * value) {
		this->value = value;
	}
};

#endif /* LIBRARIES_OMMENUMGR_MENUVALUEHOLDER_H_ */
