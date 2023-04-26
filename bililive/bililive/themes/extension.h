#ifndef BILILIVE_COMMON_EXTENSIONS_EXTENSION_H
#define BILILIVE_COMMON_EXTENSIONS_EXTENSION_H

#include <string>
#include "base/files/file_path.h"
#include "base/memory/scoped_ptr.h"

namespace base
{
    class DictionaryValue;
};


class Extension
{
public:
    static Extension *Create(const std::string &id, const base::FilePath &path);

public:
    static const size_t kIdSize;

    static const char kMimeType[];

    static const base::FilePath::CharType kManifestFilename[];

    bool is_theme() const
    {
        return true;
    }
    base::DictionaryValue *GetThemeImages() const
    {
        return theme_images_.get();
    }
    base::DictionaryValue *GetThemeColors() const
    {
        return theme_colors_.get();
    }
    base::DictionaryValue *GetThemeTints() const
    {
        return theme_tints_.get();
    }
    base::DictionaryValue *GetThemeDisplayProperties() const
    {
        return theme_display_properties_.get();
    }
    base::DictionaryValue *GetThemeLayouts() const
    {
        return theme_layouts_.get();
    }

    const base::FilePath &path() const
    {
        return path_;
    }
    const std::string &id() const
    {
        return id_;
    }

public:
    ~Extension();

private:
    void set_id(const std::string &id)
    {
        id_ = id;
    }
    void set_path(const base::FilePath &path)
    {
        path_ = path;
    }
    bool LoadThemeImages(const base::DictionaryValue *theme_value);
    bool LoadThemeColors(const base::DictionaryValue *theme_value);
    bool LoadThemeTints(const base::DictionaryValue *theme_value);
    bool LoadThemeDisplayProperties(const base::DictionaryValue *theme_value);
    bool LoadThemeLayouts(const base::DictionaryValue *theme_value);

private:
    base::FilePath path_;
    std::string id_;

    scoped_ptr<base::DictionaryValue> theme_images_;
    scoped_ptr<base::DictionaryValue> theme_colors_;
    scoped_ptr<base::DictionaryValue> theme_tints_;
    scoped_ptr<base::DictionaryValue> theme_display_properties_;
    scoped_ptr<base::DictionaryValue> theme_layouts_;
};

#endif