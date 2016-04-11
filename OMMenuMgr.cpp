/*

 Generalized Menu Library

 OpenMoco MoCoBus Core Libraries

 See www.dynamicperception.com for more information

 (c) 2008-2012 C.A. Church / Dynamic Perception LLC

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.


 */

#include "OMMenuMgr.h"
#include "MenuValueHolder.h"

/** Constructor

 Constructs an OMMenuMgr instance, with a specified root item, and a
 specified input type.

 @code
 #include "OMMenuMgr.h"

 ...

 OMMenuMgr Menu = OMMenuMgr(&rootItem, MENU_ANALOG);
 @endcode

 @param c_first
 A pointer to an OMMenuItem representing the root of the menu

 @param c_type
 The input type for the menu, either MENU_ANALOG or MENU_DIGITAL

 */

//#define MENU_DBG
#ifdef MENU_DBG
#define DBG_P(x) Serial.print(F(#x)); Serial.print(F(" = ")); Serial.println(reinterpret_cast<uint16_t>(x)); delay(100);
#define DBG(x) Serial.print(F(#x)); Serial.print(F(" = ")); Serial.println(x); delay(100);
#else
#define DBG_P(x) //
#define DBG(x) //
#endif

OMMenuMgr::OMMenuMgr(const OMMenuItem* c_first) {

    m_curParent = const_cast<OMMenuItem*>(c_first);
    m_rootItem = const_cast<OMMenuItem*>(c_first);
    m_inEdit = false;
    m_curSel = 0;
    m_curTarget = 0;
    memset(m_hist, 0, sizeof(OMMenuItem*) * sizeof(m_hist));

}

void OMMenuMgr::display(char* p_str, int p_row, int p_col, int p_count,
                        MenuDrawHandler & drawHandler) {
    drawHandler.draw(p_str, p_row, p_col, p_count);
}

// What happens when a button is pressed? Handle this activity

void OMMenuMgr::handleMenu(Button p_key, MenuDrawHandler & drawHandler,
                           MenuExitHandler & exitHandler) {
    m_inMenu = true;

    DBG(m_inMenu);

    if (p_key == BUTTON_SELECT /* || p_key == BUTTON_FORWARD*/) {

        if (m_inEdit) {
            edit(m_curSel, CHANGE_SAVE, exitHandler, drawHandler);
        } else {
            if (m_curSel != 0)
                activate(m_curSel, exitHandler, drawHandler);
            else
                activate(m_rootItem, exitHandler, drawHandler);

        }
    } else if (p_key == BUTTON_NONE) {
        displayList(m_curParent, drawHandler, m_curTarget);
    } else if (p_key == BUTTON_FORWARD) {
        // do nothing
    } else {
        MenuChangeType changeType =
                (p_key == BUTTON_INCREASE) ? CHANGE_UP :
                (p_key == BUTTON_DECREASE) ? CHANGE_DOWN : CHANGE_ABORT; // BUTTON_BACK is CHANGE_ABORT

        if (m_inEdit)
            edit(m_curSel, changeType, exitHandler, drawHandler);
        else
            menuNav(changeType, exitHandler, drawHandler);
    }

}

// exiting the menu entirely

void OMMenuMgr::exitMenu(MenuExitHandler & exitHandler) {
    m_curParent = m_rootItem;
    m_curTarget = 0;

    m_inMenu = false;
    DBG (m_inMenu);
    exitHandler.exitMenu(true);
    exitHandler.exitMenuPostCallback();
}

// navigating through menus

