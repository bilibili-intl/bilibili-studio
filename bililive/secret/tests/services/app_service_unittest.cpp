/*
 @ 0xCCCCCCCC
*/

#include "base/ext/count_down_latch.h"

#include "bililive/secret/public/app_service.h"
#include "bililive/secret/tests/access_stub.h"

#include "gtest/gtest.h"

namespace {

using ReleaseUpdateInfo = secret::AppService::ReleaseUpdateInfo;
using UpdateResult = secret::AppService::UpdateResult;
using BetaUpdateInfo = secret::AppService::BetaUpdateInfo;
using UpgradeInfo = secret::AppService::UpgradeInfo;
using UploadInfo = secret::AppService::UploadInfo;

}   // namespace

TEST(AppServiceTest, UpdateAvailable)
{
    base::CountdownLatch latch(1);

    GetSecretHandle()->app_service()->CheckForReleaseUpdate(
        [&latch](UpdateResult result, const ReleaseUpdateInfo& update_info) {
        if (UpdateResult::NetworkError == result) {
            EXPECT_TRUE(false);
        } else if (UpdateResult::AlreadyUpToDate == result) {
            EXPECT_TRUE(true);
            LOG(INFO) << "the current version is ths latest";
        } else {
            EXPECT_TRUE(update_info.file_size > 0);
            LOG(INFO) << "Installer build number: " << update_info.version_code;
            EXPECT_TRUE(!update_info.update_url.empty());
            LOG(INFO) << "Installer url: " << update_info.update_url;
            EXPECT_TRUE(!update_info.file_md5.empty());
        }
            latch.Countdown();
        }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(AppServiceTest, GetDownloadUrl)
{
    base::CountdownLatch latch(1);

    int type = 3;

    GetSecretHandle()->app_service()->CheckForBetaUpdate(
        [&latch](UpdateResult result, const BetaUpdateInfo& download_info) {
        LOG(INFO) << "version:" << download_info.version;
        LOG(INFO) << "inner_ver_num:" << download_info.inner_ver_num;
        EXPECT_TRUE(!download_info.dl_url.empty());
        LOG(INFO) << "ver_size:" << download_info.ver_size;
        LOG(INFO) << "ver_md5:" << download_info.ver_md5;
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(AppServiceTest, ForceUpgrade)
{
    base::CountdownLatch latch(1);
    int build_no = 1;

    GetSecretHandle()->app_service()->UpgradeCheck(
        [&latch](bool valid, int code, const UpgradeInfo& upgrade_info) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        LOG(INFO) << "force_upgrade:" << upgrade_info.force_upgrade;
        LOG(INFO) << "version:" << upgrade_info.version;
        LOG(INFO) << "inner_ver_num:" << upgrade_info.inner_ver_num;
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(AppServiceTest, UnForceUpgrade)
{
    base::CountdownLatch latch(1);
    int build_no = 9999;

    GetSecretHandle()->app_service()->UpgradeCheck(
        [&latch](bool valid, int code, const UpgradeInfo& upgrade_info) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        LOG(INFO) << "force_upgrade:" << upgrade_info.force_upgrade;
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}

TEST(AppServiceTest, UploadDumpFile)
{
    base::CountdownLatch latch(1);
    std::string content = "dumpfile content for test";

    GetSecretHandle()->app_service()->UploadDumpFile(
        content,
        [&latch](bool valid, int code, const UploadInfo& upload_info) {
        EXPECT_TRUE(valid);
        EXPECT_TRUE(code == 0);
        EXPECT_TRUE(!upload_info.url.empty());
        LOG(INFO) << "Update url:" << upload_info.url;
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();
}