#ifndef OBS_OBS_PROXY_CORE_PROXY_SCENE_COLLECTION_OBS_SOURCE_PROPERTIES_IMPL_H_
#define OBS_OBS_PROXY_CORE_PROXY_SCENE_COLLECTION_OBS_SOURCE_PROPERTIES_IMPL_H_

#include "bilibase/basic_macros.h"

#include "obs/obs-studio/libobs/obs.h"

#include "obs/obs_proxy/public/proxy/obs_source_properties.h"

namespace obs_proxy {

class PropertyImpl : public virtual Property {
public:
    explicit PropertyImpl(obs_property_t* prop);

    ~PropertyImpl();

    DEFAULT_COPY(PropertyImpl);

    std::string name() const override;

    PropertyType type() const override;

    std::string description() const override;

    obs_property_t* LeakUnderlying() const;

    PropertyImpl NextProperty() const;

    operator bool() const;

protected:
    void* EvilLeakUnderlying() const override;

private:
    obs_property_t* property_;
};

bool operator==(const PropertyImpl& lhs, const PropertyImpl& rhs);
bool operator!=(const PropertyImpl& lhs, const PropertyImpl& rhs);

class PropertiesImpl : public Properties {
public:
    explicit PropertiesImpl(const obs_source_t* source);

    ~PropertiesImpl();

    DISABLE_COPY(PropertiesImpl);

    iterator begin() const override;

    iterator end() const override;

    iterator find(const std::string& name) const override;

    void Refresh();

private:
    obs_properties_t* properties_;
    const obs_source_t* bound_source_view_;
};

// Visual Studio has a known issue of warning all virtual functions inherited via dominance since
// ver 2005, and Microsoft refuses to fix it but recommends disabling this warning as you need.
#pragma warning(push)
#pragma warning(disable: 4250)

class BoolPropertyImpl : public PropertyImpl, public BoolProperty {
public:
    explicit BoolPropertyImpl(obs_property_t* prop);

    ~BoolPropertyImpl();

    DEFAULT_COPY(BoolPropertyImpl);
};

class ButtonPropertyImpl : public PropertyImpl, public ButtonProperty {
public:
    explicit ButtonPropertyImpl(obs_property_t* prop);

    ~ButtonPropertyImpl();

    DEFAULT_COPY(ButtonPropertyImpl);

    bool Click(SceneItem* item) override;
};

class ColorPropertyImpl : public PropertyImpl, public ColorProperty {
public:
    explicit ColorPropertyImpl(obs_property_t* prop);

    ~ColorPropertyImpl();

    DEFAULT_COPY(ColorPropertyImpl);
};

class EditableListPropertyImpl : public PropertyImpl, public EditableListProperty {
public:
    explicit EditableListPropertyImpl(obs_property_t* prop);

    ~EditableListPropertyImpl();

    DEFAULT_COPY(EditableListPropertyImpl);
};

class IntPropertyImpl : public PropertyImpl, public IntProperty {
public:
    explicit IntPropertyImpl(obs_property_t* prop);

    ~IntPropertyImpl();

    DEFAULT_COPY(IntPropertyImpl);

    NumberType int_type() const override;

    int int_min() const override;

    int int_max() const override;

    int int_step() const override;
};

class FloatPropertyImpl : public PropertyImpl, public FloatProperty {
public:
    explicit FloatPropertyImpl(obs_property_t* prop);

    ~FloatPropertyImpl();

    DEFAULT_COPY(FloatPropertyImpl);

    NumberType float_type() const override;

    double float_min() const override;

    double float_max() const override;

    double float_step() const override;
};

class TextPropertyImpl : public PropertyImpl, public TextProperty {
public:
    explicit TextPropertyImpl(obs_property_t* prop);

    ~TextPropertyImpl();

    DEFAULT_COPY(TextPropertyImpl);

    TextType text_type() const override;
};

class PathPropertyImpl : public PropertyImpl, public PathProperty {
public:
    explicit PathPropertyImpl(obs_property_t* prop);

    ~PathPropertyImpl();

    DEFAULT_COPY(PathPropertyImpl);

    PathType path_type() const override;
};

class ListPropertyImpl : public PropertyImpl, public ListProperty {
public:
    explicit ListPropertyImpl(obs_property_t* prop);

    ~ListPropertyImpl();

    ComboFormat list_format() const override;

    ComboType list_type() const override;

    size_t GetItemCount() const override;

    std::string GetItemName(size_t index) const override;

    long long GetItemValueAsInt(size_t index) const override;

    double GetItemValueAsFloat(size_t index) const override;

    std::string GetItemValueAsString(size_t index) const override;

	bool GetItemEnableStatus(size_t index) const override;
};

class FontPropertyImpl : public PropertyImpl, public FontProperty {
public:
    explicit FontPropertyImpl(obs_property_t* prop);

    ~FontPropertyImpl();

    DEFAULT_COPY(FontPropertyImpl);
};

class FrameRatePropertyImpl : public PropertyImpl, public FrameRateProperty {
public:
    explicit FrameRatePropertyImpl(obs_property_t* prop);

    ~FrameRatePropertyImpl();

    size_t fps_ranges_count() const override;

    MediaFramesPerSecond fps_range_min(size_t index) const override;

    MediaFramesPerSecond fps_range_max(size_t index) const override;

    size_t options_count() const override;

    std::string GetOptionName(size_t index) const override;

    std::string GetOptionDescription(size_t index) const override;
};

#pragma warning(pop)

}   // namespace obs_proxy

#endif  // OBS_OBS_PROXY_CORE_PROXY_SCENE_COLLECTION_OBS_SOURCE_PROPERTIES_IMPL_H_