void OMMenuMgr::menuNav(MenuChangeType p_mode, MenuExitHandler & exitHandler,
                        MenuDrawHandler & drawHandler) {

    if (p_mode == CHANGE_ABORT) {

        // exiting this menu level

        m_curSel = 0;

        // get previous menu level
        OMMenuItem* newItem = popHist();

        if (newItem == 0) {
            // aborting at root
            exitMenu(exitHandler);
        } else {
            m_curParent = newItem;
            m_curTarget = 0;
            activate(m_curParent, exitHandler, drawHandler, true);
        }

    } else {

        uint8_t childCount = pgm_read_byte(&(m_curParent->targetCount));
        childCount--;

        m_curTarget += p_mode == CHANGE_UP ? -1 : 1;
        m_curTarget =
                (m_curTarget > childCount && m_curTarget < 255) ? 0 :
                        m_curTarget;
        m_curTarget = m_curTarget == 255 ? childCount : m_curTarget;
        displayList(m_curParent, drawHandler, m_curTarget);
        /*

         m_curTarget += p_mode == CHANGE_UP ? -1 : 1;
         m_curTarget = m_curTarget > childCount ? childCount : m_curTarget;
         _displayList(m_curParent, m_curTarget);
         */
    }

}

// activating a menu item
void OMMenuMgr::activate(OMMenuItem* p_item, MenuExitHandler & exitHandler,
                         MenuDrawHandler & drawHandler, bool p_return) {

    // get item type
    MenuItemType type = (MenuItemType) pgm_read_byte(&(p_item->type));

    // process activation based on type
    if (type == ITEM_VALUE || type == ITEM_VALUE_WITH_CALLBACK) {
        m_inEdit = true;
        displayEdit(p_item, drawHandler, type == ITEM_VALUE_WITH_CALLBACK);
    } else if (type == ITEM_MENU) {
        // sub-menu

        if (!p_return && p_item != m_curParent) {
            pushHist(m_curParent);
            m_curParent = p_item;
            m_curTarget = 0;
        }

        displayList(p_item, drawHandler, m_curTarget);
    } else if (type == ITEM_ACTION) {
        // this is gnarly, dig? We're pulling a function pointer
        // out of progmem
        MenuAction * callback =
                reinterpret_cast<MenuValueHolder<MenuAction>*>(reinterpret_cast<void*>(pgm_read_word(
                        &(p_item->target))))->getValuePtr();

        if (callback != NULL) {
            callback->doAction();
        }

        displayList(m_curParent, drawHandler, m_curTarget);

    } else if (type == ITEM_SCREEN) {

        exitHandler.exitMenu(false);

        // this is gnarly, dig? We're pulling a function pointer
        // out of progmem
        MenuAction * callback =
                reinterpret_cast<MenuValueHolder<MenuAction>*>(reinterpret_cast<void*>(pgm_read_word(
                        &(p_item->target))))->getValuePtr();

        if (callback != NULL) {
            callback->doAction();
        }

        exitHandler.exitMenuPostCallback();
    }
}

// Display list rows on the screen

void OMMenuMgr::displayList(OMMenuItem* p_item, MenuDrawHandler & drawHandler,
                            uint8_t p_target) {

    uint8_t childCount = pgm_read_byte(&(p_item->targetCount));
    childCount--;

    uint8_t selectorPosition = p_target % OM_MENU_ROWS;
    uint8_t menuPage = p_target / OM_MENU_ROWS;

    OMMenuItem** items =
            reinterpret_cast<OMMenuItem**>(reinterpret_cast<void*>(pgm_read_word(
                    &(p_item->target))));

    m_curSel = reinterpret_cast<OMMenuItem*>(pgm_read_word(&(items[p_target])));

    // loop through display rows
    for (byte i = 0; i < OM_MENU_ROWS; i++) {
        // flush buffer
        memset(m_dispBuf, ' ', sizeof(char) * sizeof(m_dispBuf));

        int startX = 0;

        // display cursor on row
        if (i == selectorPosition) {
            int dispCur = sizeof(char) * sizeof(OM_MENU_CURSOR) - 1;
            display((char*) OM_MENU_CURSOR, i, 0, dispCur, drawHandler);
            startX += dispCur;
        } else {
            int dispCur = sizeof(char) * sizeof(OM_MENU_CURSOR) - 1;
            display((char*) OM_MENU_NOCURSOR, i, 0, dispCur, drawHandler);
            startX += dispCur;
        }

        // if there's actually an item here, copy it to the display buffer
        if (childCount >= (menuPage * OM_MENU_ROWS) + i) {
            // OMMenuItem* item = reinterpret_cast<OMMenuItem*>( pgm_read_word(p_item->target.items[p_target + i]) );
            OMMenuItem* item = reinterpret_cast<OMMenuItem*>(pgm_read_word(
                    &(items[(menuPage * OM_MENU_ROWS) + i])));
            memcpy_P(m_dispBuf, &(item->label),
                    sizeof(char) * sizeof(m_dispBuf));
        }

        display(m_dispBuf, i, startX, OM_MENU_COLS - startX, drawHandler);
    }

}

