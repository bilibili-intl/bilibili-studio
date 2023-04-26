#ifndef BILILIVE_BILILIVE_LIVEHIME_SOURCE_WINDOW_PROPERTY_PRESENTER_H_
#define BILILIVE_BILILIVE_LIVEHIME_SOURCE_WINDOW_PROPERTY_PRESENTER_H_

#include "bililive/bililive/livehime/sources_properties/source_public_property_presenter.h"
#include "bililive/bililive/livehime/sources_properties/source_property_snapshot.h"

class SourceWindowPropertyPresenter 
	: public SourcePublicPropertyPresenter {
public:
	explicit SourceWindowPropertyPresenter(obs_proxy::SceneItem* scene_item)
		: SourcePublicPropertyPresenter(scene_item) {}

	virtual ~SourceWindowPropertyPresenter() {}

    virtual PropertyList<std::string> WindowList() = 0;
    virtual PropertyList<std::string> ModelList() = 0;

    virtual std::string SelectedWindow() = 0;
    virtual void SelectedWindow(std::string val) = 0;

    virtual bool IsCaptureCursor() = 0;
    virtual void IsCaptureCursor(bool val) = 0;

    virtual bool IsCompatibleMode() = 0;
    virtual void IsCompatibleMode(bool val) = 0;

    virtual bool IsSliMode() = 0;
    virtual void IsSliMode(bool val) = 0;

    virtual void SetCaptureModel(int model) = 0;
    virtual int GetCaptureModel() = 0;

    virtual void SetUseHwndSpesc(bool val) = 0;
    virtual bool GetUseHwndSpesc() = 0;

    virtual void SetHwndSpesc(long long hwnd) = 0;
    virtual long long GetHwndSpesc() = 0;
};


class SourceWindowPropertyPresenterImp
    : public SourceWindowPropertyPresenter {
public:
    explicit SourceWindowPropertyPresenterImp(obs_proxy::SceneItem* scene_item);
    virtual ~SourceWindowPropertyPresenterImp();

    virtual PropertyList<std::string> WindowList() override;
    virtual PropertyList<std::string> ModelList() override;

    virtual std::string SelectedWindow() override;
    virtual void SelectedWindow(std::string val) override;

    virtual bool IsCaptureCursor() override;
    virtual void IsCaptureCursor(bool val) override;

    virtual bool IsCompatibleMode() override;
    virtual void IsCompatibleMode(bool val) override;

    virtual bool IsSliMode() override;
    virtual void IsSliMode(bool val) override;

    virtual void SetCaptureModel(int model) override;
    virtual int GetCaptureModel() override;

    virtual void Snapshot() override;
    virtual void Restore() override;

    virtual void SetUseHwndSpesc(bool val) override;
    virtual bool GetUseHwndSpesc() override;

    virtual void SetHwndSpesc(long long hwnd) override;
    virtual long long GetHwndSpesc() override;

private:
    std::shared_ptr<WindowSceneItemHelper> window_item_helper_;
    std::unique_ptr<livehime::WindowCaptureSnapshot> snapshot_;
};

#endif // !BILILIVE_BILILIVE_LIVEHIME_SOURCE_WINDOW_PROPERTY_PRESENTER_H_
