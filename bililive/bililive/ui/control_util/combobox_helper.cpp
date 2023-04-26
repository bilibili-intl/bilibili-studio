#include "combobox_helper.h"

BililiveComboboxHelper::BililiveComboboxHelper(BililiveComboboxDataOperator* op)
    : BililiveCombobox(op)
{
    set_listener(this);
}

void BililiveComboboxHelper::SetOnIndexChanged(const ComboboxIndexChangedHandler& handler)
{
    indexChangedHandler_ = handler;
}

void BililiveComboboxHelper::OnSelectedIndexChanged(views::Combobox* combobox)
{
    if (indexChangedHandler_)
        indexChangedHandler_(this);
}


BililiveStringComboboxHelper::BililiveStringComboboxHelper()
    : BililiveComboboxHelper(BililiveCreateStringOperator())
{
}

void BililiveStringComboboxHelper::LoadList(const StringItemList& list)
{
    ClearItems();
    for (auto& x : list)
        AddItem(x.first, (int64)&x.second);
    if (GetRowCount() > 0 && selected_index() < 0)
        SetSelectedIndex(0);
}

std::string BililiveStringComboboxHelper::GetSelectedString()
{
    if (GetRowCount() > 0)
    {
        int index = selected_index();
        if (index >= 0)
        {
            auto strptr = GetItemData(index);
            if (strptr)
                return *(const std::string*)(strptr);
            else
                return std::string{};
        }
    }

    return std::string();
}

bool BililiveStringComboboxHelper::SetSelectedString(const std::string& itemdata)
{
    int index = FindItemData((int64)&itemdata);
    if (index >= 0)
    {
        SetSelectedIndex(index);
        return true;
    }
    else
        return false;
}


BililiveIntComboboxHelper::BililiveIntComboboxHelper()
    : BililiveComboboxHelper(BililiveCreateInt64Operator())
{
}

void BililiveIntComboboxHelper::LoadList(const Int64ItemList& list)
{
    ClearItems();
    for (auto& x : list)
        AddItem(x.first, x.second);
    if (GetRowCount() > 0 && selected_index() < 0)
        SetSelectedIndex(0);
}

int64 BililiveIntComboboxHelper::GetSelectedInt()
{
    int index = selected_index();
    if (index >= 0)
        return GetItemData(index);
    else
        return 0;
}

bool BililiveIntComboboxHelper::SetSelectedInt(int64 itemdata)
{
    int index = FindItemData(itemdata);
    if (index >= 0)
    {
        SetSelectedIndex(index);
        return true;
    }
    else
        return false;
}
