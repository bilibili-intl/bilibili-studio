#include "bililive/bililive/livehime/notify_ui_control/notify_ui_prop.h"

#include <map>

#include "bililive/bililive/ui/bililive_obs.h"
#include "bililive/bililive/ui/views/livehime/main_view/livehime_main_view.h"
#include "bililive/public/bililive/bililive_process.h"


namespace
{
    static std::map<livehime::UIPropType, const char*> kPropMap{
        { livehime::UIPropType::Unknown, "" },
        { livehime::UIPropType::AppMngBtn, "livehime.main.appmng.entrance" },
        { livehime::UIPropType::CpmEntranceBtn, "livehime.main.cpm.entrance" },
    };
}


namespace livehime
{
    std::unique_ptr<ui::ViewProp> SetUIProp(UIPropType ui_type, void* data)
    {
        std::unique_ptr<ui::ViewProp> prop;
        switch (ui_type)
        {
        case UIPropType::AppMngBtn:
        case UIPropType::CpmEntranceBtn:
            {
                if (GetBililiveProcess()->bililive_obs()->obs_view() && 
                    GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget())
                {
                    gfx::NativeView main_wnd = GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget()->GetNativeView();
                    DCHECK(!!main_wnd);
                    prop.reset(new ui::ViewProp(main_wnd, kPropMap[ui_type], data));
                }
            }
            break;
        default:
            break;
        }
        DCHECK(!!prop.get());
        return prop;
    }

    void* GetUIProp(UIPropType ui_type)
    {
        void* data = nullptr;
        switch (ui_type)
        {
        case UIPropType::AppMngBtn:
        case UIPropType::CpmEntranceBtn:
            {
                if (GetBililiveProcess()->bililive_obs()->obs_view() &&
                    GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget())
                {
                    gfx::NativeView main_wnd = GetBililiveProcess()->bililive_obs()->obs_view()->GetWidget()->GetNativeView();
                    DCHECK(!!main_wnd);
                    data = ui::ViewProp::GetValue(main_wnd, kPropMap[ui_type]);
                }
            }
            break;
        default:
            break;
        }
        return data;
    }

}