// display a value to be edited

void OMMenuMgr::displayEdit(OMMenuItem* p_item, MenuDrawHandler & drawHandler,
                            bool withCallback) {

    // display label

    // copy data to buffer from progmem
    memcpy_P(m_dispBuf, &(p_item->label), sizeof(char) * sizeof(m_dispBuf));

    display(m_dispBuf, 0, 0, OM_MENU_COLS, drawHandler);

    OMMenuValue* value =
            reinterpret_cast<OMMenuValue*>(reinterpret_cast<void*>(pgm_read_word(
                    &(p_item->target))));

    MenuEditType type = (MenuEditType) pgm_read_byte(&(value->type));

    void* valPtr;
    if (withCallback) {
        OMMenuValueAndAction * valueAndAction =
                reinterpret_cast<OMMenuValueAndAction*>(pgm_read_word(
                        &(value->targetValue)));
        valPtr = reinterpret_cast<void*>(pgm_read_word(
                &(valueAndAction->targetValue)));
    } else {
        valPtr = reinterpret_cast<void*>(pgm_read_word(&(value->targetValue)));
    }

    void* unwrappedValPtr = NULL;

    // get current value and squirrel it away for now
    // we use temporary variables to avoid all sorts of crazyness should
    // we want to port to another platform later, and endianness would
    // be a problem if we decided to use one large variable (ulong) and
    // then use specific bytes.  This takes more memory, but it seems
    // a viable trade

    bool numberEdit = true;

    if (type == TYPE_BYTE) {
        unwrappedValPtr = reinterpret_cast<MenuValueHolder<uint8_t>*>(valPtr)
                ->getValuePtr();
        m_temp = *reinterpret_cast<MenuValueHolder<uint8_t>*>(valPtr)
                ->getValuePtr();
    } else if (type == TYPE_UINT || type == TYPE_INT) {
        unwrappedValPtr = reinterpret_cast<MenuValueHolder<int>*>(valPtr)
                ->getValuePtr();
        m_tempI =
                *reinterpret_cast<MenuValueHolder<int>*>(valPtr)->getValuePtr();
    } else if (type == TYPE_LONG || type == TYPE_ULONG) {
        unwrappedValPtr = reinterpret_cast<MenuValueHolder<long>*>(valPtr)
                ->getValuePtr();
        m_tempL =
                *reinterpret_cast<MenuValueHolder<long>*>(valPtr)->getValuePtr();
    } else if (type == TYPE_SELECT) {
        // select types are interesting...  We have a list of possible values and
        // labels - we need to work that back to an index in the list...
        OMMenuSelectValue* sel = reinterpret_cast<OMMenuSelectValue*>(valPtr);
        unwrappedValPtr = valPtr;
        OMMenuSelectListItem** list =
                reinterpret_cast<OMMenuSelectListItem**>(reinterpret_cast<void*>(pgm_read_word(
                        &(sel->list))));
        uint8_t curVal =
                *(reinterpret_cast<MenuValueHolder<uint8_t>*>(pgm_read_word(
                        &(sel->targetValue))))->getValuePtr();
        uint8_t count = pgm_read_byte(&(sel->listCount));

        // mark to first index by default
        m_temp = 0;

        // find index of current assigned value (if can be found)
        for (uint8_t i = 0; i < count; i++) {
            OMMenuSelectListItem* item =
                    reinterpret_cast<OMMenuSelectListItem*>(pgm_read_word(
                            &(list[i])));
            uint8_t tgt = pgm_read_byte(&(item->value));
            if (tgt == curVal)
                m_temp = i;
        }

        displaySelVal(list, m_temp, drawHandler);
        numberEdit = false;
    } else if (type == TYPE_BFLAG) {
        OMMenuValueFlag* flag = reinterpret_cast<OMMenuValueFlag*>(valPtr);
        unwrappedValPtr = valPtr;
        uint8_t* target =
                reinterpret_cast<MenuValueHolder<uint8_t>*>(pgm_read_word(
                        &(flag->flag)))->getValuePtr();
        uint8_t pos = pgm_read_byte(&(flag->pos));

        if (*target & (1 << pos))
            m_temp = 1;
        else
            m_temp = 0;

        displayFlagVal(drawHandler);
        numberEdit = false;
    } else if (type >= TYPE_FLOAT) { // always run as last check
        unwrappedValPtr = reinterpret_cast<MenuValueHolder<float>*>(valPtr)
                ->getValuePtr();
        m_tempF = *reinterpret_cast<MenuValueHolder<float>*>(valPtr)
                ->getValuePtr();
    }

    if (numberEdit) {
        // throw number on-screen
        displayVoidNum(unwrappedValPtr, type, 1, 0, drawHandler);
    }

    memset(m_dispBuf, ' ', sizeof(char) * sizeof(m_dispBuf));
    // clear remaining lines
    for (byte i = 2; i < OM_MENU_ROWS; i++) {
        display(m_dispBuf, i, 0, OM_MENU_COLS, drawHandler);
    }
}

