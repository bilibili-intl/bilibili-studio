#include "source_window_property_presenter.h"
#include "obs/obs_proxy/public/proxy/obs_sources_wrapper.h"
#include "bililive/bililive/livehime/obs/srcprop_ui_util.h"

SourceWindowPropertyPresenterImp::SourceWindowPropertyPresenterImp(obs_proxy::SceneItem* scene_item) 
	:SourceWindowPropertyPresenter(scene_item),
	window_item_helper_(std::make_shared<WindowSceneItemHelper>(scene_item)){

}

SourceWindowPropertyPresenterImp::~SourceWindowPropertyPresenterImp() {

}

PropertyList<std::string> SourceWindowPropertyPresenterImp::WindowList() {
	return window_item_helper_->WindowList();
}

PropertyList<std::string> SourceWindowPropertyPresenterImp::ModelList() {
	return window_item_helper_->ModelList();
}

std::string SourceWindowPropertyPresenterImp::SelectedWindow() {
	return window_item_helper_->SelectedWindow();
}

void SourceWindowPropertyPresenterImp::SelectedWindow(std::string val) {
	window_item_helper_->SelectedWindow(val);
}

bool SourceWindowPropertyPresenterImp::IsCaptureCursor() {
	return window_item_helper_->IsCaptureCursor();
}

void SourceWindowPropertyPresenterImp::IsCaptureCursor(bool val) {
	window_item_helper_->IsCaptureCursor(val);
}

bool SourceWindowPropertyPresenterImp::IsCompatibleMode() {
	return window_item_helper_->IsCompatibleMode();
}

void SourceWindowPropertyPresenterImp::IsCompatibleMode(bool val) {
	window_item_helper_->IsCompatibleMode(val);
}

bool SourceWindowPropertyPresenterImp::IsSliMode() {
	return window_item_helper_->IsSliMode();
}

void SourceWindowPropertyPresenterImp::IsSliMode(bool val) {
	window_item_helper_->IsSliMode(val);
}

void SourceWindowPropertyPresenterImp::SetCaptureModel(int model) {
	window_item_helper_->SetCaptureModel(model);
}

int SourceWindowPropertyPresenterImp::GetCaptureModel() {
	return window_item_helper_->GetCaptureModel();
}

void SourceWindowPropertyPresenterImp::Snapshot() {
	snapshot_ = livehime::WindowCaptureSnapshot::NewTake(window_item_helper_.get());
}

void SourceWindowPropertyPresenterImp::Restore() {
	snapshot_->Restore();
}

void SourceWindowPropertyPresenterImp::SetUseHwndSpesc(bool val)
{
	window_item_helper_->SetUseHwndSpesc(val);
}

bool SourceWindowPropertyPresenterImp::GetUseHwndSpesc()
{
	return window_item_helper_->GetHwndSpesc();
}

void SourceWindowPropertyPresenterImp::SetHwndSpesc(long long hwnd)
{
	window_item_helper_->SetHwndSpesc(hwnd);
}

long long SourceWindowPropertyPresenterImp::GetHwndSpesc()
{
	return window_item_helper_->GetHwndSpesc();
}
