#include "theme_service.h"

#include "base/bind.h"
#include "base/memory/ref_counted_memory.h"
#include "base/prefs/pref_service.h"
#include "base/sequenced_task_runner.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "bililive/public/bililive/bililive_notification_types.h"
#include "bililive/bililive/profiles/profile.h"
#include "bililive/bililive/themes/bililive_theme_pack.h"
#include "bililive/bililive/themes/custom_theme_supplier.h"
#include "bililive/bililive/themes/theme_properties.h"
#include "bililive/public/common/bililive_constants.h"
#include "base/notification/notification_service.h"
#include "grit/theme_resources.h"
#include "grit/ui_resources.h"
#include "ui/base/layout.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/base/win/shell.h"
#include "bililive/public/common/pref_names.h"

//using extensions::Extension;
using ui::ResourceBundle;

typedef ThemeProperties Properties;

// The default theme if we haven't installed a theme yet or if we've clicked
// the "Use Classic" button.
const char *ThemeService::kDefaultThemeID = "";

namespace
{

    // The default theme if we've gone to the theme gallery and installed the
    // "Default" theme. We have to detect this case specifically. (By the time we
    // realize we've installed the default theme, we already have an extension
    // unpacked on the filesystem.)
    const char *kDefaultThemeGalleryID = "hkacjpbfdknhflllbcmjibkdeoafencn";

    SkColor TintForUnderline(SkColor input)
    {
        return SkColorSetA(input, SkColorGetA(input) / 3);
    }

    SkColor IncreaseLightness(SkColor color, double percent)
    {
        color_utils::HSL result;
        color_utils::SkColorToHSL(color, &result);
        result.l += (1 - result.l) * percent;
        return color_utils::HSLToSkColor(result, SkColorGetA(color));
    }

    // Writes the theme pack to disk on a separate thread.
    void WritePackToDiskCallback(BililiveThemePack *pack,
                                 const base::FilePath &path)
    {
        if (!pack->WriteToDisk(path))
        {
            NOTREACHED() << "Could not write theme pack to disk";
        }
    }

}

ThemeService *ThemeService::Create(Profile *profile)
{
    ThemeService *theme_service = new ThemeService();
    theme_service->Init(profile);
    return theme_service;
}

void ThemeService::RegisterProfilePrefs(PrefRegistrySimple *registry)
{
    registry->RegisterFilePathPref(
        prefs::kCurrentThemePackFilename,
        base::FilePath());
    registry->RegisterStringPref(
        prefs::kCurrentThemeID,
        ThemeService::kDefaultThemeID);
    //registry->RegisterDictionaryPref(
    //  prefs::kCurrentThemeImages);
    //registry->RegisterDictionaryPref(
    //  prefs::kCurrentThemeColors);
    //registry->RegisterDictionaryPref(
    //  prefs::kCurrentThemeTints);
    //registry->RegisterDictionaryPref(
    //  prefs::kCurrentThemeDisplayProperties);
}

ThemeService::ThemeService()
    : ready_(false),
      rb_(ResourceBundle::GetSharedInstance()),
      profile_(NULL),
      number_of_infobars_(0),
      weak_ptr_factory_(this)
{
}

ThemeService::~ThemeService()
{
    FreePlatformCaches();
}

void ThemeService::Init(Profile *profile)
{
    DCHECK(CalledOnValidThread());
    profile_ = profile;

    LoadThemePrefs();

    if (!ready_)
    {
        registrar_.Add(this,
                       bililive::NOTIFICATION_EXTENSIONS_READY,
                       base::Source<Profile>(profile_));
    }
}

gfx::Image ThemeService::GetImageNamed(int id) const
{
    DCHECK(CalledOnValidThread());

    gfx::Image image;
    if (theme_supplier_.get())
    {
        image = theme_supplier_->GetImageNamed(id);
    }

    if (image.IsEmpty())
    {
        image = rb_.GetNativeImageNamed(id);
    }

    return image;
}

gfx::ImageSkia *ThemeService::GetImageSkiaNamed(int id) const
{
    gfx::Image image = GetImageNamed(id);
    if (image.IsEmpty())
    {
        return NULL;
    }
    // TODO(pkotwicz): Remove this const cast.  The gfx::Image interface returns
    // its images const. GetImageSkiaNamed() also should but has many callsites.
    return const_cast<gfx::ImageSkia *>(image.ToImageSkia());
}

