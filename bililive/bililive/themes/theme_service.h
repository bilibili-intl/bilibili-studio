#ifndef BILILIVE_BILILIVE_THEMES_THEME_SERVICE_H
#define BILILIVE_BILILIVE_THEMES_THEME_SERVICE_H


#include <map>
#include <set>
#include <string>
#include <utility>

#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/threading/non_thread_safe.h"
#include "base/notification/notification_observer.h"
#include "base/notification/notification_registrar.h"
#include "ui/base/theme_provider.h"
#include "base/prefs/pref_registry_simple.h"


class CustomThemeSupplier;
class BililiveThemePack;
class ThemeSyncableService;
class Profile;

namespace base
{
    class FilePath;
}

namespace color_utils
{
    struct HSL;
}

namespace extensions
{
    class Extension;
}

namespace gfx
{
    class Image;
}

namespace theme_service_internal
{
    class ThemeServiceTest;
}

namespace ui
{
    class ResourceBundle;
}

class ThemeService
    : public base::NonThreadSafe
    , public base::NotificationObserver
    , public ui::ThemeProvider
{
public:
    static const char *kDefaultThemeID;

    static ThemeService *Create(Profile *profile);

    static void RegisterProfilePrefs(
        PrefRegistrySimple *registry);

    ThemeService();
    virtual ~ThemeService();

    virtual void Init(Profile *profile);

    virtual gfx::Image GetImageNamed(int id) const;

    virtual gfx::ImageSkia *GetImageSkiaNamed(int id) const OVERRIDE;
    virtual SkColor GetColor(int id) const OVERRIDE;
    virtual bool GetDisplayProperty(int id, int *result) const OVERRIDE;
    virtual bool ShouldUseNativeFrame() const OVERRIDE;
    virtual bool HasCustomImage(int id) const OVERRIDE;
    virtual base::RefCountedMemory *GetRawData(
        int id,
        ui::ScaleFactor scale_factor) const OVERRIDE;

    virtual void Observe(int type,
                         const base::NotificationSource &source,
                         const base::NotificationDetails &details) OVERRIDE;


    virtual void UseDefaultTheme();

    virtual void SetNativeTheme();

    virtual bool UsingDefaultTheme() const;

    virtual bool UsingNativeTheme() const;

    virtual std::string GetThemeID() const;

    void OnInfobarDisplayed();

    void OnInfobarDestroyed();

    void RemoveUnusedThemes();

    typedef std::map<base::FilePath, int> ImagesDiskCache;

protected:
    virtual void SetCustomDefaultTheme(
        scoped_refptr<CustomThemeSupplier> theme_supplier);

    virtual bool ShouldInitWithNativeTheme() const;

    color_utils::HSL GetTint(int id) const;

    virtual void ClearAllThemeData();

    virtual void LoadThemePrefs();

    virtual void NotifyThemeChanged();

#if defined(OS_MACOSX)
    virtual void NotifyPlatformThemeChanged();
#endif

    virtual void FreePlatformCaches();

    Profile *profile() const
    {
        return profile_;
    }

    void set_ready()
    {
        ready_ = true;
    }

    const CustomThemeSupplier *get_theme_supplier() const
    {
        return theme_supplier_.get();
    }

    bool ready_;

private:
    friend class theme_service_internal::ThemeServiceTest;

    void SwapThemeSupplier(scoped_refptr<CustomThemeSupplier> theme_supplier);

    void MigrateTheme();

    void SavePackName(const base::FilePath &pack_path);

    void SaveThemeID(const std::string &id);


    void OnManagedUserInitialized();

    ui::ResourceBundle &rb_;
    Profile *profile_;

    scoped_refptr<CustomThemeSupplier> theme_supplier_;

    int number_of_infobars_;

    base::NotificationRegistrar registrar_;


    base::WeakPtrFactory<ThemeService> weak_ptr_factory_;

    DISALLOW_COPY_AND_ASSIGN(ThemeService);
};

#endif