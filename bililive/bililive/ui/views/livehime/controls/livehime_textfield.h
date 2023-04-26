#ifndef LIVEHIME_TEXTFIELD_H
#define LIVEHIME_TEXTFIELD_H

#include "bililive/bililive/ui/views/controls/bililive_textfield.h"

class LivehimeTextfield : public BililiveTextfield
{
public:
    explicit LivehimeTextfield(views::Textfield::StyleFlags style = views::Textfield::STYLE_DEFAULT);
    explicit LivehimeTextfield(views::View *view, CooperateDirection dir = BD_RIGHT, views::Textfield::StyleFlags style = views::Textfield::STYLE_DEFAULT);
    virtual ~LivehimeTextfield();

protected:
    // View
    virtual gfx::Size GetPreferredSize() OVERRIDE;
    virtual int GetHeightForWidth(int w) OVERRIDE;
    virtual void ViewHierarchyChanged(const ViewHierarchyChangedDetails& details) OVERRIDE;

    void SetCommonStyle();
};

#endif
