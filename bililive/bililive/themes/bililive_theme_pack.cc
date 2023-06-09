#include "bililive_theme_pack.h"

#include <limits>

#include "base/memory/ref_counted_memory.h"
#include "base/memory/scoped_ptr.h"
#include "base/stl_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/sequenced_worker_pool.h"
#include "base/threading/thread_restrictions.h"
#include "base/values.h"
#include "bililive/bililive/themes/theme_properties.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "grit/theme_resources.h"
#include "grit/ui_resources.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "ui/base/resource/data_pack.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/codec/png_codec.h"
#include "ui/gfx/image/canvas_image_source.h"
#include "ui/gfx/image/image.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/gfx/image/image_skia_operations.h"
#include "ui/gfx/screen.h"
#include "ui/gfx/size_conversions.h"
#include "ui/gfx/skia_util.h"
#include "base/files/file_stream_win.h"
#include "base/files/net_error_list.h"

namespace
{

    // Version number of the current theme pack. We just throw out and rebuild
    // theme packs that aren't int-equal to this. Increment this number if you
    // change default theme assets.
    const int kThemePackVersion = 31;

    // IDs that are in the DataPack won't clash with the positive integer
    // uint16. kHeaderID should always have the maximum value because we want the
    // "header" to be written last. That way we can detect whether the pack was
    // successfully written and ignore and regenerate if it was only partially
    // written (i.e. bililive crashed on a different thread while writing the pack).
    const int kMaxID = 0x0000FFFF;  // Max unsigned 16-bit int.
    const int kHeaderID = kMaxID - 1;
    const int kTintsID = kMaxID - 2;
    const int kColorsID = kMaxID - 3;
    const int kDisplayPropertiesID = kMaxID - 4;
    const int kSourceImagesID = kMaxID - 5;
    const int kScaleFactorsID = kMaxID - 6;

    const int kIdSize = 16;

    // The sum of kFrameBorderThickness and kNonClientRestoredExtraThickness from
    // OpaqueBililiveFrameView.
    const int kRestoredTabVerticalOffset = 15;

    // Persistent constants for the main images that we need. These have the same
    // names as their IDR_* counterparts but these values will always stay the
    // same.
    const int PRS_THEME_FRAME = 1;
    const int PRS_THEME_TOOLBAR = 2;

    struct PersistingImagesTable
    {
        // A non-changing integer ID meant to be saved in theme packs. This ID must
        // not change between versions of bililive.
        int persistent_id;

        // The IDR that depends on the whims of GRIT and therefore changes whenever
        // someone adds a new resource.
        int idr_id;

        // String to check for when parsing theme manifests or NULL if this isn't
        // supposed to be changeable by the user.
        const char *key;
    };

    // IDR_* resource names change whenever new resources are added; use persistent
    // IDs when storing to a cached pack.
    PersistingImagesTable kPersistingImages[] =
    {
        //{
        //    PRS_THEME_FRAME, IDR_THEME_FRAME,
        //    "theme_frame"
        //},
        //{
        //    PRS_THEME_TOOLBAR, IDR_THEME_TOOLBAR,
        //    "theme_toolbar"
        //},

        // The rest of these entries have no key because they can't be overridden
        // from the json manifest.
        { 39, IDR_MENU_DROPARROW, NULL },
        { 40, IDR_THROBBER, NULL },
    };
    const size_t kPersistingImagesLength = arraysize(kPersistingImages);

#if defined(OS_WIN) && defined(USE_AURA)
    // Persistent theme ids for Windows AURA.
    const int PRS_THEME_FRAME_WIN = 100;
    const int PRS_THEME_FRAME_INACTIVE_WIN = 101;
    const int PRS_THEME_FRAME_INCOGNITO_WIN = 102;
    const int PRS_THEME_FRAME_INCOGNITO_INACTIVE_WIN = 103;
    const int PRS_THEME_TOOLBAR_WIN = 104;
    const int PRS_THEME_TAB_BACKGROUND_WIN = 105;
    const int PRS_THEME_TAB_BACKGROUND_INCOGNITO_WIN = 106;

    // Persistent theme to resource id mapping for Windows AURA.
    PersistingImagesTable kPersistingImagesWinDesktopAura[] =
    {
        {
            PRS_THEME_FRAME_WIN, IDR_THEME_FRAME_WIN,
            "theme_frame"
        },
        {
            PRS_THEME_FRAME_INACTIVE_WIN, IDR_THEME_FRAME_INACTIVE_WIN,
            "theme_frame_inactive"
        },
        {
            PRS_THEME_FRAME_INCOGNITO_WIN, IDR_THEME_FRAME_INCOGNITO_WIN,
            "theme_frame_incognito"
        },
        {
            PRS_THEME_FRAME_INCOGNITO_INACTIVE_WIN,
            IDR_THEME_FRAME_INCOGNITO_INACTIVE_WIN,
            "theme_frame_incognito_inactive"
        },
        {
            PRS_THEME_TOOLBAR_WIN, IDR_THEME_TOOLBAR_WIN,
            "theme_toolbar"
        },
        {
            PRS_THEME_TAB_BACKGROUND_WIN, IDR_THEME_TAB_BACKGROUND_WIN,
            "theme_tab_background"
        },
        {
            PRS_THEME_TAB_BACKGROUND_INCOGNITO_WIN,
            IDR_THEME_TAB_BACKGROUND_INCOGNITO_WIN,
            "theme_tab_background_incognito"
        },
    };
    const size_t kPersistingImagesWinDesktopAuraLength =
        arraysize(kPersistingImagesWinDesktopAura);
#endif

    int GetPersistentIDByNameHelper(const std::string &key,
                                    const PersistingImagesTable *image_table,
                                    size_t image_table_size)
    {
        for (size_t i = 0; i < image_table_size; ++i)
        {
            if (image_table[i].key != NULL &&
                base::strcasecmp(key.c_str(), image_table[i].key) == 0)
            {
                return image_table[i].persistent_id;
            }
        }
        return -1;
    }

    int GetPersistentIDByName(const std::string &key)
    {
        return GetPersistentIDByNameHelper(key,
                                           kPersistingImages,
                                           kPersistingImagesLength);
    }

    int GetPersistentIDByIDR(int idr)
    {
        static std::map<int, int> *lookup_table = new std::map<int, int>();
        if (lookup_table->empty())
        {
            for (size_t i = 0; i < kPersistingImagesLength; ++i)
            {
                int idr = kPersistingImages[i].idr_id;
                int prs_id = kPersistingImages[i].persistent_id;
                (*lookup_table)[idr] = prs_id;
            }
#if defined(OS_WIN) && defined(USE_AURA)
            for (size_t i = 0; i < kPersistingImagesWinDesktopAuraLength; ++i)
            {
                int idr = kPersistingImagesWinDesktopAura[i].idr_id;
                int prs_id = kPersistingImagesWinDesktopAura[i].persistent_id;
                (*lookup_table)[idr] = prs_id;
            }
#endif
        }
        std::map<int, int>::iterator it = lookup_table->find(idr);
        return (it == lookup_table->end()) ? -1 : it->second;
    }

    // Returns true if the scales in |input| match those in |expected|.
    // The order must match as the index is used in determining the raw id.
    bool InputScalesValid(const base::StringPiece &input,
                          const std::vector<ui::ScaleFactor> &expected)
    {
        size_t scales_size = static_cast<size_t>(input.size() / sizeof(float));
        if (scales_size != expected.size())
        {
            return false;
        }
        scoped_ptr<float[]> scales(new float[scales_size]);
        // Do a memcpy to avoid misaligned memory access.
        memcpy(scales.get(), input.data(), input.size());
        for (size_t index = 0; index < scales_size; ++index)
        {
            if (scales[index] != ui::GetScaleFactorScale(expected[index]))
            {
                return false;
            }
        }
        return true;
    }

