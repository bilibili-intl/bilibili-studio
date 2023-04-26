#include "bililive/bililive/livehime/tabarea/global_banned_presenter_impl.h"

#include "base/ext/callable_callback.h"

#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/secret/bililive_secret.h"

GlobalBannedPresenterImpl::GlobalBannedPresenterImpl(
    contracts::GlobalBannedView* view) : weak_ptr_factory_(this) {
    view_ = view;
}

void GlobalBannedPresenterImpl::GetBannedInfo() {

}

void GlobalBannedPresenterImpl::OnBannedInfo(bool valid_response, int code, int second) {
    view_->OnBannedInfo(valid_response, code, second);
}

void GlobalBannedPresenterImpl::GlobalBanned(
    bool banned, int minute, const std::string& type, int level)
{

}

void GlobalBannedPresenterImpl::OnGlobalBannedOn(
    bool valid_response, int code, const std::string& error_msg)
{
    view_->OnGlobalBanned(true, (valid_response && code == 0));
}

void GlobalBannedPresenterImpl::OnGlobalBannedOff(
    bool valid_response, int code, const std::string& error_msg)
{
    view_->OnGlobalBanned(false, (valid_response && code == 0));
}