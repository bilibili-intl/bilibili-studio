#include "bililive/bililive/livehime/tabarea/tab_pane_danmaku_presenter_impl.h"

#include "base/ext/callable_callback.h"
#include "base/memory/scoped_vector.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"

#include "bililive/bililive/bililive_database.h"
#include "bililive/bililive/ui/views/controls/bililive_theme_common.h"
#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/bililive/bililive_thread.h"
#include "bililive/public/secret/bililive_secret.h"

#include "ui/base/resource/resource_bundle.h"
#include "grit/generated_resources.h"

namespace
{
const SkColor clrLocal = SkColorSetRGB(0xFF, 0x98, 0x38);
const SkColor clrManager = SkColorSetRGB(0xFF, 0x98, 0x38);
const SkColor clrSVip = SkColorSetRGB(0xFF, 0xB1, 0x00);
const SkColor clrVip = SkColorSetRGB(0xFB, 0x72, 0x99);
const SkColor clrGift = SkColorSetRGB(0x0E, 0xBE, 0xFF);
const SkColor clrGovernor = SkColorSetRGB(0xD7, 0x49, 0xFF);
const SkColor clrAdmiral = SkColorSetRGB(0xE9, 0x1E, 0x63);
const SkColor clrCaptain = SkColorSetRGB(0x5A, 0x92, 0xF2);
const SkColor clrNormal = SkColorSetRGB(0x86, 0x96, 0xA3);
const SkColor clrSTime = SkColorSetRGB(0x86, 0x96, 0xA3);
const SkColor clrText = SkColorSetRGB(0x2C, 0x44, 0x57);
const int nDanmakusPerAdd = 50;
const int nDanmakusMaxCount = 500;
}

TabPaneDanmakuPresenterImpl::TabPaneDanmakuPresenterImpl(
    contracts::TabPaneDanmakuView* view) : weak_ptr_factory_(this),
    danmaku_begin_(0),
    danmaku_end_(0),
    total_(0) {
    view_ = view;
}

void TabPaneDanmakuPresenterImpl::InitDanmaku() {
    BililiveThread::PostTask(BililiveThread::DB, FROM_HERE,
        base::Bind(&TabPaneDanmakuPresenterImpl::InitDanmakuFromDB,
        weak_ptr_factory_.GetWeakPtr()));
}

void TabPaneDanmakuPresenterImpl::InitDanmakuFromDB() {
    BililiveDatabase *db = GetBililiveProcess()->database();
    if (db) {
        total_ = db->GetDanmakuCount();

        ScopedVector<DanmakuInfo> vct;
        std::vector<contracts::BarrageDetails> barrage_add;

        danmaku_end_ = total_;
        if (total_ <= nDanmakusPerAdd) {
            danmaku_begin_ = 0;
        } else {
            danmaku_begin_ = danmaku_end_ - nDanmakusPerAdd;
        }

        db->GetDanmaku(danmaku_begin_, danmaku_end_ - danmaku_begin_, vct);

        uint32 vs = vct.size();
        for (uint32 i = 0; i < vs; i++) {
            barrage_add.push_back(AppendDanmaku(*(vct)[i]));
        }

        barrage_list_.swap(barrage_add);

        view_->OnInitDanmaku(barrage_list_);
    }
}

void TabPaneDanmakuPresenterImpl::LoadDanmaku(contracts::UpdateDataType type) {
    BililiveThread::PostTask(BililiveThread::DB, FROM_HERE,
        base::Bind(&TabPaneDanmakuPresenterImpl::LoadDanmakuFromDB,
        weak_ptr_factory_.GetWeakPtr(), type));
}