// rationalize a way to display any sort of number as a char*, rationalize it buddy, rationalize it good...

void OMMenuMgr::displayVoidNum(void* p_ptr, MenuEditType p_type, int p_row,
                               int p_col, MenuDrawHandler & drawHandler) {

    // clear out display buffer
    memset(m_dispBuf, ' ', sizeof(char) * sizeof(m_dispBuf));

    // handle variable precision for float nums
    byte floatPrecision = 1;

    if (p_type == TYPE_FLOAT_100)
        floatPrecision = 2;
    else if (p_type == TYPE_FLOAT_1000)
        floatPrecision = 3;

    if (p_type == TYPE_BYTE)
        utoa(*reinterpret_cast<uint8_t*>(p_ptr), m_dispBuf, 10);
    else if (p_type == TYPE_INT)
        itoa(*reinterpret_cast<int*>(p_ptr), m_dispBuf, 10);
    else if (p_type == TYPE_UINT)
        utoa(*reinterpret_cast<unsigned int*>(p_ptr), m_dispBuf, 10);
    else if (p_type == TYPE_LONG)
        ltoa(*reinterpret_cast<long*>(p_ptr), m_dispBuf, 10);
    else if (p_type == TYPE_ULONG)
        ultoa(*reinterpret_cast<unsigned long*>(p_ptr), m_dispBuf, 10);
    else if (p_type >= TYPE_FLOAT)
        dtostrf(*reinterpret_cast<float*>(p_ptr), floatPrecision + 2,
                floatPrecision, m_dispBuf);

    display(m_dispBuf, p_row, p_col, sizeof(char) * sizeof(m_dispBuf),
            drawHandler);

}

// handle modifying temp values based on their type

