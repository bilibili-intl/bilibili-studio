#ifndef BILILIVE_BILILIVE_THEMES_BILILIVE_THEME_PACK_H
#define BILILIVE_BILILIVE_THEMES_BILILIVE_THEME_PACK_H


#include <map>
#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/memory/scoped_ptr.h"
#include "base/sequenced_task_runner_helpers.h"
#include "bililive/bililive/themes/custom_theme_supplier.h"
#include "extension.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/base/layout.h"
#include "ui/gfx/color_utils.h"

namespace base
{
    class DictionaryValue;
    class FilePath;
    class RefCountedMemory;
}

namespace extensions
{
    class Extensions;
}

namespace gfx
{
    class Image;
}

namespace ui
{
    class DataPack;
}

class BililiveThemePack : public CustomThemeSupplier
{
public:

    static scoped_refptr<BililiveThemePack> BuildFromDataPack(
        const base::FilePath &path, const std::string &expected_id);

    static void GetThemeableImageIDRs(std::set<int> *result);

    bool WriteToDisk(const base::FilePath &path) const;

    virtual bool GetTint(int id, color_utils::HSL *hsl) const OVERRIDE;
    virtual bool GetColor(int id, SkColor *color) const OVERRIDE;
    virtual bool GetDisplayProperty(int id, int *result) const OVERRIDE;
    virtual gfx::Image GetImageNamed(int id) OVERRIDE;
    virtual base::RefCountedMemory *GetRawData(
        int id, ui::ScaleFactor scale_factor) const OVERRIDE;
    virtual bool HasCustomImage(int id) const OVERRIDE;

private:
    friend class BililiveThemePackTest;

    typedef std::map<int, gfx::Image> ImageCache;

    typedef std::map<int, scoped_refptr<base::RefCountedMemory> > RawImages;

    typedef std::map<uint16, base::StringPiece> RawDataForWriting;

    typedef std::map<ui::ScaleFactor, base::FilePath> ScaleFactorToFileMap;

    typedef std::map<int, ScaleFactorToFileMap> FilePathMap;

    BililiveThemePack();

    virtual ~BililiveThemePack();

    void BuildHeader(const Extension *extension);

    void BuildTintsFromJSON(const base::DictionaryValue *tints_value);

    void BuildColorsFromJSON(const base::DictionaryValue *color_value);

    void ReadColorsFromJSON(const base::DictionaryValue *colors_value,
                            std::map<int, SkColor> *temp_colors);
    void GenerateMissingColors(std::map<int, SkColor> *temp_colors);

    void BuildDisplayPropertiesFromJSON(
        const base::DictionaryValue *display_value);

    void ParseImageNamesFromJSON(const base::DictionaryValue *images_value,
                                 const base::FilePath &images_path,
                                 FilePathMap *file_paths) const;

    void AddFileAtScaleToMap(const std::string &image_name,
                             ui::ScaleFactor scale_factor,
                             const base::FilePath &image_path,
                             FilePathMap *file_paths) const;

    void BuildSourceImagesArray(const FilePathMap &file_paths);

    bool LoadRawBitmapsTo(const FilePathMap &file_paths,
                          ImageCache *image_cache);

    void CreateImages(ImageCache *images) const;

    void CropImages(ImageCache *images) const;

    void CreateFrameImages(ImageCache *images) const;

    void CreateTintedButtons(const color_utils::HSL &button_tint,
                             ImageCache *processed_images) const;

    void RepackImages(const ImageCache &images,
                      RawImages *reencoded_images) const;

    void MergeImageCaches(const ImageCache &source,
                          ImageCache *destination) const;

    void CopyImagesTo(const ImageCache &source, ImageCache *destination) const;

    void AddRawImagesTo(const RawImages &images, RawDataForWriting *out) const;

    color_utils::HSL GetTintInternal(int id) const;

    int GetRawIDByPersistentID(int prs_id, ui::ScaleFactor scale_factor) const;

    bool GetScaleFactorFromManifestKey(const std::string &key,
                                       ui::ScaleFactor *scale_factor) const;

    void GenerateRawImageForAllSupportedScales(int prs_id);

    scoped_ptr<ui::DataPack> data_pack_;

#pragma pack(push,1)
    struct BililiveThemePackHeader
    {
        int32 version;

        int32 little_endian;

        uint8 theme_id[16];
    } *header_;

    struct TintEntry
    {
        int32 id;
        double h;
        double s;
        double l;
    } *tints_;

    struct ColorPair
    {
        int32 id;
        SkColor color;
    } *colors_;

    struct DisplayPropertyPair
    {
        int32 id;
        int32 property;
    } *display_properties_;

    int *source_images_;
#pragma pack(pop)

    std::vector<ui::ScaleFactor> scale_factors_;

    RawImages image_memory_;

    ImageCache images_on_ui_thread_;

    ImageCache images_on_file_thread_;

    DISALLOW_COPY_AND_ASSIGN(BililiveThemePack);
};

#endif