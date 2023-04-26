#ifndef CONTROL_UTILS_COMBOBOX_HELPER_H_
#define CONTROL_UTILS_COMBOBOX_HELPER_H_

#include <vector>
#include <utility>
#include <string>
#include <functional>
#include "bililive/bililive/ui/views/controls/combox/bililive_combobox.h"

class BililiveComboboxHelper;

class BililiveComboboxHelper;
using StringItemList = std::vector < std::pair<std::wstring, std::string> > ;
using Int64ItemList = std::vector < std::pair<std::wstring, int64> > ;
using ComboboxIndexChangedHandler = std::function < void(BililiveComboboxHelper*) > ;

class BililiveComboboxHelper : public BililiveCombobox, public views::ComboboxListener
{
public:
    void SetOnIndexChanged(const ComboboxIndexChangedHandler& handler);

protected:
    BililiveComboboxHelper(BililiveComboboxDataOperator* op);
    void OnSelectedIndexChanged(views::Combobox* combobox) override;

private:
    ComboboxIndexChangedHandler indexChangedHandler_;
};

class BililiveStringComboboxHelper : public BililiveComboboxHelper
{
public:
    BililiveStringComboboxHelper();

    void LoadList(const StringItemList& list);
    std::string GetSelectedString();
    bool SetSelectedString(const std::string& itemdata);
};

class BililiveIntComboboxHelper : public BililiveComboboxHelper
{
public:
    BililiveIntComboboxHelper();

    void LoadList(const Int64ItemList& list);
    int64 GetSelectedInt();
    bool SetSelectedInt(int64 itemdata);
};

#endif