void OMMenuMgr::modifyTemp(MenuEditType p_type, MenuEditMode p_mode, long p_min,
                           long p_max, MenuDrawHandler & drawHandler) {

    void* tempNum;

    int mod = (p_mode == MODE_INCREMENT) ? 1 : -1;

    // apply holding rate mod
    //mod *= m_holdMod;

    // handle float precision adjustment
    float fmod = (float) mod;

    if (p_type == TYPE_FLOAT_10)
        fmod /= 10.0;
    else if (p_type == TYPE_FLOAT_100)
        fmod /= 100.0;
    else if (p_type == TYPE_FLOAT_1000)
        fmod /= 1000.0;

    // manage correct temporary variable change based on type
    // and apply floor/ceiling from min/max

    if (p_type == TYPE_BYTE) {
        m_temp += mod;
        if (p_min != 0 || p_max != 0)
            m_temp = m_temp > p_max ? p_min : (m_temp < p_min ? p_max : m_temp);
        tempNum = reinterpret_cast<void*>(&m_temp);
    } else if (p_type == TYPE_INT) {
        m_tempI += mod;
        if (p_min != 0 || p_max != 0)
            m_tempI =
                    m_tempI > p_max ? p_min :
                            (m_tempI < p_min ? p_max : m_tempI);
        tempNum = reinterpret_cast<void*>(&m_tempI);
    } else if (p_type == TYPE_UINT) {
        *reinterpret_cast<unsigned int*>(&m_tempI) += mod;
        if (p_min != 0 || p_max != 0)
            m_tempI =
                    m_tempI > p_max ? p_min :
                            (m_tempI < p_min ? p_max : m_tempI);
        tempNum = reinterpret_cast<void*>(&m_tempI);
    } else if (p_type == TYPE_LONG) {
        m_tempL += mod;
        if (p_min != 0 || p_max != 0)
            m_tempL =
                    m_tempL > p_max ? p_min :
                            (m_tempL < p_min ? p_max : m_tempL);
        tempNum = reinterpret_cast<void*>(&m_tempL);
    } else if (p_type == TYPE_ULONG) {
        *reinterpret_cast<unsigned long*>(&m_tempL) += mod;
        if (p_min != 0 || p_max != 0)
            m_tempL =
                    m_tempL > p_max ? p_min :
                            (m_tempL < p_min ? p_max : m_tempL);
        tempNum = reinterpret_cast<void*>(&m_tempL);
    } else if (p_type >= TYPE_FLOAT) {
        m_tempF += fmod;
        if (p_min != 0 || p_max != 0)
            m_tempF =
                    m_tempF > p_max ? p_min :
                            (m_tempF < p_min ? p_max : m_tempF);
        tempNum = reinterpret_cast<void*>(&m_tempF);
    }

    // display new temporary value
    displayVoidNum(tempNum, p_type, 1, 0, drawHandler);

}

// display the label value from a select list on the screen

void OMMenuMgr::displaySelVal(OMMenuSelectListItem** p_list, uint8_t p_idx,
                              MenuDrawHandler & drawHandler) {

    // clear out display buffer
    memset(m_dispBuf, ' ', sizeof(char) * sizeof(m_dispBuf));

    // get the actual select list item
    OMMenuSelectListItem* item =
            reinterpret_cast<OMMenuSelectListItem*>(pgm_read_word(
                    &(p_list[p_idx])));

    // copy the label contents from flash to the buffer
    memcpy_P(m_dispBuf, &(item->label), sizeof(char) * OM_MENU_LBLLEN);

    // display the buffer
    display(m_dispBuf, 1, 0, sizeof(char) * sizeof(m_dispBuf), drawHandler);

}

// modifying a select type value - we cycle through a list of
// values by cycling list index...

void OMMenuMgr::modifySel(OMMenuValue* p_value, MenuEditMode p_mode,
                          MenuDrawHandler & drawHandler, bool withCallback) {

    OMMenuSelectValue* sel;
    if (withCallback) {
        OMMenuValueAndAction * valueAndAction =
                reinterpret_cast<OMMenuValueAndAction*>(pgm_read_word(
                        &(p_value->targetValue)));
        sel = reinterpret_cast<OMMenuSelectValue*>(pgm_read_word(
                &(valueAndAction->targetValue)));
    } else {
        sel = reinterpret_cast<OMMenuSelectValue*>(pgm_read_word(
                &(p_value->targetValue)));
    }

    uint8_t count = pgm_read_byte(&(sel->listCount));
    OMMenuSelectListItem** list =
            reinterpret_cast<OMMenuSelectListItem**>(reinterpret_cast<void*>(pgm_read_word(
                    &(sel->list))));

    count--;

    if (p_mode == MODE_DECREMENT) {
        if (m_temp == 0)
            m_temp = count;
        else
            m_temp--;
    } else if (p_mode == MODE_INCREMENT) {
        if (m_temp == count)
            m_temp = 0;
        else
            m_temp++;
    }

    displaySelVal(list, m_temp, drawHandler);

}

