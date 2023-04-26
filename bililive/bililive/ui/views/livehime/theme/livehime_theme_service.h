#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_THEMES_SERVICE_H
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_THEMES_SERVICE_H

#include "livehime_theme_constants.h"

#include "base/prefs/pref_registry_simple.h"
#include "base/threading/non_thread_safe.h"

#include "ui/base/theme_provider.h"
#include "ui/base/resource/resource_bundle.h"

class Profile;

class LivehimeThemeService
    : public base::NonThreadSafe
    , public ui::ThemeProvider
{
public:
    static void RegisterProfilePrefs(PrefRegistrySimple *registry);

    static LivehimeThemeService *Create();

    LivehimeThemeService();
    virtual ~LivehimeThemeService();

    void Init(Profile *profile);
    LivehimeThemeType GetThemeType() const;
    void ChangeTheme(LivehimeThemeType type);

    gfx::ImageSkia *GetImageSkiaNamed(int id) const override;
    SkColor GetColor(int id) const override;
    bool GetDisplayProperty(int id, int *result) const override;
    bool ShouldUseNativeFrame() const override;
    bool HasCustomImage(int id) const override;
    base::RefCountedMemory *GetRawData(int id, ui::ScaleFactor scale_factor) const override;

private:
    void LoadTheme();
    void NotifyThemeChanged();

private:
    LivehimeThemeType theme_type_;
    const std::map<LivehimeColorType, SkColor>* default_theme_colors_ = nullptr;
    const std::map<LivehimeColorType, SkColor>* theme_colors_ = nullptr;
    ui::ResourceBundle &rb_;

    //base::WeakPtrFactory<LivehimeThemeService> weak_ptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(LivehimeThemeService);
};

#endif