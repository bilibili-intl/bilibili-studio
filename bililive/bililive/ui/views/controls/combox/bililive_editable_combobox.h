#ifndef BILILIVE_EDITABLE_COMBOBOX_H
#define BILILIVE_EDITABLE_COMBOBOX_H

#include <vector>
#include <utility>
#include <memory>
#include <string>

#include "bililive/bililive/ui/views/controls/combox/bililive_combobox.h"

namespace bililive
{
	class BililiveEditableCombobox
		: public BililiveCombobox
	{
	public:
		BililiveEditableCombobox(BililiveComboboxDataOperator* op);
		~BililiveEditableCombobox();

		void OnTextfieldBlur() override;
		void OnTextfieldKeyPressed(const ui::KeyEvent& ke) override;

		void SetCurrentText(const std::wstring& text);
		void SetCurrentData(int64 data);

	protected:
		virtual bool ConvertToData(BililiveComboboxDataOperator* op, int64* outData, const std::wstring& inText) = 0;
		virtual bool ConvertFromData(BililiveComboboxDataOperator* op, std::wstring* outText, int64 inData) = 0;
	};
};


#endif
