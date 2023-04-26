#ifndef LIVEHIME_COMBOBOX_H
#define LIVEHIME_COMBOBOX_H

#include "bililive/bililive/ui/views/controls/combox/bililive_combobox_ex.h"

class LivehimeCombobox : public BililiveComboboxEx
{
public:
    explicit LivehimeCombobox(bool editable = false);
    virtual ~LivehimeCombobox();

protected:
    // View
    gfx::Size GetPreferredSize() override;
    int GetHeightForWidth(int w) override;
};

#endif