SkColor ThemeService::GetColor(int id) const
{
    DCHECK(CalledOnValidThread());
    SkColor color;
    if (theme_supplier_.get() && theme_supplier_->GetColor(id, &color))
    {
        return color;
    }

    // For backward compat with older themes, some newer colors are generated from
    // older ones if they are missing.
    switch (id)
    {
    case Properties::COLOR_NTP_SECTION_HEADER_TEXT:
        return IncreaseLightness(GetColor(Properties::COLOR_NTP_TEXT), 0.30);
    case Properties::COLOR_NTP_SECTION_HEADER_TEXT_HOVER:
        return GetColor(Properties::COLOR_NTP_TEXT);
    case Properties::COLOR_NTP_SECTION_HEADER_RULE:
        return IncreaseLightness(GetColor(Properties::COLOR_NTP_TEXT), 0.70);
    case Properties::COLOR_NTP_SECTION_HEADER_RULE_LIGHT:
        return IncreaseLightness(GetColor(Properties::COLOR_NTP_TEXT), 0.86);
    case Properties::COLOR_NTP_TEXT_LIGHT:
        return IncreaseLightness(GetColor(Properties::COLOR_NTP_TEXT), 0.40);
    case Properties::COLOR_MANAGED_USER_LABEL:
        return color_utils::GetReadableColor(
                   SK_ColorWHITE,
                   GetColor(Properties::COLOR_MANAGED_USER_LABEL_BACKGROUND));
    case Properties::COLOR_MANAGED_USER_LABEL_BACKGROUND:
        return color_utils::BlendTowardOppositeLuminance(
                   GetColor(Properties::COLOR_FRAME), 0x80);
    case Properties::COLOR_MANAGED_USER_LABEL_BORDER:
        return color_utils::AlphaBlend(
                   GetColor(Properties::COLOR_MANAGED_USER_LABEL_BACKGROUND),
                   SK_ColorBLACK,
                   230);
    }

    return Properties::GetDefaultColor(id);
}

bool ThemeService::GetDisplayProperty(int id, int *result) const
{
    if (theme_supplier_.get())
    {
        return theme_supplier_->GetDisplayProperty(id, result);
    }

    return Properties::GetDefaultDisplayProperty(id, result);
}

bool ThemeService::ShouldUseNativeFrame() const
{
    //if (HasCustomImage(IDR_THEME_FRAME))
    //{
    //    return false;
    //}
#if defined(OS_WIN)
    return ui::win::IsAeroGlassEnabled();
#else
    return false;
#endif
}

bool ThemeService::HasCustomImage(int id) const
{
    if (!Properties::IsThemeableImage(id))
    {
        return false;
    }

    if (theme_supplier_.get())
    {
        return theme_supplier_->HasCustomImage(id);
    }

    return false;
}

base::RefCountedMemory *ThemeService::GetRawData(
    int id,
    ui::ScaleFactor scale_factor) const
{
    base::RefCountedMemory *data = NULL;
    if (theme_supplier_.get())
    {
        data = theme_supplier_->GetRawData(id, scale_factor);
    }
    if (!data)
    {
        data = rb_.LoadDataResourceBytesForScale(id, ui::SCALE_FACTOR_100P);
    }

    return data;
}

void ThemeService::Observe(int type,
                           const base::NotificationSource &source,
                           const base::NotificationDetails &details)
{
    DCHECK(type == bililive::NOTIFICATION_EXTENSIONS_READY);
    registrar_.Remove(this, bililive::NOTIFICATION_EXTENSIONS_READY,
                      base::Source<Profile>(profile_));

    //MigrateTheme();
    set_ready();

    // Send notification in case anyone requested data and cached it when the
    // theme service was not ready yet.
    NotifyThemeChanged();
}

//void ThemeService::SetTheme(const Extension* extension) {
//  // Clear our image cache.
//  FreePlatformCaches();
//
//  DCHECK(extension);
//  DCHECK(extension->is_theme());
//
//  BuildFromExtension(extension);
//  SaveThemeID(extension->id());
//
//  NotifyThemeChanged();
//}

void ThemeService::SetCustomDefaultTheme(
    scoped_refptr<CustomThemeSupplier> theme_supplier)
{
    ClearAllThemeData();
    SwapThemeSupplier(theme_supplier);
    NotifyThemeChanged();
}

bool ThemeService::ShouldInitWithNativeTheme() const
{
    return false;
}

void ThemeService::RemoveUnusedThemes()
{
    // We do not want to garbage collect themes on startup (|ready_| is false).
    // Themes will get garbage collected once
    // ExtensionService::GarbageCollectExtensions() runs.
    if (!profile_ || !ready_)
    {
        return;
    }
}