    // Returns |scale_factors| as a string to be written to disk.
    std::string GetScaleFactorsAsString(
        const std::vector<ui::ScaleFactor> &scale_factors)
    {
        scoped_ptr<float[]> scales(new float[scale_factors.size()]);
        for (size_t i = 0; i < scale_factors.size(); ++i)
        {
            scales[i] = ui::GetScaleFactorScale(scale_factors[i]);
        }
        std::string out_string = std::string(
                                     reinterpret_cast<const char *>(scales.get()),
                                     scale_factors.size() * sizeof(float));
        return out_string;
    }

    struct StringToIntTable
    {
        const char *key;
        ThemeProperties::OverwritableByUserThemeProperty id;
    };

    // Strings used by themes to identify tints in the JSON.
    StringToIntTable kTintTable[] =
    {
        { "buttons", ThemeProperties::TINT_BUTTONS },
        { "frame", ThemeProperties::TINT_FRAME },
        { "frame_inactive", ThemeProperties::TINT_FRAME_INACTIVE },
        { "frame_incognito", ThemeProperties::TINT_FRAME_INCOGNITO },
        {
            "frame_incognito_inactive",
            ThemeProperties::TINT_FRAME_INCOGNITO_INACTIVE
        },
        { "background_tab", ThemeProperties::TINT_BACKGROUND_TAB },
    };
    const size_t kTintTableLength = arraysize(kTintTable);

    // Strings used by themes to identify colors in the JSON.
    StringToIntTable kColorTable[] =
    {
        { "frame", ThemeProperties::COLOR_FRAME },
        { "frame_inactive", ThemeProperties::COLOR_FRAME_INACTIVE },
        { "frame_incognito", ThemeProperties::COLOR_FRAME_INCOGNITO },
        {
            "frame_incognito_inactive",
            ThemeProperties::COLOR_FRAME_INCOGNITO_INACTIVE
        },
        { "toolbar", ThemeProperties::COLOR_TOOLBAR },
        { "tab_text", ThemeProperties::COLOR_TAB_TEXT },
        { "tab_background_text", ThemeProperties::COLOR_BACKGROUND_TAB_TEXT },
        { "bookmark_text", ThemeProperties::COLOR_BOOKMARK_TEXT },
        { "ntp_background", ThemeProperties::COLOR_NTP_BACKGROUND },
        { "ntp_text", ThemeProperties::COLOR_NTP_TEXT },
        { "ntp_link", ThemeProperties::COLOR_NTP_LINK },
        { "ntp_link_underline", ThemeProperties::COLOR_NTP_LINK_UNDERLINE },
        { "ntp_header", ThemeProperties::COLOR_NTP_HEADER },
        { "ntp_section", ThemeProperties::COLOR_NTP_SECTION },
        { "ntp_section_text", ThemeProperties::COLOR_NTP_SECTION_TEXT },
        { "ntp_section_link", ThemeProperties::COLOR_NTP_SECTION_LINK },
        {
            "ntp_section_link_underline",
            ThemeProperties::COLOR_NTP_SECTION_LINK_UNDERLINE
        },
        { "button_background", ThemeProperties::COLOR_BUTTON_BACKGROUND },
    };
    const size_t kColorTableLength = arraysize(kColorTable);

    // Strings used by themes to identify display properties keys in JSON.
    StringToIntTable kDisplayProperties[] =
    {
        {
            "ntp_background_alignment",
            ThemeProperties::NTP_BACKGROUND_ALIGNMENT
        },
        { "ntp_background_repeat", ThemeProperties::NTP_BACKGROUND_TILING },
        { "ntp_logo_alternate", ThemeProperties::NTP_LOGO_ALTERNATE },
    };
    const size_t kDisplayPropertiesSize = arraysize(kDisplayProperties);

    int GetIntForString(const std::string &key,
                        StringToIntTable *table,
                        size_t table_length)
    {
        for (size_t i = 0; i < table_length; ++i)
        {
            if (base::strcasecmp(key.c_str(), table[i].key) == 0)
            {
                return table[i].id;
            }
        }

        return -1;
    }

    struct IntToIntTable
    {
        int key;
        int value;
    };

    // Mapping used in CreateFrameImages() to associate frame images with the
    // tint ID that should maybe be applied to it.
    IntToIntTable kFrameTintMap[] =
    {
        { PRS_THEME_FRAME, ThemeProperties::TINT_FRAME },
    };

    struct CropEntry
    {
        int prs_id;

        // The maximum useful height of the image at |prs_id|.
        int max_height;

        // Whether cropping the image at |prs_id| should be skipped on OSes which
        // have a frame border to the left and right of the web contents.
        // This should be true for images which can be used to decorate the border to
        // the left and the right of the web contents.
        bool skip_if_frame_border;
    };

    // The images which should be cropped before being saved to the data pack. The
    // maximum heights are meant to be conservative as to give room for the UI to
    // change without the maximum heights having to be modified.
    // |kThemePackVersion| must be incremented if any of the maximum heights below
    // are modified.
    struct CropEntry kImagesToCrop[] =
    {
        { PRS_THEME_FRAME, 120, true }
    };


    // Returns true if this OS uses a bililive frame which has a non zero width to
    // the left and the right of the web contents.
    bool HasFrameBorder()
    {
        return true;
    }

    // Returns a piece of memory with the contents of the file |path|.
    base::RefCountedMemory *ReadFileData(const base::FilePath &path)
    {
        if (!path.empty())
        {
            base::FileStreamWin file;
            int flags = base::PLATFORM_FILE_OPEN | base::PLATFORM_FILE_READ;
            if (file.OpenSync(path, flags) == 0)
            {
                int64 avail = file.Available();
                if (avail > 0 && avail < INT_MAX)
                {
                    size_t size = static_cast<size_t>(avail);
                    std::vector<unsigned char> raw_data;
                    raw_data.resize(size);
                    char *data = reinterpret_cast<char *>(&(raw_data.front()));
                    if (file.ReadUntilComplete(data, size) == avail)
                    {
                        return base::RefCountedBytes::TakeVector(&raw_data);
                    }
                }
            }
        }

        return NULL;
    }

    // Shifts an image's HSL values. The caller is responsible for deleting
    // the returned image.
    gfx::Image CreateHSLShiftedImage(const gfx::Image &image,
                                     const color_utils::HSL &hsl_shift)
    {
        const gfx::ImageSkia *src_image = image.ToImageSkia();
        return gfx::Image(gfx::ImageSkiaOperations::CreateHSLShiftedImage(
                              *src_image, hsl_shift));
    }