void TabPaneDanmakuPresenterImpl::LoadDanmakuFromDB(contracts::UpdateDataType type) {
    BililiveDatabase *db = GetBililiveProcess()->database();
    if (db) {
        ScopedVector<DanmakuInfo> vct;
        std::vector<contracts::BarrageDetails> barrage_add;
        int count = 0;

        if (type == contracts::UpdateDataType::TOTOP) {
            if (danmaku_begin_ <= 0) {
                view_->OnLoadStoped();
                return;
            }

            if (danmaku_begin_ >= nDanmakusPerAdd) {
                count = nDanmakusPerAdd;
                danmaku_begin_ = danmaku_begin_ - nDanmakusPerAdd;
            } else {
                count = danmaku_begin_;
                danmaku_begin_ = 0;
            }

            if ((danmaku_end_ - danmaku_begin_) > nDanmakusMaxCount) {
                danmaku_end_ = danmaku_end_ - nDanmakusPerAdd;
            }
        } else {
            if (danmaku_end_ >= total_) {
                view_->OnLoadStoped();
                return;
            }

            if ((danmaku_end_ + nDanmakusPerAdd) >= (total_)) {
                danmaku_end_ = total_;
            } else {
                danmaku_end_ = danmaku_end_ + nDanmakusPerAdd;
            }

            if ((danmaku_end_ - danmaku_begin_) > nDanmakusMaxCount) {
                danmaku_begin_ = danmaku_begin_ + nDanmakusPerAdd;
            }
        }

        db->GetDanmaku(danmaku_begin_, danmaku_end_ - danmaku_begin_, vct);

        uint32 vs = vct.size();
        for (uint32 i = 0; i < vs; i++) {
            barrage_add.push_back(AppendDanmaku(*(vct)[i]));
        }
        barrage_list_.swap(barrage_add);

        view_->OnLoadDanmaku(barrage_list_, count, type);
    } else {
        view_->OnLoadStoped();
    }

}

void TabPaneDanmakuPresenterImpl::LocateDanmaku(int id) {
    BililiveThread::PostTask(BililiveThread::DB, FROM_HERE,
        base::Bind(&TabPaneDanmakuPresenterImpl::LocateDanmakuFromDB,
        weak_ptr_factory_.GetWeakPtr(), id));
}

void TabPaneDanmakuPresenterImpl::LocateDanmakuFromDB(int id) {
    BililiveDatabase *db = GetBililiveProcess()->database();
    if (db) {
        total_ = db->GetDanmakuCount();

        ScopedVector<DanmakuInfo> vct;
        std::vector<contracts::BarrageDetails> barrage_locate;

        danmaku_begin_ = id - 1;
        danmaku_end_ = danmaku_begin_ + nDanmakusPerAdd;

        if (danmaku_end_ >= total_) {
            danmaku_end_ = total_;
        }

        db->GetDanmaku(danmaku_begin_, danmaku_end_ - danmaku_begin_, vct);

        uint32 vs = vct.size();
        for (uint32 i = 0; i < vs; i++) {
            barrage_locate.push_back(AppendDanmaku(*(vct)[i]));
        }

        barrage_list_.swap(barrage_locate);

        view_->OnLocateDanmaku(barrage_list_);
    }
}

