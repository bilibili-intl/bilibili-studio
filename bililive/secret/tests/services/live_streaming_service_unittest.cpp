/*
 @ 0xCCCCCCCC
*/

#include "base/ext/count_down_latch.h"

#include "bililive/secret/public/live_streaming_service.h"
#include "bililive/secret/tests/access_stub.h"

#include "gtest/gtest.h"

namespace {

using PKInviteResultInfo = secret::LiveStreamingService::PKInviteResultInfo;
using OncePKAnchorsInfo = secret::LiveStreamingService::OncePKAnchorsInfo;
using PKInviteInfo = secret::LiveStreamingService::PKInviteInfo;
using FollowEachInfo = secret::LiveStreamingService::FollowEachInfo;
using PKSearchInfo = secret::LiveStreamingService::PKSearchInfo;
using InviteConfigInfo = secret::LiveStreamingService::InviteConfigInfo;
using BeInvitedInfo = secret::LiveStreamingService::BeInvitedInfo;
using RoomInfo = secret::LiveStreamingService::RoomInfo;
using AreaDetailsInfo = secret::LiveStreamingService::AreaDetailsInfo;
using AreaListInfo = secret::LiveStreamingService::AreaListInfo;
using MyChooseAreaInfo = secret::LiveStreamingService::MyChooseAreaInfo;
using PKInfo = secret::LiveStreamingService::PKInfo;
using PKListRateInfo = secret::LiveStreamingService::PKListRateInfo;
using PKListInfo = secret::LiveStreamingService::PKListInfo;
using LiveRoomCdnLineInfo = secret::LiveStreamingService::LiveRoomCdnLineInfo;
using BlockedUserItem = secret::LiveStreamingService::BlockedUserItem;
using VoiceJoinGetConfigInfo = secret::LiveStreamingService::VoiceJoinGetConfigInfo;
using VoiceJoinSetConfigParams = secret::LiveStreamingService::VoiceJoinSetConfigParams;
using VoiceJoinSearchUserInfo = secret::LiveStreamingService::VoiceJoinSearchUserInfo;
using VoiceJoinApplyListInfo = secret::LiveStreamingService::VoiceJoinListInfo;
using VoiceJoinPickUserInfo = secret::LiveStreamingService::VoiceJoinPickUserInfo;
using VoiceJoinRoomCanInfo = secret::LiveStreamingService::VoiceJoinRoomCanInfo;
using LivemsgInfo = secret::LiveStreamingService::LivemsgInfo;

const int64_t kTestCallerId = 460780;
const int64_t kTestBlockUserId = 28007935;

int64_t invite_id = 0;
int64_t blacklist_id = 0;

}   // namespace
TEST(LiveStreamingServiceTest, VoiceJoinGetConfig)
{
    base::CountdownLatch latch(1);

    GetSecretHandle()->live_streaming_service()->VoiceJoinGetConfig(
        GetSecretHandle()->user_info().room_id(),
        [&latch](bool valid, int code,
            const std::string& err_msg,
            const VoiceJoinGetConfigInfo& info) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, VoiceJoinSetConfig)
{
    base::CountdownLatch latch(1);

    VoiceJoinSetConfigParams params;
    params.type = 0;
    params.room_id = 460737;
    params.guard = 3;
    params.medal_start = 1;
    params.user_list = "1,2,3";

    GetSecretHandle()->live_streaming_service()->VoiceJoinSetConfig(
        params,
        [&latch](bool valid, int code,
            const std::string& err_msg) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, VoiceJoinSearchUser)
{
    base::CountdownLatch latch(1);

    int64_t search_id = 460737;

    GetSecretHandle()->live_streaming_service()->VoiceJoinSearchUser(
        search_id,
        GetSecretHandle()->account_info().mid(),
        [&latch](bool valid, int code,
            const std::string& err_msg,
            const VoiceJoinSearchUserInfo& info) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, VoiceJoinApplyList)
{
    base::CountdownLatch latch(1);

    int type = 1;

    GetSecretHandle()->live_streaming_service()->VoiceJoinGetList(
        type,
        GetSecretHandle()->user_info().room_id(),
        GetSecretHandle()->account_info().mid(),
        [&latch](bool valid, int code,
            const std::string& err_msg,
            const VoiceJoinApplyListInfo& info) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, VoiceJoinReject)
{
    base::CountdownLatch latch(1);

    GetSecretHandle()->live_streaming_service()->VoiceJoinReject(
        1,
        GetSecretHandle()->account_info().mid(),
        GetSecretHandle()->user_info().room_id(),
        1,
        [&latch](bool valid, int code,
            const std::string& err_msg) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, VoiceJoinPickUser)
{
    base::CountdownLatch latch(1);

    int64_t select_id = 460737;

    GetSecretHandle()->live_streaming_service()->VoiceJoinPickUser(
        select_id,
        GetSecretHandle()->user_info().room_id(),
        [&latch](bool valid, int code,
            const std::string& err_msg,
            const VoiceJoinPickUserInfo& info) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, VoiceJoinRoomCan)
{
    base::CountdownLatch latch(1);

    GetSecretHandle()->live_streaming_service()->VoiceJoinRoomCan(
        GetSecretHandle()->user_info().room_id(),
        [&latch](bool valid, int code,
            const std::string& err_msg,
            const VoiceJoinRoomCanInfo& info) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, VoiceJoinRoomSwitch)
{
    base::CountdownLatch latch(1);

    int status = 1; //1´ò¿ª 2¹Ø±Õ

    GetSecretHandle()->live_streaming_service()->VoiceJoinRoomSwitch(
        GetSecretHandle()->user_info().room_id(), status,
        [&latch](bool valid, int code,
            const std::string& err_msg) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, VoiceJoinStop)
{
    base::CountdownLatch latch(1);

    std::string channel = "test_channel";

    GetSecretHandle()->live_streaming_service()->VoiceJoinStop(
        GetSecretHandle()->user_info().room_id(), channel,
        [&latch](bool valid, int code,
            const std::string& err_msg) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, GetLiveRoomCdnLineList)
{
    base::CountdownLatch latch(1);

    GetSecretHandle()->live_streaming_service()->GetLiveRoomCdnLineList(
        GetSecretHandle()->user_info().room_id(),
        [&latch](bool valid, int code,
        const std::string& rtmp_addr,
        const std::string& rtmp_key,
        const std::vector<LiveRoomCdnLineInfo>& list) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        LOG(INFO) << "rtmp addr: " << rtmp_addr;
        LOG(INFO) << "rtmp code: " << rtmp_key;
        LOG(INFO) << "cdn line count: " << list.size();
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, BlockUser)
{
    base::CountdownLatch latch(1);

    std::string content = std::to_string(kTestBlockUserId);
    int type = 1;
    int hour = 1;

    GetSecretHandle()->live_streaming_service()->BlockUser(
        GetSecretHandle()->user_info().room_id(), content, type, hour,
        [&latch](bool valid, int code) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, GetBlockedUserList)
{
    base::CountdownLatch latch(1);

    int page = 1;

    GetSecretHandle()->live_streaming_service()->GetBlockedUserList(
        GetSecretHandle()->user_info().room_id(), page,
        [&latch](bool valid, int code, const std::vector<BlockedUserItem>& list) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        if (list.size() > 0) {
            auto iter = --list.end();
            blacklist_id = iter->id;
        }
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, UnblockUser)
{
    base::CountdownLatch latch(1);

    GetSecretHandle()->live_streaming_service()->UnblockUser(
        GetSecretHandle()->user_info().room_id(), blacklist_id,
        [&latch](bool valid, int code) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, PKEntrance)
{
    base::CountdownLatch latch(1);

    GetSecretHandle()->live_streaming_service()->PKEntrance(
        GetSecretHandle()->user_info().room_id(),
        [&latch](bool valid, int code, const std::string& error_msg, bool pk_is_open) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        LOG(INFO) << "pk entrance is " << (pk_is_open?"open":"closed");
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, PKStart)
{
    base::CountdownLatch latch(1);

    GetSecretHandle()->live_streaming_service()->PKStart(
        GetSecretHandle()->user_info().room_id(),
        [&latch](bool valid, int code, const std::string& error_msg) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        if (code != 0) {
            LOG(INFO) << "can not start pk, error code is " << code;
        }
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, GetPKList)
{
    base::CountdownLatch latch(1);

    GetSecretHandle()->live_streaming_service()->GetPKList(
        GetSecretHandle()->user_info().room_id(),
        [&latch](bool valid,
        int code,
        const std::string& error_msg,
        const PKListRateInfo& rate_info,
        const std::vector<PKListInfo>& info) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        LOG(INFO) << "win rate is " << rate_info.win_rate;
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, PKInvite)
{
    base::CountdownLatch latch(1);

    GetSecretHandle()->live_streaming_service()->PKInvite(
        GetSecretHandle()->user_info().room_id(), kTestCallerId,
        [&latch](bool valid, int code, const std::string& error_msg, const PKInviteResultInfo& info) {
            EXPECT_TRUE(valid);
            EXPECT_TRUE(code == 0);
            invite_id = info.invite_id;
            latch.Countdown();
        }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, GetOncePKAnchorsList)
{
    base::CountdownLatch latch(1);

    GetSecretHandle()->live_streaming_service()->GetOncePKAnchorsList(
        GetSecretHandle()->user_info().room_id(),
        [&latch](bool valid, int code, const std::string& error_msg,
        int live_count,
        const std::vector<OncePKAnchorsInfo>& list) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        LOG(INFO) << "once pk anchors list size : " << list.size();
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, InviteConditionNoAttention)
{
    base::CountdownLatch latch(1);

    int level = 0;
    int attention = 0;

    GetSecretHandle()->live_streaming_service()->SetInviteCondition(
        GetSecretHandle()->user_info().room_id(), level, attention,
        [&latch](bool valid, int code, const std::string& error_msg) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, InviteConditionAttention)
{
    base::CountdownLatch latch(1);

    int level = 0;
    int attention = 1;

    GetSecretHandle()->live_streaming_service()->SetInviteCondition(
        GetSecretHandle()->user_info().room_id(), level, attention,
        [&latch](bool valid, int code, const std::string& error_msg) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, InviteConditionAttentioned)
{
    base::CountdownLatch latch(1);

    int level = 0;
    int attention = 2;

    GetSecretHandle()->live_streaming_service()->SetInviteCondition(
        GetSecretHandle()->user_info().room_id(), level, attention,
        [&latch](bool valid, int code, const std::string& error_msg) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, InviteSwitchOn)
{
    base::CountdownLatch latch(1);

    std::string type = "on";

    GetSecretHandle()->live_streaming_service()->SetInviteSwitch(
        GetSecretHandle()->user_info().room_id(), type,
        [&latch](bool valid, int code, const std::string& error_msg) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, InviteSwitchOff)
{
    base::CountdownLatch latch(1);

    std::string type = "off";

    GetSecretHandle()->live_streaming_service()->SetInviteSwitch(
        GetSecretHandle()->user_info().room_id(), type,
        [&latch](bool valid, int code, const std::string& error_msg) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, GetInviteConfig)
{
    base::CountdownLatch latch(1);

    GetSecretHandle()->live_streaming_service()->GetInviteConfig(
        GetSecretHandle()->user_info().room_id(),
        [&latch](bool valid, int code, const std::string& error_msg, const InviteConfigInfo& info) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, AddBlackList)
{
    base::CountdownLatch latch(1);

    int64_t test_id = 460491;

    GetSecretHandle()->live_streaming_service()->AddPKBlackList(
        test_id,
        [&latch](bool valid, int code, const std::string& error_msg) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, GetPKInviteInfo)
{
    base::CountdownLatch latch(1);

    GetSecretHandle()->live_streaming_service()->GetPKInviteInfo(
        invite_id,
        [&latch](bool valid, int code, const std::string& error_msg,
        const PKInviteInfo& info) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, GetBeInvitedList)
{
    base::CountdownLatch latch(1);

    GetSecretHandle()->live_streaming_service()->GetBeInvitedList(
        GetSecretHandle()->user_info().room_id(),
        [&latch](bool valid, int code, const std::string& error_msg,
        const std::vector<BeInvitedInfo>& list) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}


TEST(LiveStreamingServiceTest, GetFollowEachList)
{
    base::CountdownLatch latch(1);

    GetSecretHandle()->live_streaming_service()->GetFollowEachList(
        [&latch](bool valid, int code, const std::string& error_msg, int total_count, int live_count,
        const std::vector<FollowEachInfo>& list) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        LOG(INFO) << "total count : " << total_count;
        LOG(INFO) << "live_count : " << live_count;
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, PKSearch)
{
    base::CountdownLatch latch(1);

    std::string keyword = "1";
    std::string search_type = "live_user";
    int page = 0;
    int page_size = 10;

    GetSecretHandle()->live_streaming_service()->PKSearch(
        keyword, search_type, page, page_size,
        [&latch](bool valid, int code, const std::string& error_msg,
        const std::vector<PKSearchInfo>& list) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        LOG(INFO) << "total count of this page : " << list.size();
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}


TEST(LiveStreamingServiceTest, GetUserSan)
{
    base::CountdownLatch latch(1);

    GetSecretHandle()->live_streaming_service()->GetUserSAN(
        GetSecretHandle()->account_info().mid(),
        [&latch](bool succeeded, int64_t san) {
        EXPECT_TRUE(succeeded);
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, GetAreaList)
{
    base::CountdownLatch latch(1);

    GetSecretHandle()->live_streaming_service()->GetAreaList(
        [&latch](bool succeeded,
        const std::vector<AreaListInfo>& area_list) {
        EXPECT_TRUE(succeeded);
        LOG(INFO) << "total count of this area : " << area_list.size();
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, GetMyChooseArea)
{
    base::CountdownLatch latch(1);

    GetSecretHandle()->live_streaming_service()->GetMyChooseArea(
        GetSecretHandle()->user_info().room_id(), [&latch](bool succeeded,
        const std::vector<MyChooseAreaInfo>& info) {
        EXPECT_TRUE(succeeded);
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

//TEST(LiveStreamingServiceTest, OpenLiveRoom)
//{
//    base::CountdownLatch latch(1);
//
//    int channel_id = 21;
//    int src = -1;
//    int checked = 1;
//
//
//    GetSecretHandle()->live_streaming_service()->OpenLiveRoom(
//        GetSecretHandle()->user_info().room_id(), channel_id, src, checked, [&latch](
//        bool valid,
//        int code,
//        const std::string& err_msg,
//        const std::string& key,
//        const std::string& addr,
//        const std::string& new_link) {
//        EXPECT_TRUE(valid);
//        EXPECT_TRUE(code == 0);
//        LOG(INFO) << "address : " << addr;
//        LOG(INFO) << "new_link : " << new_link;
//        latch.Countdown();
//    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();
//
//    latch.Wait();
//}

TEST(LiveStreamingServiceTest, CloseLiveRoom)
{
    base::CountdownLatch latch(1);

    GetSecretHandle()->live_streaming_service()->StopLive(
        GetSecretHandle()->user_info().room_id(), [&latch](bool valid, int code, const std::string& error_msg) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, UpdateRoomInfo)
{
    base::CountdownLatch latch(1);

    std::string title = "test";
    int channel_id = 21;

    GetSecretHandle()->live_streaming_service()->UpdateRoomInfo(
        GetSecretHandle()->user_info().room_id(), title, channel_id, [&latch](bool valid, int code, const std::string& error_msg) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, NoticeList)
{
    base::CountdownLatch latch(1);

    std::string product = "live";
    int belle = 1;
    std::string category = "all";
    int page_no = 1;
    int page_size = 5;

    GetSecretHandle()->live_streaming_service()->NoticeList(
        product, belle, category, page_no, page_size, [&latch](
            bool valid,
            int code,
            const std::string& error_msg,
            const std::vector<secret::LiveStreamingService::NoticeInfo>& list) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        LOG(INFO) << "notice count : " << list.size();
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, NewBannerList)
{
    base::CountdownLatch latch(1);

    GetSecretHandle()->live_streaming_service()->NewBannerList(
        [&latch](
            bool valid,
            int code,
            const std::string& error_msg,
            const std::vector<secret::LiveStreamingService::NewBannerInfo>& list) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        LOG(INFO) << "notice count : " << list.size();
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, LivemsgUnreadCount)
{
    base::CountdownLatch latch(1);

    GetSecretHandle()->live_streaming_service()->LivemsgUnreadCount([&latch](
        bool valid,
        int code,
        const std::string& err_msg,
        int unread_count) {
        EXPECT_TRUE(valid);
        LOG(INFO) << "unread_count : " << unread_count;
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(LiveStreamingServiceTest, LivemsgList)
{
    base::CountdownLatch latch(1);

    int page_size = 20;
    std::string cursor = "";

    GetSecretHandle()->live_streaming_service()->LivemsgList(
        page_size, cursor, [&latch](
            bool valid,
            int code,
            const std::string& err_msg,
            int has_more,
            const std::vector<LivemsgInfo>& list) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        LOG(INFO) << "msg_count : " << list.size();
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}