    // Computes a bitmap at one scale from a bitmap at a different scale.
    SkBitmap CreateLowQualityResizedBitmap(const SkBitmap &source_bitmap,
                                           ui::ScaleFactor source_scale_factor,
                                           ui::ScaleFactor desired_scale_factor)
    {
        gfx::Size scaled_size = gfx::ToCeiledSize(
                                    gfx::ScaleSize(gfx::Size(source_bitmap.width(),
                                            source_bitmap.height()),
                                            ui::GetScaleFactorScale(desired_scale_factor) /
                                            ui::GetScaleFactorScale(source_scale_factor)));
        SkBitmap scaled_bitmap;
        scaled_bitmap.setConfig(SkBitmap::kARGB_8888_Config,
                                scaled_size.width(),
                                scaled_size.height());
        if (!scaled_bitmap.allocPixels())
        {
            SK_CRASH();
        }
        scaled_bitmap.eraseARGB(0, 0, 0, 0);
        SkCanvas canvas(scaled_bitmap);
        SkRect scaled_bounds = RectToSkRect(gfx::Rect(scaled_size));
        // Note(oshima): The following scaling code doesn't work with
        // a mask image.
        canvas.drawBitmapRect(source_bitmap, NULL, scaled_bounds);
        return scaled_bitmap;
    }

    // A ImageSkiaSource that scales 100P image to the target scale factor
    // if the ImageSkiaRep for the target scale factor isn't available.
    class ThemeImageSource: public gfx::ImageSkiaSource
    {
    public:
        explicit ThemeImageSource(const gfx::ImageSkia &source) : source_(source)
        {
        }
        virtual ~ThemeImageSource() {}

        virtual gfx::ImageSkiaRep GetImageForScale(
            ui::ScaleFactor scale_factor) OVERRIDE
        {
            if (source_.HasRepresentation(scale_factor))
            {
                return source_.GetRepresentation(scale_factor);
            }
            const gfx::ImageSkiaRep &rep_100p =
            source_.GetRepresentation(ui::SCALE_FACTOR_100P);
            SkBitmap scaled_bitmap = CreateLowQualityResizedBitmap(
                rep_100p.sk_bitmap(),
                ui::SCALE_FACTOR_100P,
                scale_factor);
            return gfx::ImageSkiaRep(scaled_bitmap, scale_factor);
        }

    private:
        const gfx::ImageSkia source_;

        DISALLOW_COPY_AND_ASSIGN(ThemeImageSource);
    };

    // An ImageSkiaSource that delays decoding PNG data into bitmaps until
    // needed. Missing data for a scale factor is computed by scaling data for an
    // available scale factor. Computed bitmaps are stored for future look up.
    class ThemeImagePngSource : public gfx::ImageSkiaSource
    {
    public:
        typedef std::map<ui::ScaleFactor,
                scoped_refptr<base::RefCountedMemory> > PngMap;

        explicit ThemeImagePngSource(const PngMap &png_map) : png_map_(png_map) {}

        virtual ~ThemeImagePngSource() {}

    private:
        virtual gfx::ImageSkiaRep GetImageForScale(
            ui::ScaleFactor scale_factor) OVERRIDE
        {
            // Look up the bitmap for |scale factor| in the bitmap map. If found
            // return it.
            BitmapMap::const_iterator exact_bitmap_it = bitmap_map_.find(scale_factor);
            if (exact_bitmap_it != bitmap_map_.end())
            {
                return gfx::ImageSkiaRep(exact_bitmap_it->second, scale_factor);
            }

            // Look up the raw PNG data for |scale_factor| in the png map. If found,
            // decode it, store the result in the bitmap map and return it.
            PngMap::const_iterator exact_png_it = png_map_.find(scale_factor);
            if (exact_png_it != png_map_.end())
            {
                SkBitmap bitmap;
                if (!gfx::PNGCodec::Decode(exact_png_it->second->front(),
                exact_png_it->second->size(),
                &bitmap))
                {
                    NOTREACHED();
                    return gfx::ImageSkiaRep();
                }
                bitmap_map_[scale_factor] = bitmap;
                return gfx::ImageSkiaRep(bitmap, scale_factor);
            }

            // Find an available PNG for another scale factor. We want to use the
            // highest available scale factor.
            PngMap::const_iterator available_png_it = png_map_.end();
            for (PngMap::const_iterator png_it = png_map_.begin();
                 png_it != png_map_.end(); ++png_it)
            {
                if (available_png_it == png_map_.end() ||
                    ui::GetScaleFactorScale(png_it->first) >
                    ui::GetScaleFactorScale(available_png_it->first))
                {
                    available_png_it = png_it;
                }
            }
            if (available_png_it == png_map_.end())
            {
                return gfx::ImageSkiaRep();
            }
            ui::ScaleFactor available_scale_factor = available_png_it->first;

            // Look up the bitmap for |available_scale_factor| in the bitmap map.
            // If not found, decode the corresponging png data, store the result
            // in the bitmap map.
            BitmapMap::const_iterator available_bitmap_it =
                bitmap_map_.find(available_scale_factor);
            if (available_bitmap_it == bitmap_map_.end())
            {
                SkBitmap available_bitmap;
                if (!gfx::PNGCodec::Decode(available_png_it->second->front(),
                                           available_png_it->second->size(),
                                           &available_bitmap))
                {
                    NOTREACHED();
                    return gfx::ImageSkiaRep();
                }
                bitmap_map_[available_scale_factor] = available_bitmap;
                available_bitmap_it = bitmap_map_.find(available_scale_factor);
            }

            // Scale the available bitmap to the desired scale factor, store the result
            // in the bitmap map and return it.
            SkBitmap scaled_bitmap = CreateLowQualityResizedBitmap(
                                         available_bitmap_it->second,
                                         available_scale_factor,
                                         scale_factor);
            bitmap_map_[scale_factor] = scaled_bitmap;
            return gfx::ImageSkiaRep(scaled_bitmap, scale_factor);
        }

        PngMap png_map_;

        typedef std::map<ui::ScaleFactor, SkBitmap> BitmapMap;
        BitmapMap bitmap_map_;

        DISALLOW_COPY_AND_ASSIGN(ThemeImagePngSource);
    };

    class TabBackgroundImageSource: public gfx::CanvasImageSource
    {
    public:
        TabBackgroundImageSource(const gfx::ImageSkia &image_to_tint,
                                 const gfx::ImageSkia &overlay,
                                 const color_utils::HSL &hsl_shift,
                                 int vertical_offset)
            : gfx::CanvasImageSource(image_to_tint.size(), false),
              image_to_tint_(image_to_tint),
              overlay_(overlay),
              hsl_shift_(hsl_shift),
              vertical_offset_(vertical_offset)
        {
        }

        virtual ~TabBackgroundImageSource()
        {
        }

        // Overridden from CanvasImageSource:
        virtual void Draw(gfx::Canvas *canvas) OVERRIDE
        {
            gfx::ImageSkia bg_tint =
            gfx::ImageSkiaOperations::CreateHSLShiftedImage(image_to_tint_,
            hsl_shift_);
            canvas->TileImageInt(bg_tint, 0, vertical_offset_, 0, 0,
            size().width(), size().height());

            // If they've provided a custom image, overlay it.
            if (!overlay_.isNull())
            {
                canvas->TileImageInt(overlay_, 0, 0, size().width(),
                overlay_.height());
            }
        }

    private:
        const gfx::ImageSkia image_to_tint_;
        const gfx::ImageSkia overlay_;
        const color_utils::HSL hsl_shift_;
        const int vertical_offset_;

        DISALLOW_COPY_AND_ASSIGN(TabBackgroundImageSource);
    };

}

BililiveThemePack::~BililiveThemePack()
{
    if (!data_pack_.get())
    {
        delete header_;
        delete [] tints_;
        delete [] colors_;
        delete [] display_properties_;
        delete [] source_images_;
    }
}