void ThemeService::UseDefaultTheme()
{
    ClearAllThemeData();
    NotifyThemeChanged();
}

void ThemeService::SetNativeTheme()
{
    UseDefaultTheme();
}

bool ThemeService::UsingDefaultTheme() const
{
    std::string id = GetThemeID();
    return id == ThemeService::kDefaultThemeID ||
           (id == kDefaultThemeGalleryID);
}

bool ThemeService::UsingNativeTheme() const
{
    return UsingDefaultTheme();
}

std::string ThemeService::GetThemeID() const
{
    return profile_->GetPrefs()->GetString(prefs::kCurrentThemeID);
}

color_utils::HSL ThemeService::GetTint(int id) const
{
    DCHECK(CalledOnValidThread());

    color_utils::HSL hsl;
    if (theme_supplier_.get() && theme_supplier_->GetTint(id, &hsl))
    {
        return hsl;
    }

    return ThemeProperties::GetDefaultTint(id);
}

void ThemeService::ClearAllThemeData()
{
    if (!ready_)
    {
        return;
    }

    SwapThemeSupplier(NULL);

    // Clear our image cache.
    FreePlatformCaches();

    profile_->GetPrefs()->ClearPref(prefs::kCurrentThemePackFilename);
    SaveThemeID(kDefaultThemeID);

    RemoveUnusedThemes();
}

void ThemeService::LoadThemePrefs()
{
    PrefService *prefs = profile_->GetPrefs();

    std::string current_id = GetThemeID();
    if (current_id == kDefaultThemeID)
    {
        // Managed users have a different default theme.
        if (ShouldInitWithNativeTheme())
        {
            SetNativeTheme();
        }
        else
        {
            UseDefaultTheme();
        }
        set_ready();
        return;
    }

    bool loaded_pack = false;

    // If we don't have a file pack, we're updating from an old version.
    base::FilePath path = prefs->GetFilePath(prefs::kCurrentThemePackFilename);
    if (path != base::FilePath())
    {
        SwapThemeSupplier(BililiveThemePack::BuildFromDataPack(path, current_id));
        loaded_pack = theme_supplier_.get() != NULL;
    }

    if (loaded_pack)
    {
        set_ready();
    }
    else
    {

    }
}

void ThemeService::FreePlatformCaches()
{
    // Views (Skia) has no platform image cache to clear.
}

void ThemeService::NotifyThemeChanged()
{
    if (!ready_)
    {
        return;
    }

    DVLOG(1) << "Sending BILILIVE_THEME_CHANGED";
    // Redraw!
    base::NotificationService *service =
        base::NotificationService::current();
    service->Notify(bililive::NOTIFICATION_BILILIVE_THEME_CHANGED,
                    base::Source<ThemeService>(this),
                    base::NotificationService::NoDetails());
#if defined(OS_MACOSX)
    NotifyPlatformThemeChanged();
#endif  // OS_MACOSX
}


void ThemeService::SwapThemeSupplier(
    scoped_refptr<CustomThemeSupplier> theme_supplier)
{
    if (theme_supplier_.get())
    {
        theme_supplier_->StopUsingTheme();
    }
    theme_supplier_ = theme_supplier;
    if (theme_supplier_.get())
    {
        theme_supplier_->StartUsingTheme();
    }
}

void ThemeService::SavePackName(const base::FilePath &pack_path)
{
    profile_->GetPrefs()->SetFilePath(
        prefs::kCurrentThemePackFilename, pack_path);
}

void ThemeService::SaveThemeID(const std::string &id)
{
    profile_->GetPrefs()->SetString(prefs::kCurrentThemeID, id);
}

//void ThemeService::BuildFromExtension(const Extension* extension) {
//  scoped_refptr<BililiveThemePack> pack(
//      BililiveThemePack::BuildFromExtension(extension));
//  if (!pack.get()) {
//    // TODO(erg): We've failed to install the theme; perhaps we should tell the
//    // user? http://crbug.com/34780
//    LOG(ERROR) << "Could not load theme.";
//    return;
//  }
//
//  ExtensionService* service =
//      extensions::ExtensionSystem::Get(profile_)->extension_service();
//  if (!service)
//    return;
//
//  // Write the packed file to disk.
//  base::FilePath pack_path =
//      extension->path().Append(chrome::kThemePackFilename);
//  service->GetFileTaskRunner()->PostTask(
//      FROM_HERE,
//      base::Bind(&WritePackToDiskCallback, pack, pack_path));
//
//  SavePackName(pack_path);
//  SwapThemeSupplier(pack);
//}