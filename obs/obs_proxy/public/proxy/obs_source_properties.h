#ifndef OBS_OBS_PROXY_PUBLIC_PROXY_OBS_SOURCE_PROPERTIES_H_
#define OBS_OBS_PROXY_PUBLIC_PROXY_OBS_SOURCE_PROPERTIES_H_

#include <memory>

#include "bilibase/basic_macros.h"

#include "obs/obs_proxy/app/obs_proxy_export.h"

namespace obs_proxy {

class SceneItem;

enum class PropertiesRefreshMode {
    NoRefresh,
    Refresh
};

// Must keep consistent with obs definitions.
enum class PropertyType {
    Invalid = 0,
    Bool,
    Int,
    Float,
    Text,
    Path,
    List,
    Color,
    Button,
    Font,
    EditableList,
    FrameRate
};

class Property {
public:
    virtual ~Property() {}

    virtual std::string name() const = 0;

    virtual PropertyType type() const = 0;

    virtual std::string description() const = 0;

protected:
    template<typename P>
    friend std::unique_ptr<P> property_cast(const Property& prop);

    // We need to create a specific property via this common base type, however, we couldn't
    // just simply use dynamic_cast, which is coerced by virtual inheritance, to complete
    // the down-cast, because RTTI is disabled.
    // Therefore, this function comes to do some dirty workaround.
    virtual void* EvilLeakUnderlying() const = 0;
};

class OBS_PROXY_EXPORT PropertyIterator {
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = Property;
    using difference_type = ptrdiff_t;
    using pointer = const Property*;
    using reference = const Property&;

    // Creates an empty iterator. Note it is internal impl is also empty.
    // We allow empty impl here, due to performance considerations.
    PropertyIterator();

    template<typename P>
    explicit PropertyIterator(const P& prop);

    ~PropertyIterator();

    DEFAULT_COPY(PropertyIterator);

    reference operator*() const;

    pointer operator->() const;

    PropertyIterator& operator++();

    // Since we internally share the real impl, and also with the input iterator category
    // enforced, this suffix increment has no de-facto difference with its prefix cousin.
    PropertyIterator operator++(int);

    friend bool operator==(const PropertyIterator& lhs, const PropertyIterator& rhs);

    friend bool operator!=(const PropertyIterator& lhs, const PropertyIterator& rhs);

private:
    // An iterator that is end is not incrementable.
    bool IsEnd() const;

private:
    class Impl;
    std::shared_ptr<Impl> impl_;
};

// Class `Properties` contains a bunch of properties a scene item type has already predefined.
class Properties {
public:
    using iterator = PropertyIterator;
    using const_iterator = iterator;

    virtual ~Properties() {}

    virtual iterator begin() const = 0;

    virtual iterator end() const = 0;

    virtual iterator find(const std::string& name) const = 0;
};

// Specific property interface classes below.

class BoolProperty : public virtual Property {};

class ColorProperty : public virtual Property {};

class EditableListProperty : public virtual Property {};

class ButtonProperty : public virtual Property {
public:
    virtual bool Click(SceneItem* item) = 0;
};

class IntProperty : public virtual Property {
public:
    enum class NumberType {
        SCROLLER,
        SLIDER
    };

    virtual NumberType int_type() const = 0;

    virtual int int_min() const = 0;

    virtual int int_max() const = 0;

    virtual int int_step() const = 0;
};

class FloatProperty : public virtual Property {
public:
    enum class NumberType {
        SCROLLER,
        SLIDER
    };

    virtual NumberType float_type() const = 0;

    virtual double float_min() const = 0;

    virtual double float_max() const = 0;

    virtual double float_step() const = 0;
};

class TextProperty : public virtual Property {
public:
    enum class TextType {
        DEFAULT,
        PASSWORD,
        MULTILINE
    };

    virtual TextType text_type() const = 0;
};

class PathProperty : public virtual Property {
public:
    enum class PathType {
        FILE,
        FILE_SAVE,
        DIRECTORY
    };

    virtual PathType path_type() const = 0;
};

class ListProperty : public virtual Property {
public:
    enum class ComboFormat {
        INVALID,
        INT,
        FLOAT,
        STRING
    };

    enum class ComboType {
        INVALID,
        EDITABLE,
        LIST
    };

    virtual ComboFormat list_format() const = 0;

    virtual ComboType list_type() const = 0;

    virtual size_t GetItemCount() const = 0;

    virtual std::string GetItemName(size_t index) const = 0;

    virtual long long GetItemValueAsInt(size_t index) const = 0;

    virtual double GetItemValueAsFloat(size_t index) const = 0;

    virtual std::string GetItemValueAsString(size_t index) const = 0;

	virtual bool GetItemEnableStatus(size_t index) const = 0;
};

class FontProperty : public virtual Property {
public:
    enum class FontStyle {
        BOLD = 1 << 0,
        ITALIC = 1 << 1,
        UNDERLINE = 1 << 2,
        STRIKEOUT = 1 << 3
    };
};

class FrameRateProperty : public virtual Property {
public:
    struct MediaFramesPerSecond {
        uint32_t numerator;
        uint32_t denominator;
    };

    virtual size_t fps_ranges_count() const = 0;

    virtual MediaFramesPerSecond fps_range_min(size_t index) const = 0;

    virtual MediaFramesPerSecond fps_range_max(size_t index) const = 0;

    virtual size_t options_count() const = 0;

    virtual std::string GetOptionName(size_t index) const = 0;

    virtual std::string GetOptionDescription(size_t index) const = 0;
};

}   // namespace obs_proxy

#endif  // OBS_OBS_PROXY_PUBLIC_PROXY_OBS_SOURCE_PROPERTIES_H_