// static
//scoped_refptr<BililiveThemePack> BililiveThemePack::BuildFromExtension(
//    const Extension* extension) {
//  DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));
//  DCHECK(extension);
//  DCHECK(extension->is_theme());
//
//  scoped_refptr<BililiveThemePack> pack(new BililiveThemePack);
//  pack->BuildHeader(extension);
//  pack->BuildTintsFromJSON(extensions::ThemeInfo::GetTints(extension));
//  pack->BuildColorsFromJSON(extensions::ThemeInfo::GetColors(extension));
//  pack->BuildDisplayPropertiesFromJSON(
//      extensions::ThemeInfo::GetDisplayProperties(extension));
//
//  // Builds the images. (Image building is dependent on tints).
//  FilePathMap file_paths;
//  pack->ParseImageNamesFromJSON(
//      extensions::ThemeInfo::GetImages(extension),
//      extension->path(),
//      &file_paths);
//  pack->BuildSourceImagesArray(file_paths);
//
//  if (!pack->LoadRawBitmapsTo(file_paths, &pack->images_on_ui_thread_))
//    return NULL;
//
//  pack->CreateImages(&pack->images_on_ui_thread_);
//
//  // Make sure the |images_on_file_thread_| has bitmaps for supported
//  // scale factors before passing to FILE thread.
//  pack->images_on_file_thread_ = pack->images_on_ui_thread_;
//  for (ImageCache::iterator it = pack->images_on_file_thread_.begin();
//       it != pack->images_on_file_thread_.end(); ++it) {
//    gfx::ImageSkia* image_skia =
//        const_cast<gfx::ImageSkia*>(it->second.ToImageSkia());
//    image_skia->MakeThreadSafe();
//  }
//
//  // Set ThemeImageSource on |images_on_ui_thread_| to resample the source
//  // image if a caller of BililiveThemePack::GetImageNamed() requests an
//  // ImageSkiaRep for a scale factor not specified by the theme author.
//  // Callers of BililiveThemePack::GetImageNamed() to be able to retrieve
//  // ImageSkiaReps for all supported scale factors.
//  for (ImageCache::iterator it = pack->images_on_ui_thread_.begin();
//       it != pack->images_on_ui_thread_.end(); ++it) {
//    const gfx::ImageSkia source_image_skia = it->second.AsImageSkia();
//    ThemeImageSource* source = new ThemeImageSource(source_image_skia);
//    // image_skia takes ownership of source.
//    gfx::ImageSkia image_skia(source, source_image_skia.size());
//    it->second = gfx::Image(image_skia);
//  }
//
//  // Generate raw images (for new-tab-page attribution and background) for
//  // any missing scale from an available scale image.
//  for (size_t i = 0; i < arraysize(kPreloadIDs); ++i) {
//    pack->GenerateRawImageForAllSupportedScales(kPreloadIDs[i]);
//  }
//
//  // The BililiveThemePack is now in a consistent state.
//  return pack;
//}

// static
scoped_refptr<BililiveThemePack> BililiveThemePack::BuildFromDataPack(
    const base::FilePath &path, const std::string &expected_id)
{
    DCHECK(BililiveThread::CurrentlyOn(BililiveThread::UI));
    // Allow IO on UI thread due to deep-seated theme design issues.
    // (see http://crbug.com/80206)
    base::ThreadRestrictions::ScopedAllowIO allow_io;
    scoped_refptr<BililiveThemePack> pack(new BililiveThemePack);
    // Scale factor parameter is moot as data pack has image resources for all
    // supported scale factors.
    pack->data_pack_.reset(
        new ui::DataPack(ui::SCALE_FACTOR_NONE));

    if (!pack->data_pack_->LoadFromPath(path))
    {
        LOG(ERROR) << "Failed to load theme data pack.";
        return NULL;
    }

    base::StringPiece pointer;
    if (!pack->data_pack_->GetStringPiece(kHeaderID, &pointer))
    {
        return NULL;
    }
    pack->header_ = reinterpret_cast<BililiveThemePackHeader *>(const_cast<char *>(
                        pointer.data()));

    if (pack->header_->version != kThemePackVersion)
    {
        DLOG(ERROR) << "BuildFromDataPack failure! Version mismatch!";
        return NULL;
    }
    // TODO(erg): Check endianess once DataPack works on the other endian.
    std::string theme_id(reinterpret_cast<char *>(pack->header_->theme_id),
                         kIdSize);
    std::string truncated_id =
        expected_id.substr(0, kIdSize);
    if (theme_id != truncated_id)
    {
        DLOG(ERROR) << "Wrong id: " << theme_id << " vs " << expected_id;
        return NULL;
    }

    if (!pack->data_pack_->GetStringPiece(kTintsID, &pointer))
    {
        return NULL;
    }
    pack->tints_ = reinterpret_cast<TintEntry *>(const_cast<char *>(
                       pointer.data()));

    if (!pack->data_pack_->GetStringPiece(kColorsID, &pointer))
    {
        return NULL;
    }
    pack->colors_ =
        reinterpret_cast<ColorPair *>(const_cast<char *>(pointer.data()));

    if (!pack->data_pack_->GetStringPiece(kDisplayPropertiesID, &pointer))
    {
        return NULL;
    }
    pack->display_properties_ = reinterpret_cast<DisplayPropertyPair *>(
                                    const_cast<char *>(pointer.data()));

    if (!pack->data_pack_->GetStringPiece(kSourceImagesID, &pointer))
    {
        return NULL;
    }
    pack->source_images_ = reinterpret_cast<int *>(
                               const_cast<char *>(pointer.data()));

    if (!pack->data_pack_->GetStringPiece(kScaleFactorsID, &pointer))
    {
        return NULL;
    }

    if (!InputScalesValid(pointer, pack->scale_factors_))
    {
        DLOG(ERROR) << "BuildFromDataPack failure! The pack scale factors differ "
                    << "from those supported by platform.";
    }
    return pack;
}

// static
void BililiveThemePack::GetThemeableImageIDRs(std::set<int> *result)
{
    if (!result)
    {
        return;
    }

    result->clear();
    for (size_t i = 0; i < kPersistingImagesLength; ++i)
    {
        result->insert(kPersistingImages[i].idr_id);
    }

#if defined(OS_WIN) && defined(USE_AURA)
    for (size_t i = 0; i < kPersistingImagesWinDesktopAuraLength; ++i)
    {
        result->insert(kPersistingImagesWinDesktopAura[i].idr_id);
    }
#endif
}

bool BililiveThemePack::WriteToDisk(const base::FilePath &path) const
{
    // Add resources for each of the property arrays.
    RawDataForWriting resources;
    resources[kHeaderID] = base::StringPiece(
                               reinterpret_cast<const char *>(header_), sizeof(BililiveThemePackHeader));
    resources[kTintsID] = base::StringPiece(
                              reinterpret_cast<const char *>(tints_),
                              sizeof(TintEntry[kTintTableLength]));
    resources[kColorsID] = base::StringPiece(
                               reinterpret_cast<const char *>(colors_),
                               sizeof(ColorPair[kColorTableLength]));
    resources[kDisplayPropertiesID] = base::StringPiece(
                                          reinterpret_cast<const char *>(display_properties_),
                                          sizeof(DisplayPropertyPair[kDisplayPropertiesSize]));

    int source_count = 1;
    int *end = source_images_;
    for (; *end != -1 ; end++)
    {
        source_count++;
    }
    resources[kSourceImagesID] = base::StringPiece(
                                     reinterpret_cast<const char *>(source_images_),
                                     source_count * sizeof(*source_images_));

    // Store results of GetScaleFactorsAsString() in std::string as
    // base::StringPiece does not copy data in constructor.
    std::string scale_factors_string = GetScaleFactorsAsString(scale_factors_);
    resources[kScaleFactorsID] = scale_factors_string;

    AddRawImagesTo(image_memory_, &resources);

    RawImages reencoded_images;
    RepackImages(images_on_file_thread_, &reencoded_images);
    AddRawImagesTo(reencoded_images, &resources);

    return ui::DataPack::WritePack(path, resources, ui::DataPack::BINARY);
}

