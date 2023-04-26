#include "source_browser_property_presenter_impl.h"

#include "base/logging.h"

SourceBrowserPropertyPresenterImpl::SourceBrowserPropertyPresenterImpl(obs_proxy::SceneItem* scene_item)
    :contracts::SourceBrowserPropertyPresenter(scene_item)
    , browser_scene_item_(scene_item)
{

}

void SourceBrowserPropertyPresenterImpl::SetUrl(const std::string& url)
{
    browser_scene_item_.SetUrl(url);
}

std::string SourceBrowserPropertyPresenterImpl::GetUrl()
{
    return browser_scene_item_.GetUrl();
}

void SourceBrowserPropertyPresenterImpl::SetCSS(const std::string& css)
{
    browser_scene_item_.SetCSS(css);
}

std::string SourceBrowserPropertyPresenterImpl::GetCSS()
{
    return browser_scene_item_.GetCSS();
}

void SourceBrowserPropertyPresenterImpl::SetWidth(int width)
{
    browser_scene_item_.width(width);
}

int SourceBrowserPropertyPresenterImpl::GetWidth()
{
    return browser_scene_item_.width();
}

void SourceBrowserPropertyPresenterImpl::SetHeight(int height)
{
    browser_scene_item_.height(height);
}

int SourceBrowserPropertyPresenterImpl::GetHeight()
{
    return browser_scene_item_.height();
}

void SourceBrowserPropertyPresenterImpl::Refresh()
{
    browser_scene_item_.Refresh();
}

void SourceBrowserPropertyPresenterImpl::Shutdown()
{
    browser_scene_item_.shutdown();
}


void SourceBrowserPropertyPresenterImpl::SetRestartWhenActive(bool use)
{
    browser_scene_item_.SetRestartWhenActive(use);
}

bool SourceBrowserPropertyPresenterImpl::GetRestartWhenActive()
{
    return browser_scene_item_.GetRestartWhenActive();
}

void SourceBrowserPropertyPresenterImpl::Snapshot()
{
    snapshot_ = livehime::BrowserPropertySnapshot::NewTake(&browser_scene_item_);
}

void SourceBrowserPropertyPresenterImpl::Restore()
{
    DCHECK(snapshot_);

    snapshot_->Restore();
}

float SourceBrowserPropertyPresenterImpl::GetVolumeValue()
{
    return MapFloatFromInt(browser_scene_item_.Volume());
}

void SourceBrowserPropertyPresenterImpl::SetVolumeValue(float value)
{
    browser_scene_item_.Volume(MapFloatToInt(value));
}

bool SourceBrowserPropertyPresenterImpl::IsMuted()
{
    return browser_scene_item_.IsMuted();
}

void SourceBrowserPropertyPresenterImpl::SetMuted(bool muted)
{
    browser_scene_item_.SetMuted(muted);
}