// displaying flag parameters

void OMMenuMgr::displayFlagVal(MenuDrawHandler & drawHandler) {

    // overwrite buffer
    memset(m_dispBuf, ' ', sizeof(char) * sizeof(m_dispBuf));

    if (m_temp == 1)
        memcpy(m_dispBuf, (char*) OM_MENU_FLAG_ON,
                sizeof(char) * sizeof(OM_MENU_FLAG_ON) - 1);
    else
        memcpy(m_dispBuf, (char*) OM_MENU_FLAG_OFF,
                sizeof(char) * sizeof(OM_MENU_FLAG_OFF) - 1);

    display(m_dispBuf, 1, 0, sizeof(char) * sizeof(m_dispBuf), drawHandler);

}

// edit operations against a displayed value

void OMMenuMgr::edit(OMMenuItem* p_item, MenuChangeType p_type,
                     MenuExitHandler & exitHandler,
                     MenuDrawHandler & drawHandler) {

    // get item type
    MenuItemType itemType = (MenuItemType) pgm_read_byte(&(p_item->type));
    bool withCallback = itemType == ITEM_VALUE_WITH_CALLBACK;

    OMMenuValue* thisValue = reinterpret_cast<OMMenuValue*>(pgm_read_word(
            &(p_item->target)));

    MenuEditType type = (MenuEditType) pgm_read_byte(&(thisValue->type));
    long min = pgm_read_dword(&(thisValue->min));
    long max = pgm_read_dword(&(thisValue->max));

    MenuEditMode mode = (p_type == CHANGE_UP) ? MODE_INCREMENT : MODE_DECREMENT;

    if (p_type == CHANGE_ABORT) {
        m_inEdit = false;
        activate(m_curParent, exitHandler, drawHandler, true);
    }

    else if (p_type == CHANGE_UP || p_type == CHANGE_DOWN) {
        if (type == TYPE_SELECT) {
            modifySel(thisValue, mode, drawHandler, withCallback);
        } else if (type == TYPE_BFLAG) {
            m_temp = (m_temp == 1) ? 0 : 1;
            displayFlagVal(drawHandler);
        } else
            modifyTemp(type, mode, min, max, drawHandler);
    }

    else if (p_type == CHANGE_SAVE) {

        void* ptr;
        if (withCallback) {
            OMMenuValueAndAction * valueAndAction =
                    reinterpret_cast<OMMenuValueAndAction*>(pgm_read_word(
                            &(thisValue->targetValue)));
            ptr = reinterpret_cast<void*>(pgm_read_word(
                    &(valueAndAction->targetValue)));
        } else {
            ptr = reinterpret_cast<void*>(pgm_read_word(
                    &(thisValue->targetValue)));
        }

        if (type == TYPE_SELECT) {
            // select is more special than the rest, dig?

            // some what convoluted - we get the value stored in the current index (in m_temp) from the list,
            // and store it in the byte pointer provided attached to the OMMenuSelectValue
            OMMenuSelectValue* sel = reinterpret_cast<OMMenuSelectValue*>(ptr);
            OMMenuSelectListItem** list =
                    reinterpret_cast<OMMenuSelectListItem**>(reinterpret_cast<void*>(pgm_read_word(
                            &(sel->list))));
            OMMenuSelectListItem* item =
                    reinterpret_cast<OMMenuSelectListItem*>(pgm_read_word(
                            &(list[m_temp])));
            uint8_t newVal = pgm_read_byte(&(item->value));
            uint8_t* real =
                    (reinterpret_cast<MenuValueHolder<uint8_t>*>(pgm_read_word(
                            &(sel->targetValue))))->getValuePtr();
            *real = newVal;

            _eewrite<uint8_t>(thisValue, newVal);
        } else if (type == TYPE_BFLAG) {
            // bflag is special too, we want to set a specific bit based on the current temp value
            OMMenuValueFlag* flag = reinterpret_cast<OMMenuValueFlag*>(ptr);
            uint8_t* target =
                    reinterpret_cast<MenuValueHolder<uint8_t>*>(pgm_read_word(
                            &(flag->flag)))->getValuePtr();
            uint8_t pos = pgm_read_byte(&(flag->pos));

            if (m_temp)
                *target |= (1 << pos);
            else
                *target &= (0xFF ^ (1 << pos));

            _eewrite<uint8_t>(thisValue, *target);
        } else if (type == TYPE_BYTE) {
            *reinterpret_cast<MenuValueHolder<uint8_t>*>(ptr)->getValuePtr() =
                    m_temp;
            _eewrite<uint8_t>(thisValue,
                    *reinterpret_cast<MenuValueHolder<uint8_t>*>(ptr)
                            ->getValuePtr());
        } else if (type == TYPE_UINT) {
            *reinterpret_cast<MenuValueHolder<unsigned int>*>(ptr)->getValuePtr() =
                    *reinterpret_cast<unsigned int*>(&m_tempI);
            _eewrite<unsigned int>(thisValue,
                    *reinterpret_cast<MenuValueHolder<unsigned int>*>(ptr)
                            ->getValuePtr());
        } else if (type == TYPE_INT) {
            *reinterpret_cast<MenuValueHolder<int>*>(ptr)->getValuePtr() =
                    m_tempI;
            _eewrite<int>(thisValue,
                    *reinterpret_cast<MenuValueHolder<int>*>(ptr)->getValuePtr());
        } else if (type == TYPE_ULONG) {
            *reinterpret_cast<MenuValueHolder<unsigned long>*>(ptr)->getValuePtr() =
                    *reinterpret_cast<unsigned long*>(&m_tempL);
            _eewrite<unsigned long>(thisValue,
                    *reinterpret_cast<MenuValueHolder<unsigned long>*>(ptr)
                            ->getValuePtr());
        } else if (type == TYPE_LONG) {
            *reinterpret_cast<MenuValueHolder<long>*>(ptr)->getValuePtr() =
                    m_tempL;
            _eewrite<long>(thisValue,
                    *reinterpret_cast<MenuValueHolder<long>*>(ptr)->getValuePtr());
        } else if (type >= TYPE_FLOAT) {
            *reinterpret_cast<MenuValueHolder<float>*>(ptr)->getValuePtr() =
                    m_tempF;
            _eewrite<float>(thisValue,
                    *reinterpret_cast<MenuValueHolder<float>*>(ptr)->getValuePtr());
        }

        if (withCallback) {
            OMMenuValueAndAction * valueAndAction =
                    reinterpret_cast<OMMenuValueAndAction*>(pgm_read_word(
                            &(thisValue->targetValue)));
            MenuAction * callback =
                    reinterpret_cast<MenuValueHolder<MenuAction>*>(reinterpret_cast<void*>(pgm_read_word(
                            &(valueAndAction->targetAction))))->getValuePtr();

            if (callback != NULL) {
                callback->doAction();
            }
        }

        m_inEdit = false;
        activate(m_curParent, exitHandler, drawHandler, true);
    }
}

// add a menu level to the history

void OMMenuMgr::pushHist(OMMenuItem* p_item) {
    // note that if you have no room left, you'll lose this
    // item - we only store up to MAXDEPTH
    for (uint8_t i = 0; i < OM_MENU_MAXDEPTH; i++) {
        if (m_hist[i] == 0) {
            m_hist[i] = p_item;
            return;
        }
    }
}

// remove the latest menu item from the history and return it

OMMenuItem* OMMenuMgr::popHist() {
    // work backwards, remove the first non-zero pointer
    // and return it
    for (uint8_t i = OM_MENU_MAXDEPTH; i > 0; i--) {
        if (m_hist[i - 1] != 0) {
            OMMenuItem* item = m_hist[i - 1];
            m_hist[i - 1] = 0;
            return item;
        }
    }

    return 0;
}