bool BililiveThemePack::GetTint(int id, color_utils::HSL *hsl) const
{
    if (tints_)
    {
        for (size_t i = 0; i < kTintTableLength; ++i)
        {
            if (tints_[i].id == id)
            {
                hsl->h = tints_[i].h;
                hsl->s = tints_[i].s;
                hsl->l = tints_[i].l;
                return true;
            }
        }
    }

    return false;
}

bool BililiveThemePack::GetColor(int id, SkColor *color) const
{
    if (colors_)
    {
        for (size_t i = 0; i < kColorTableLength; ++i)
        {
            if (colors_[i].id == id)
            {
                *color = colors_[i].color;
                return true;
            }
        }
    }

    return false;
}

bool BililiveThemePack::GetDisplayProperty(int id, int *result) const
{
    if (display_properties_)
    {
        for (size_t i = 0; i < kDisplayPropertiesSize; ++i)
        {
            if (display_properties_[i].id == id)
            {
                *result = display_properties_[i].property;
                return true;
            }
        }
    }

    return false;
}

gfx::Image BililiveThemePack::GetImageNamed(int idr_id)
{
    int prs_id = GetPersistentIDByIDR(idr_id);
    if (prs_id == -1)
    {
        return gfx::Image();
    }

    // Check if the image is cached.
    ImageCache::const_iterator image_iter = images_on_ui_thread_.find(prs_id);
    if (image_iter != images_on_ui_thread_.end())
    {
        return image_iter->second;
    }

    ThemeImagePngSource::PngMap png_map;
    for (size_t i = 0; i < scale_factors_.size(); ++i)
    {
        scoped_refptr<base::RefCountedMemory> memory =
            GetRawData(idr_id, scale_factors_[i]);
        if (memory.get())
        {
            png_map[scale_factors_[i]] = memory;
        }
    }
    if (!png_map.empty())
    {
        gfx::ImageSkia image_skia(new ThemeImagePngSource(png_map),
                                  ui::SCALE_FACTOR_100P);
        // |image_skia| takes ownership of ThemeImagePngSource.
        gfx::Image ret = gfx::Image(image_skia);
        images_on_ui_thread_[prs_id] = ret;
        return ret;
    }

    return gfx::Image();
}

base::RefCountedMemory *BililiveThemePack::GetRawData(
    int idr_id,
    ui::ScaleFactor scale_factor) const
{
    base::RefCountedMemory *memory = NULL;
    int prs_id = GetPersistentIDByIDR(idr_id);
    int raw_id = GetRawIDByPersistentID(prs_id, scale_factor);

    if (raw_id != -1)
    {
        if (data_pack_.get())
        {
            memory = data_pack_->GetStaticMemory(raw_id);
        }
        else
        {
            RawImages::const_iterator it = image_memory_.find(raw_id);
            if (it != image_memory_.end())
            {
                memory = it->second.get();
            }
        }
    }

    return memory;
}

bool BililiveThemePack::HasCustomImage(int idr_id) const
{
    int prs_id = GetPersistentIDByIDR(idr_id);
    if (prs_id == -1)
    {
        return false;
    }

    int *img = source_images_;
    for (; *img != -1; ++img)
    {
        if (*img == prs_id)
        {
            return true;
        }
    }

    return false;
}

// private:

BililiveThemePack::BililiveThemePack()
    : CustomThemeSupplier(EXTENSION),
      header_(NULL),
      tints_(NULL),
      colors_(NULL),
      display_properties_(NULL),
      source_images_(NULL)
{
    scale_factors_ = ui::GetSupportedScaleFactors();
}

void BililiveThemePack::BuildHeader(const Extension *extension)
{
    header_ = new BililiveThemePackHeader;
    header_->version = kThemePackVersion;

    // TODO(erg): Need to make this endian safe on other computers. Prerequisite
    // is that ui::DataPack removes this same check.
#if defined(__BYTE_ORDER)
    // Linux check
    COMPILE_ASSERT(__BYTE_ORDER == __LITTLE_ENDIAN,
                   datapack_assumes_little_endian);
#elif defined(__BIG_ENDIAN__)
    // Mac check
#error DataPack assumes little endian
#endif
    header_->little_endian = 1;

    const std::string &id = extension->id();
    memcpy(header_->theme_id, id.c_str(), kIdSize);
}

void BililiveThemePack::BuildTintsFromJSON(
    const base::DictionaryValue *tints_value)
{
    tints_ = new TintEntry[kTintTableLength];
    for (size_t i = 0; i < kTintTableLength; ++i)
    {
        tints_[i].id = -1;
        tints_[i].h = -1;
        tints_[i].s = -1;
        tints_[i].l = -1;
    }

    if (!tints_value)
    {
        return;
    }

    // Parse the incoming data from |tints_value| into an intermediary structure.
    std::map<int, color_utils::HSL> temp_tints;
    for (DictionaryValue::Iterator iter(*tints_value); !iter.IsAtEnd();
         iter.Advance())
    {
        const ListValue *tint_list;
        if (iter.value().GetAsList(&tint_list) &&
            (tint_list->GetSize() == 3))
        {
            color_utils::HSL hsl = { -1, -1, -1 };

            if (tint_list->GetDouble(0, &hsl.h) &&
                tint_list->GetDouble(1, &hsl.s) &&
                tint_list->GetDouble(2, &hsl.l))
            {
                int id = GetIntForString(iter.key(), kTintTable, kTintTableLength);
                if (id != -1)
                {
                    temp_tints[id] = hsl;
                }
            }
        }
    }

    // Copy data from the intermediary data structure to the array.
    size_t count = 0;
    for (std::map<int, color_utils::HSL>::const_iterator it =
             temp_tints.begin();
         it != temp_tints.end() && count < kTintTableLength;
         ++it, ++count)
    {
        tints_[count].id = it->first;
        tints_[count].h = it->second.h;
        tints_[count].s = it->second.s;
        tints_[count].l = it->second.l;
    }
}

void BililiveThemePack::BuildColorsFromJSON(
    const base::DictionaryValue *colors_value)
{
    colors_ = new ColorPair[kColorTableLength];
    for (size_t i = 0; i < kColorTableLength; ++i)
    {
        colors_[i].id = -1;
        colors_[i].color = SkColorSetRGB(0, 0, 0);
    }

    std::map<int, SkColor> temp_colors;
    if (colors_value)
    {
        ReadColorsFromJSON(colors_value, &temp_colors);
    }
    GenerateMissingColors(&temp_colors);

    // Copy data from the intermediary data structure to the array.
    size_t count = 0;
    for (std::map<int, SkColor>::const_iterator it = temp_colors.begin();
         it != temp_colors.end() && count < kColorTableLength; ++it, ++count)
    {
        colors_[count].id = it->first;
        colors_[count].color = it->second;
    }
}

