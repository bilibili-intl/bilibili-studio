#include "bililive_editable_combobox.h"

namespace bililive
{
	BililiveEditableCombobox::BililiveEditableCombobox(BililiveComboboxDataOperator* op)
		: BililiveCombobox(op)
	{
		SetEditAble(true);
		SetTextInputType(ui::TEXT_INPUT_TYPE_TEXT);
	}

	BililiveEditableCombobox::~BililiveEditableCombobox()
	{
	}

	void BililiveEditableCombobox::OnTextfieldBlur()
	{
		views::Textfield *field = textfield();
		if (field)
		{
			std::wstring currentText = textfield()->text();
			SetCurrentText(currentText);

			field->SetVisible(false);
		}
	}

	void BililiveEditableCombobox::OnTextfieldKeyPressed(const ui::KeyEvent& ke)
	{
		if (ke.key_code() == ui::KeyboardCode::VKEY_RETURN)
		{
			views::Textfield *field = textfield();
			if (field)
			{
				OnTextfieldBlur();
			}
		}
	}

	void BililiveEditableCombobox::SetCurrentText(const std::wstring& text)
	{
		std::wstring currentText = textfield()->text();
		int index = FindItem(currentText);
		if (index >= 0)
		{
			SetSelectedIndex(index);
		}
		else
		{
			int64 r;
			if (ConvertToData(dataOp_, &r, currentText))
			{
				index = AddItem(currentText, r);
				SetSelectedIndex(index);
			}
		}
	}

	void BililiveEditableCombobox::SetCurrentData(int64 data)
	{
		int index = FindItemData(data);
		if (index >= 0)
		{
			SetSelectedIndex(index);
		}
		else
		{
			std::wstring r;
			if (ConvertFromData(dataOp_, &r, data))
			{
				int64 copiedData = dataOp_->Alloc();
				dataOp_->Assign(copiedData, data);
				index = AddItem(r, copiedData);
				SetSelectedIndex(index);
			}
		}
	}
};
