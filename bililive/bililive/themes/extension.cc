#include "extension.h"
#include "base/values.h"
#include "base/file_util.h"
#include "base/json/json_file_value_serializer.h"

namespace
{

  const char kTheme[] = "theme";
  const char kThemeImages[] = "images";
  const char kThemeColors[] = "colors";
  const char kThemeTints[] = "tints";
  const char kThemeDisplayProperties[] = "properties";
  const char kThemeLayouts[] = "layouts";

  base::DictionaryValue *LoadManifest(const base::FilePath &extension_path)
  {
    base::FilePath manifest_path =
      extension_path.Append(Extension::kManifestFilename);
    if (!base::PathExists(manifest_path))
    {
      return NULL;
    }

    JSONFileValueSerializer serializer(manifest_path);
    scoped_ptr<base::Value> root(serializer.Deserialize(NULL, NULL));
    if (!root.get())
    {
      return NULL;
    }

    if (!root->IsType(base::Value::TYPE_DICTIONARY))
    {
      return NULL;
    }

    return static_cast<base::DictionaryValue *>(root.release());
  }

}

// first 16 bytes of SHA256 hashed public key.
const size_t Extension::kIdSize = 16;
const char Extension::kMimeType[] = "application/x-bililive-extension";
const base::FilePath::CharType Extension::kManifestFilename[] = FILE_PATH_LITERAL("manifest.json");

Extension *Extension::Create(const std::string &id, const base::FilePath &path)
{
  Extension *ret_val = NULL;

  do
  {
    scoped_ptr<base::DictionaryValue> manifest(LoadManifest(path));
    if(manifest.get() == NULL)
    {
      return NULL;
    }

    base::DictionaryValue *theme;
    if(!manifest->GetDictionaryWithoutPathExpansion(kTheme, &theme))
    {
      break;
    }

    scoped_ptr<Extension> extension(new Extension);

    if(!extension->LoadThemeImages(theme))
    {
      break;
    }

    if(!extension->LoadThemeColors(theme))
    {
      break;
    }

    if(!extension->LoadThemeTints(theme))
    {
      break;
    }

    if(!extension->LoadThemeDisplayProperties(theme))
    {
      break;
    }

    if(!extension->LoadThemeLayouts(theme))
    {
      break;
    }

    extension->set_id(id);
    extension->set_path(path);

    ret_val = extension.release();
  }
  while(false);

  return ret_val;
}

Extension::~Extension() {}

bool Extension::LoadThemeImages(const base::DictionaryValue *theme_value)
{
  const base::DictionaryValue *images_value = NULL;
  if (theme_value->GetDictionary(kThemeImages, &images_value))
  {
    // Validate that the images are all strings
    for (base::DictionaryValue::Iterator iter(*images_value);
         !iter.IsAtEnd(); iter.Advance())
    {
      std::string val;
      if (!images_value->GetString(iter.key(), &val))
      {
        return false;
      }
    }
    theme_images_.reset(images_value->DeepCopy());
  }
  return true;
}

bool Extension::LoadThemeColors(const base::DictionaryValue *theme_value)
{
  const base::DictionaryValue *colors_value = NULL;
  if (theme_value->GetDictionary(kThemeColors, &colors_value))
  {
    // Validate that the colors are RGB or RGBA lists
    for (base::DictionaryValue::Iterator iter(*colors_value);
         !iter.IsAtEnd(); iter.Advance())
    {
      const base::ListValue *color_list = NULL;
      double alpha = 0.0;
      int color = 0;
      // The color must be a list
      if (!colors_value->GetListWithoutPathExpansion(iter.key(), &color_list) ||
          // And either 3 items (RGB) or 4 (RGBA)
          ((color_list->GetSize() != 3) &&
           ((color_list->GetSize() != 4) ||
            // For RGBA, the fourth item must be a real or int alpha value.
            // Note that GetDouble() can get an integer value.
            !color_list->GetDouble(3, &alpha))) ||
          // For both RGB and RGBA, the first three items must be ints (R,G,B)
          !color_list->GetInteger(0, &color) ||
          !color_list->GetInteger(1, &color) ||
          !color_list->GetInteger(2, &color))
      {
        return false;
      }
    }
    theme_colors_.reset(colors_value->DeepCopy());
  }
  return true;
}

bool Extension::LoadThemeTints(const base::DictionaryValue *theme_value)
{
  const base::DictionaryValue *tints_value = NULL;
  if (theme_value->GetDictionary(kThemeTints, &tints_value))
  {
    // Validate that the tints are all reals.
    for (base::DictionaryValue::Iterator iter(*tints_value);
         !iter.IsAtEnd(); iter.Advance())
    {
      const base::ListValue *tint_list = NULL;
      double v = 0.0;
      if (!tints_value->GetListWithoutPathExpansion(iter.key(), &tint_list) ||
          tint_list->GetSize() != 3 ||
          !tint_list->GetDouble(0, &v) ||
          !tint_list->GetDouble(1, &v) ||
          !tint_list->GetDouble(2, &v))
      {
        return false;
      }
    }
    theme_tints_.reset(tints_value->DeepCopy());
  }
  return true;
}

bool Extension::LoadThemeDisplayProperties(const base::DictionaryValue *theme_value)
{
  const base::DictionaryValue *display_properties_value = NULL;
  if (theme_value->GetDictionary(kThemeDisplayProperties,
                                 &display_properties_value))
  {
    theme_display_properties_.reset(
      display_properties_value->DeepCopy());
  }
  return true;
}

bool Extension::LoadThemeLayouts(const base::DictionaryValue *theme_value)
{
  const base::DictionaryValue *layouts_value = NULL;
  if (theme_value->GetDictionary(kThemeLayouts, &layouts_value))
  {
    // Validate that the layouts are all strings
    for (base::DictionaryValue::Iterator iter(*layouts_value);
         !iter.IsAtEnd(); iter.Advance())
    {
      std::string val;
      if (!layouts_value->GetString(iter.key(), &val))
      {
        return false;
      }
    }
    theme_layouts_.reset(layouts_value->DeepCopy());
  }
  return true;
}