void BililiveThemePack::ReadColorsFromJSON(
    const base::DictionaryValue *colors_value,
    std::map<int, SkColor> *temp_colors)
{
    // Parse the incoming data from |colors_value| into an intermediary structure.
    for (DictionaryValue::Iterator iter(*colors_value); !iter.IsAtEnd();
         iter.Advance())
    {
        const ListValue *color_list;
        if (iter.value().GetAsList(&color_list) &&
            ((color_list->GetSize() == 3) || (color_list->GetSize() == 4)))
        {
            SkColor color = SK_ColorWHITE;
            int r, g, b;
            if (color_list->GetInteger(0, &r) &&
                color_list->GetInteger(1, &g) &&
                color_list->GetInteger(2, &b))
            {
                if (color_list->GetSize() == 4)
                {
                    double alpha;
                    int alpha_int;
                    if (color_list->GetDouble(3, &alpha))
                    {
                        color = SkColorSetARGB(static_cast<int>(alpha * 255), r, g, b);
                    }
                    else if (color_list->GetInteger(3, &alpha_int) &&
                             (alpha_int == 0 || alpha_int == 1))
                    {
                        color = SkColorSetARGB(alpha_int ? 255 : 0, r, g, b);
                    }
                    else
                    {
                        // Invalid entry for part 4.
                        continue;
                    }
                }
                else
                {
                    color = SkColorSetRGB(r, g, b);
                }

                int id = GetIntForString(iter.key(), kColorTable, kColorTableLength);
                if (id != -1)
                {
                    (*temp_colors)[id] = color;
                }
            }
        }
    }
}

void BililiveThemePack::GenerateMissingColors(
    std::map<int, SkColor> *colors)
{
    // Generate link colors, if missing. (See GetColor()).
    if (!colors->count(ThemeProperties::COLOR_NTP_HEADER) &&
        colors->count(ThemeProperties::COLOR_NTP_SECTION))
    {
        (*colors)[ThemeProperties::COLOR_NTP_HEADER] =
            (*colors)[ThemeProperties::COLOR_NTP_SECTION];
    }

    if (!colors->count(ThemeProperties::COLOR_NTP_SECTION_LINK_UNDERLINE) &&
        colors->count(ThemeProperties::COLOR_NTP_SECTION_LINK))
    {
        SkColor color_section_link =
            (*colors)[ThemeProperties::COLOR_NTP_SECTION_LINK];
        (*colors)[ThemeProperties::COLOR_NTP_SECTION_LINK_UNDERLINE] =
            SkColorSetA(color_section_link, SkColorGetA(color_section_link) / 3);
    }

    if (!colors->count(ThemeProperties::COLOR_NTP_LINK_UNDERLINE) &&
        colors->count(ThemeProperties::COLOR_NTP_LINK))
    {
        SkColor color_link = (*colors)[ThemeProperties::COLOR_NTP_LINK];
        (*colors)[ThemeProperties::COLOR_NTP_LINK_UNDERLINE] =
            SkColorSetA(color_link, SkColorGetA(color_link) / 3);
    }

    // Generate frame colors, if missing. (See GenerateFrameColors()).
    SkColor frame;
    std::map<int, SkColor>::const_iterator it =
        colors->find(ThemeProperties::COLOR_FRAME);
    if (it != colors->end())
    {
        frame = it->second;
    }
    else
    {
        frame = ThemeProperties::GetDefaultColor(
                    ThemeProperties::COLOR_FRAME);
    }

    if (!colors->count(ThemeProperties::COLOR_FRAME))
    {
        (*colors)[ThemeProperties::COLOR_FRAME] =
            HSLShift(frame, GetTintInternal(ThemeProperties::TINT_FRAME));
    }
    if (!colors->count(ThemeProperties::COLOR_FRAME_INACTIVE))
    {
        (*colors)[ThemeProperties::COLOR_FRAME_INACTIVE] =
            HSLShift(frame, GetTintInternal(
                         ThemeProperties::TINT_FRAME_INACTIVE));
    }
    if (!colors->count(ThemeProperties::COLOR_FRAME_INCOGNITO))
    {
        (*colors)[ThemeProperties::COLOR_FRAME_INCOGNITO] =
            HSLShift(frame, GetTintInternal(
                         ThemeProperties::TINT_FRAME_INCOGNITO));
    }
    if (!colors->count(ThemeProperties::COLOR_FRAME_INCOGNITO_INACTIVE))
    {
        (*colors)[ThemeProperties::COLOR_FRAME_INCOGNITO_INACTIVE] =
            HSLShift(frame, GetTintInternal(
                         ThemeProperties::TINT_FRAME_INCOGNITO_INACTIVE));
    }
}

void BililiveThemePack::BuildDisplayPropertiesFromJSON(
    const base::DictionaryValue *display_properties_value)
{
    display_properties_ = new DisplayPropertyPair[kDisplayPropertiesSize];
    for (size_t i = 0; i < kDisplayPropertiesSize; ++i)
    {
        display_properties_[i].id = -1;
        display_properties_[i].property = 0;
    }

    if (!display_properties_value)
    {
        return;
    }

    std::map<int, int> temp_properties;
    for (DictionaryValue::Iterator iter(*display_properties_value);
         !iter.IsAtEnd(); iter.Advance())
    {
        int property_id = GetIntForString(iter.key(), kDisplayProperties,
                                          kDisplayPropertiesSize);
        switch (property_id)
        {
        case ThemeProperties::NTP_BACKGROUND_ALIGNMENT:
        {
            std::string val;
            if (iter.value().GetAsString(&val))
            {
                temp_properties[ThemeProperties::NTP_BACKGROUND_ALIGNMENT] =
                    ThemeProperties::StringToAlignment(val);
            }
            break;
        }
        case ThemeProperties::NTP_BACKGROUND_TILING:
        {
            std::string val;
            if (iter.value().GetAsString(&val))
            {
                temp_properties[ThemeProperties::NTP_BACKGROUND_TILING] =
                    ThemeProperties::StringToTiling(val);
            }
            break;
        }
        case ThemeProperties::NTP_LOGO_ALTERNATE:
        {
            int val = 0;
            if (iter.value().GetAsInteger(&val))
            {
                temp_properties[ThemeProperties::NTP_LOGO_ALTERNATE] = val;
            }
            break;
        }
        }
    }

    // Copy data from the intermediary data structure to the array.
    size_t count = 0;
    for (std::map<int, int>::const_iterator it = temp_properties.begin();
         it != temp_properties.end() && count < kDisplayPropertiesSize;
         ++it, ++count)
    {
        display_properties_[count].id = it->first;
        display_properties_[count].property = it->second;
    }
}

void BililiveThemePack::ParseImageNamesFromJSON(
    const base::DictionaryValue *images_value,
    const base::FilePath &images_path,
    FilePathMap *file_paths) const
{
    if (!images_value)
    {
        return;
    }

    for (DictionaryValue::Iterator iter(*images_value); !iter.IsAtEnd();
         iter.Advance())
    {
        if (iter.value().IsType(Value::TYPE_DICTIONARY))
        {
            const DictionaryValue *inner_value = NULL;
            if (iter.value().GetAsDictionary(&inner_value))
            {
                for (DictionaryValue::Iterator inner_iter(*inner_value);
                     !inner_iter.IsAtEnd();
                     inner_iter.Advance())
                {
                    std::string name;
                    ui::ScaleFactor scale_factor = ui::SCALE_FACTOR_NONE;
                    if (GetScaleFactorFromManifestKey(inner_iter.key(), &scale_factor) &&
                        inner_iter.value().IsType(Value::TYPE_STRING) &&
                        inner_iter.value().GetAsString(&name))
                    {
                        AddFileAtScaleToMap(iter.key(),
                                            scale_factor,
                                            images_path.AppendASCII(name),
                                            file_paths);
                    }
                }
            }
        }
        else if (iter.value().IsType(Value::TYPE_STRING))
        {
            std::string name;
            if (iter.value().GetAsString(&name))
            {
                AddFileAtScaleToMap(iter.key(),
                                    ui::SCALE_FACTOR_100P,
                                    images_path.AppendASCII(name),
                                    file_paths);
            }
        }
    }
}