contracts::BarrageDetails TabPaneDanmakuPresenterImpl::AppendDanmaku(const DanmakuInfo &danmaku) {
    auto &rb = ResourceBundle::GetSharedInstance();
    auto uid = GetBililiveProcess()->secret_core()->account_info().mid();
    base::string16 myself;
    base::string16 manager;
    base::string16 svip;
    base::string16 vip;
    base::string16 gift;
    base::string16 governor;
    base::string16 admiral;
    base::string16 captain;
    if (myself.empty()) {
        myself = rb.GetLocalizedString(IDS_DANMAKU_HISTORY_MYSELF);
        manager = rb.GetLocalizedString(IDS_DANMAKU_HISTORY_MANAGE);
        svip = rb.GetLocalizedString(IDS_DANMAKU_HISTORY_SVIP);
        vip = rb.GetLocalizedString(IDS_DANMAKU_HISTORY_VIP);
        gift = rb.GetLocalizedString(IDS_DANMAKU_HISTORY_GIFT);
        governor = rb.GetLocalizedString(IDS_DANMAKU_GOVERNOR);
        admiral = rb.GetLocalizedString(IDS_DANMAKU_ADMIRAL);
        captain = rb.GetLocalizedString(IDS_DANMAKU_CAPTAIN);
    }

    SkColor crTitleColor = clrNormal;
    base::string16 title;
    base::string16 pre;
    base::string16 stime;
    base::string16 content;

    switch (danmaku.dt)
    {
    case DT_DANMU_MSG:
    {
        if (danmaku.uid == uid) {
            pre = myself;
            crTitleColor = clrLocal;
        } else {
            if (danmaku.manager) {
                pre = manager;
                crTitleColor = clrManager;
            }

            if (danmaku.vip) {
                pre = vip;
                crTitleColor = clrVip;
            }

            if (danmaku.svip) {
                pre = svip;
                crTitleColor = clrSVip;
            }

            if (danmaku.barrage) {
                if (BarrageType::BARRAGE_GOVERNOR == danmaku.barrage) {
                    pre = governor;
                    crTitleColor = clrGovernor;
                } else if (BarrageType::BARRAGE_ADMIRAL == danmaku.barrage) {
                    pre = admiral;
                    crTitleColor = clrAdmiral;
                } else if (BarrageType::BARRAGE_CAPTAIN == danmaku.barrage) {
                    pre = captain;
                    crTitleColor = clrCaptain;
                }
            }
        }

        title.append(pre).append(L" ").append(base::UTF8ToUTF16(danmaku.uname));
    }
    break;
    case DT_SEND_GIFT:
    {
        title = gift;
        crTitleColor = clrGift;
    }
    break;
    default:
    break;
    }

    content.append(L" ").append(base::UTF8ToUTF16(danmaku.danmaku));

    base::Time::Exploded time_exploded;
    base::Time::FromTimeT(danmaku.stime).LocalExplode(&time_exploded);
    auto time = base::StringPrintf("%d/%02d/%02d %02d:%02d:%02d",
        time_exploded.year,
        time_exploded.month,
        time_exploded.day_of_month,
        time_exploded.hour,
        time_exploded.minute,
        time_exploded.second);

    contracts::BarrageDetails barrage_details;

    barrage_details.crTitleColor = crTitleColor;
    barrage_details.title = title;
    barrage_details.stime = base::UTF8ToUTF16(time);
    barrage_details.crContentColor = clrText;
    barrage_details.crSTime = clrSTime;
    barrage_details.content = content;
    barrage_details.uid = danmaku.uid;
    barrage_details.uname = danmaku.uname;
    barrage_details.danmaku = danmaku.danmaku;
    barrage_details.id = danmaku.id;
    barrage_details.ts = danmaku.ts;
    barrage_details.ct = danmaku.ct;

    return barrage_details;
}

void TabPaneDanmakuPresenterImpl::EventTracking() {
    GetBililiveProcess()->secret_core()->event_tracking_service()->
        ReportLivehimeBehaviorEvent(
        secret::LivehimeBehaviorEvent::LivehimeDanmaku,
        GetBililiveProcess()->secret_core()->account_info().mid(),
        std::string()).Call();
}

void TabPaneDanmakuPresenterImpl::ReportDanmaku(const SelectedDanmakuDetails& details)
{

}

void TabPaneDanmakuPresenterImpl::OnReportDanmaku(
    bool valid_response, int code, const std::string& error_msg)
{
    view_->OnReportDanmaku(valid_response && code == 0);
}

void TabPaneDanmakuPresenterImpl::SearchDanmaku(const std::string& content, int64 time_range) {
    BililiveThread::PostTask(BililiveThread::DB, FROM_HERE,
        base::Bind(&TabPaneDanmakuPresenterImpl::SearchDanmakuFromDB,
        weak_ptr_factory_.GetWeakPtr(), content, time_range));
}

void TabPaneDanmakuPresenterImpl::SearchDanmakuFromDB(const std::string& content, int64 time_range) {
    BililiveDatabase *db = GetBililiveProcess()->database();
    if (db) {
        ScopedVector<DanmakuInfo> vct;
        std::vector<contracts::BarrageDetails> barrage_search;

        db->SearchDanmaku(content, time_range, nDanmakusMaxCount, vct);

        uint32 vs = vct.size();
        for (uint32 i = 0; i < vs; i++) {
            barrage_search.push_back(AppendDanmaku(*(vct)[i]));
        }

        barrage_list_.swap(barrage_search);
    } else {
        barrage_list_.clear();
    }

    view_->OnSearchDanmaku(barrage_list_);
}

void TabPaneDanmakuPresenterImpl::RoomAdminAdd(int64_t id) {

}

void TabPaneDanmakuPresenterImpl::OnRoomAdminAdd(
    bool valid_response, int code, const std::string& error_msg) {
    view_->OnRoomAdminAdd(valid_response, code, error_msg);
}