#include "bililive/bililive/livehime/sources_properties/source_public_property_presenter.h"

class SourcePublicPropertyPresenter::Impl
{
public:
    explicit Impl(obs_proxy::SceneItem* scene_item);

    virtual ~Impl() {}

    string16 GetName();

    bool SetName(const string16& name);

    void Update();

    void UpdateFilters();

    obs_proxy::SceneItem* GetSceneItem() const;

private:
    std::unique_ptr<SceneItemHelper> item_;

    DISALLOW_COPY_AND_ASSIGN(Impl);
};

SourcePublicPropertyPresenter::Impl::Impl(
    obs_proxy::SceneItem* scene_item)
    : item_(std::make_unique<SceneItemHelper>(scene_item))
{}

string16 SourcePublicPropertyPresenter::Impl::GetName()
{
    return item_->Name();
}

bool SourcePublicPropertyPresenter::Impl::SetName(const string16& name)
{
    std::wstring source_name_utf8 = item_->Name();
    std::wstring new_name_utf8 = name;
    if (!new_name_utf8.empty() && source_name_utf8 != new_name_utf8)
    {
        return item_->Name(new_name_utf8);
    }

    return true;
}

void SourcePublicPropertyPresenter::Impl::Update()
{
    item_->Update();
}

void SourcePublicPropertyPresenter::Impl::UpdateFilters()
{
    item_->UpdateFilters();
}

obs_proxy::SceneItem* SourcePublicPropertyPresenter::Impl::GetSceneItem() const
{
    return item_->GetItem();
}


// SourcePublicPropertyPresenter
SourcePublicPropertyPresenter::SourcePublicPropertyPresenter(
    obs_proxy::SceneItem* scene_item)
    : presenter_(std::make_unique<Impl>(scene_item))
{}

SourcePublicPropertyPresenter::~SourcePublicPropertyPresenter()
{}

string16 SourcePublicPropertyPresenter::GetName()
{
    return presenter_->GetName();
}

bool SourcePublicPropertyPresenter::SetName(const string16& name)
{
    return presenter_->SetName(name);
}

void SourcePublicPropertyPresenter::Update()
{
    presenter_->Update();
}

void SourcePublicPropertyPresenter::UpdateFilters()
{
    presenter_->UpdateFilters();
}

obs_proxy::SceneItem* SourcePublicPropertyPresenter::GetSceneItem() const
{
    return presenter_->GetSceneItem();
}