void BililiveThemePack::AddFileAtScaleToMap(const std::string &image_name,
        ui::ScaleFactor scale_factor,
        const base::FilePath &image_path,
        FilePathMap *file_paths) const
{
    int id = GetPersistentIDByName(image_name);
    if (id != -1)
    {
        (*file_paths)[id][scale_factor] = image_path;
    }
#if defined(OS_WIN) && defined(USE_AURA)
    id = GetPersistentIDByNameHelper(image_name,
                                     kPersistingImagesWinDesktopAura,
                                     kPersistingImagesWinDesktopAuraLength);
    if (id != -1)
    {
        (*file_paths)[id][scale_factor] = image_path;
    }
#endif
}

void BililiveThemePack::BuildSourceImagesArray(const FilePathMap &file_paths)
{
    std::vector<int> ids;
    for (FilePathMap::const_iterator it = file_paths.begin();
         it != file_paths.end(); ++it)
    {
        ids.push_back(it->first);
    }

    source_images_ = new int[ids.size() + 1];
    std::copy(ids.begin(), ids.end(), source_images_);
    source_images_[ids.size()] = -1;
}

bool BililiveThemePack::LoadRawBitmapsTo(
    const FilePathMap &file_paths,
    ImageCache *image_cache)
{
    // Themes should be loaded on the file thread, not the UI thread.
    // http://crbug.com/61838
    base::ThreadRestrictions::ScopedAllowIO allow_io;

    for (FilePathMap::const_iterator it = file_paths.begin();
         it != file_paths.end(); ++it)
    {
        int prs_id = it->first;
        // Some images need to go directly into |image_memory_|. No modification is
        // necessary or desirable.
        bool is_copyable = false;
        gfx::ImageSkia image_skia;
        for (int pass = 0; pass < 2; ++pass)
        {
            // Two passes: In the first pass, we process only scale factor
            // 100% and in the second pass all other scale factors. We
            // process scale factor 100% first because the first image added
            // in image_skia.AddRepresentation() determines the DIP size for
            // all representations.
            for (ScaleFactorToFileMap::const_iterator s2f = it->second.begin();
                 s2f != it->second.end(); ++s2f)
            {
                ui::ScaleFactor scale_factor = s2f->first;
                if ((pass == 0 && scale_factor != ui::SCALE_FACTOR_100P) ||
                    (pass == 1 && scale_factor == ui::SCALE_FACTOR_100P))
                {
                    continue;
                }
                scoped_refptr<base::RefCountedMemory> raw_data(
                    ReadFileData(s2f->second));
                if (!raw_data.get() || !raw_data->size())
                {
                    LOG(ERROR) << "Could not load theme image"
                               << " prs_id=" << prs_id
                               << " scale_factor_enum=" << scale_factor
                               << " file=" << s2f->second.value()
                               << (raw_data.get() ? " (zero size)" : " (read error)");
                    return false;
                }
                if (is_copyable)
                {
                    int raw_id = GetRawIDByPersistentID(prs_id, scale_factor);
                    image_memory_[raw_id] = raw_data;
                }
                else
                {
                    SkBitmap bitmap;
                    if (gfx::PNGCodec::Decode(raw_data->front(), raw_data->size(),
                                              &bitmap))
                    {
                        image_skia.AddRepresentation(
                            gfx::ImageSkiaRep(bitmap, scale_factor));
                    }
                    else
                    {
                        NOTREACHED() << "Unable to decode theme image resource "
                                     << it->first;
                    }
                }
            }
        }
        if (!is_copyable && !image_skia.isNull())
        {
            (*image_cache)[prs_id] = gfx::Image(image_skia);
        }
    }

    return true;
}

void BililiveThemePack::CreateImages(ImageCache *images) const
{
    CropImages(images);
    CreateFrameImages(images);
    CreateTintedButtons(GetTintInternal(ThemeProperties::TINT_BUTTONS), images);
}

void BililiveThemePack::CropImages(ImageCache *images) const
{
    bool has_frame_border = HasFrameBorder();
    for (size_t i = 0; i < arraysize(kImagesToCrop); ++i)
    {
        if (has_frame_border && kImagesToCrop[i].skip_if_frame_border)
        {
            continue;
        }

        int prs_id = kImagesToCrop[i].prs_id;
        ImageCache::iterator it = images->find(prs_id);
        if (it == images->end())
        {
            continue;
        }

        int crop_height = kImagesToCrop[i].max_height;
        gfx::ImageSkia image_skia = it->second.AsImageSkia();
        (*images)[prs_id] = gfx::Image(gfx::ImageSkiaOperations::ExtractSubset(
                                           image_skia, gfx::Rect(0, 0, image_skia.width(), crop_height)));
    }
}

void BililiveThemePack::CreateFrameImages(ImageCache *images) const
{
    ResourceBundle &rb = ResourceBundle::GetSharedInstance();

    // Create all the output images in a separate cache and move them back into
    // the input images because there can be name collisions.
    ImageCache temp_output;

    for (size_t i = 0; i < arraysize(kFrameTintMap); ++i)
    {
        int prs_id = kFrameTintMap[i].key;
        gfx::Image frame;
        // If there's no frame image provided for the specified id, then load
        // the default provided frame. If that's not provided, skip this whole
        // thing and just use the default images.
        int prs_base_id = 0;

#if defined(OS_WIN) && defined(USE_AURA)
        if (prs_id == PRS_THEME_FRAME_INCOGNITO_INACTIVE_WIN)
        {
            prs_base_id = images->count(PRS_THEME_FRAME_INCOGNITO_WIN) ?
                          PRS_THEME_FRAME_INCOGNITO_WIN : PRS_THEME_FRAME_WIN;
        }
        else if (prs_id == PRS_THEME_FRAME_INACTIVE_WIN)
        {
            prs_base_id = PRS_THEME_FRAME_WIN;
        }
        else if (prs_id == PRS_THEME_FRAME_INCOGNITO_WIN &&
                 !images->count(PRS_THEME_FRAME_INCOGNITO_WIN))
        {
            prs_base_id = PRS_THEME_FRAME_WIN;
        }
#endif
        if (!prs_base_id)
        {
            prs_base_id = prs_id;
        }
        if (images->count(prs_id))
        {
            frame = (*images)[prs_id];
        }
        else if (prs_base_id != prs_id && images->count(prs_base_id))
        {
            frame = (*images)[prs_base_id];
        }
        else
        {
            // If the theme doesn't specify an image, then apply the tint to
            // the default frame.
            //frame = rb.GetImageNamed(IDR_THEME_FRAME);
#if defined(OS_WIN) && defined(USE_AURA)
            if (prs_id >= PRS_THEME_FRAME_WIN &&
                prs_id <= PRS_THEME_FRAME_INCOGNITO_INACTIVE_WIN)
            {
                frame = rb.GetImageNamed(IDR_THEME_FRAME_WIN);
            }
#endif
        }
        if (!frame.IsEmpty())
        {
            temp_output[prs_id] = CreateHSLShiftedImage(
                                      frame, GetTintInternal(kFrameTintMap[i].value));
        }
    }
    MergeImageCaches(temp_output, images);
}

void BililiveThemePack::CreateTintedButtons(
    const color_utils::HSL &button_tint,
    ImageCache *processed_images) const
{
    if (button_tint.h != -1 || button_tint.s != -1 || button_tint.l != -1)
    {
        ResourceBundle &rb = ResourceBundle::GetSharedInstance();
        const std::set<int> &idr_ids =
            ThemeProperties::GetTintableToolbarButtons();
        for (std::set<int>::const_iterator it = idr_ids.begin();
             it != idr_ids.end(); ++it)
        {
            int prs_id = GetPersistentIDByIDR(*it);
            DCHECK(prs_id > 0);

            // Fetch the image by IDR...
            gfx::Image &button = rb.GetImageNamed(*it);

            // but save a version with the persistent ID.
            (*processed_images)[prs_id] =
                CreateHSLShiftedImage(button, button_tint);
        }
    }
}

void BililiveThemePack::RepackImages(const ImageCache &images,
                                     RawImages *reencoded_images) const
{
    typedef std::vector<ui::ScaleFactor> ScaleFactors;
    for (ImageCache::const_iterator it = images.begin();
         it != images.end(); ++it)
    {
        gfx::ImageSkia image_skia = *it->second.ToImageSkia();

        typedef std::vector<gfx::ImageSkiaRep> ImageSkiaReps;
        ImageSkiaReps image_reps = image_skia.image_reps();
        if (image_reps.empty())
        {
            NOTREACHED() << "No image reps for resource " << it->first << ".";
        }
        for (ImageSkiaReps::iterator rep_it = image_reps.begin();
             rep_it != image_reps.end(); ++rep_it)
        {
            std::vector<unsigned char> bitmap_data;
            if (!gfx::PNGCodec::EncodeBGRASkBitmap(rep_it->sk_bitmap(), false,
                                                   &bitmap_data))
            {
                NOTREACHED() << "Image file for resource " << it->first
                             << " could not be encoded.";
            }
            int raw_id = GetRawIDByPersistentID(it->first, rep_it->scale_factor());
            (*reencoded_images)[raw_id] =
                base::RefCountedBytes::TakeVector(&bitmap_data);
        }
    }
}

void BililiveThemePack::MergeImageCaches(
    const ImageCache &source, ImageCache *destination) const
{
    for (ImageCache::const_iterator it = source.begin(); it != source.end();
         ++it)
    {
        (*destination)[it->first] = it->second;
    }
}

void BililiveThemePack::AddRawImagesTo(const RawImages &images,
                                       RawDataForWriting *out) const
{
    for (RawImages::const_iterator it = images.begin(); it != images.end();
         ++it)
    {
        (*out)[it->first] = base::StringPiece(
                                reinterpret_cast<const char *>(it->second->front()), it->second->size());
    }
}

color_utils::HSL BililiveThemePack::GetTintInternal(int id) const
{
    if (tints_)
    {
        for (size_t i = 0; i < kTintTableLength; ++i)
        {
            if (tints_[i].id == id)
            {
                color_utils::HSL hsl;
                hsl.h = tints_[i].h;
                hsl.s = tints_[i].s;
                hsl.l = tints_[i].l;
                return hsl;
            }
        }
    }

    return ThemeProperties::GetDefaultTint(id);
}

int BililiveThemePack::GetRawIDByPersistentID(
    int prs_id,
    ui::ScaleFactor scale_factor) const
{
    if (prs_id < 0)
    {
        return -1;
    }

    for (size_t i = 0; i < scale_factors_.size(); ++i)
    {
        if (scale_factors_[i] == scale_factor)
        {
            return static_cast<int>(kPersistingImagesLength * i) + prs_id;
        }
    }
    return -1;
}

bool BililiveThemePack::GetScaleFactorFromManifestKey(
    const std::string &key,
    ui::ScaleFactor *scale_factor) const
{
    int percent = 0;
    if (base::StringToInt(key, &percent))
    {
        float scale = static_cast<float>(percent) / 100.0f;
        for (size_t i = 0; i < scale_factors_.size(); ++i)
        {
            if (fabs(ui::GetScaleFactorScale(scale_factors_[i]) - scale) < 0.001)
            {
                *scale_factor = scale_factors_[i];
                return true;
            }
        }
    }
    return false;
}

void BililiveThemePack::GenerateRawImageForAllSupportedScales(int prs_id)
{
    // Compute (by scaling) bitmaps for |prs_id| for any scale factors
    // for which the theme author did not provide a bitmap. We compute
    // the bitmaps using the highest scale factor that theme author
    // provided.
    // Note: We use only supported scale factors. For example, if scale
    // factor 2x is supported by the current system, but 1.8x is not and
    // if the theme author did not provide an image for 2x but one for
    // 1.8x, we will not use the 1.8x image here. Here we will only use
    // images provided for scale factors supported by the current system.

    // See if any image is missing. If not, we're done.
    bool image_missing = false;
    for (size_t i = 0; i < scale_factors_.size(); ++i)
    {
        int raw_id = GetRawIDByPersistentID(prs_id, scale_factors_[i]);
        if (image_memory_.find(raw_id) == image_memory_.end())
        {
            image_missing = true;
            break;
        }
    }
    if (!image_missing)
    {
        return;
    }

    // Find available scale factor with highest scale.
    ui::ScaleFactor available_scale_factor = ui::SCALE_FACTOR_NONE;
    for (size_t i = 0; i < scale_factors_.size(); ++i)
    {
        int raw_id = GetRawIDByPersistentID(prs_id, scale_factors_[i]);
        if ((available_scale_factor == ui::SCALE_FACTOR_NONE ||
             (ui::GetScaleFactorScale(scale_factors_[i]) >
              ui::GetScaleFactorScale(available_scale_factor))) &&
            image_memory_.find(raw_id) != image_memory_.end())
        {
            available_scale_factor = scale_factors_[i];
        }
    }
    // If no scale factor is available, we're done.
    if (available_scale_factor == ui::SCALE_FACTOR_NONE)
    {
        return;
    }

    // Get bitmap for the available scale factor.
    int available_raw_id = GetRawIDByPersistentID(prs_id, available_scale_factor);
    RawImages::const_iterator it = image_memory_.find(available_raw_id);
    SkBitmap available_bitmap;
    if (!gfx::PNGCodec::Decode(it->second->front(),
                               it->second->size(),
                               &available_bitmap))
    {
        NOTREACHED() << "Unable to decode theme image for prs_id="
                     << prs_id << " for scale_factor=" << available_scale_factor;
        return;
    }

    // Fill in all missing scale factors by scaling the available bitmap.
    for (size_t i = 0; i < scale_factors_.size(); ++i)
    {
        int scaled_raw_id = GetRawIDByPersistentID(prs_id, scale_factors_[i]);
        if (image_memory_.find(scaled_raw_id) != image_memory_.end())
        {
            continue;
        }
        SkBitmap scaled_bitmap =
            CreateLowQualityResizedBitmap(available_bitmap,
                                          available_scale_factor,
                                          scale_factors_[i]);
        std::vector<unsigned char> bitmap_data;
        if (!gfx::PNGCodec::EncodeBGRASkBitmap(scaled_bitmap,
                                               false,
                                               &bitmap_data))
        {
            NOTREACHED() << "Unable to encode theme image for prs_id="
                         << prs_id << " for scale_factor=" << scale_factors_[i];
            break;
        }
        image_memory_[scaled_raw_id] =
            base::RefCountedBytes::TakeVector(&bitmap_data);
    }
}