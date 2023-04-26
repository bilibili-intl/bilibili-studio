/*
 @ 0xCCCCCCCC
*/

#include "bililive/secret/tests/test_runner.h"

#include "gtest/gtest.h"

#include "base/bind.h"
#include "base/ext/count_down_latch.h"
#include "base/files/file_path.h"
#include "base/path_service.h"
#include "base/run_loop.h"

#include "bililive/common/bililive_logging.h"
#include "bililive/secret/tests/access_stub.h"

namespace {

using BililiveSecretCreator = BililiveSecret* (*)();

const wchar_t kBililiveSecretDLL[] = L"bililive_secret.dll";

HMODULE LoadSecretDLL(base::FilePath& secret_dll_path)
{
    base::FilePath main_dir;
    PathService::Get(base::DIR_EXE, &main_dir);
    CHECK(!main_dir.empty()) << "Failed to obtain main directory!";
    auto dll_path = main_dir.Append(kBililiveSecretDLL);
    secret_dll_path = dll_path;
    return LoadLibraryExW(dll_path.value().c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
}

BililiveSecretCreator GetBililiveSecretCreator()
{
    base::FilePath dll_path;
    HMODULE secret_dll = LoadSecretDLL(dll_path);
    if (!secret_dll) {
        PLOG(ERROR) << "Failed to load secret dll from " << dll_path.AsUTF8Unsafe();
        return nullptr;
    }

    BililiveSecretCreator creator_func =
        reinterpret_cast<BililiveSecretCreator>(GetProcAddress(secret_dll, "CreateBililiveSecret"));

    if (!creator_func) {
        PLOG(ERROR) << "No entry point in secret dll found; possibly defected!";
        return nullptr;
    }

    return creator_func;
}

}   // namespace

TestRunner::TestRunner()
{}

void TestRunner::Setup(int argc, char* argv[])
{
    bililive::InitBililiveLogging();

    testing::InitGoogleTest(&argc, argv);

    process_ = std::make_unique<FakeBililiveProcess>();
    std::cout << "FakeBililiveProcess has been created!\n";

    auto secret_creator = GetBililiveSecretCreator();
    secret_ = secret_creator();
    CHECK(secret_) << "Create secret core failed!";

    base::FilePath global_data_dir;
    PathService::Get(base::DIR_LOCAL_APP_DATA, &global_data_dir);
    CHECK(!global_data_dir.empty());
    global_data_dir = global_data_dir.AppendASCII("bililive").AppendASCII("User Data").AppendASCII("Global");

    secret_->Init(loop_.message_loop_proxy(), global_data_dir, process_.get());

    SetSecretHandle(secret_);

    std::cout << "Secret module has been initialized!\n";

    InitAccountInfo();

    InitRoomId();
}

void TestRunner::InitAccountInfo()
{
    auto info = secret_->GetLastValidLoginInfo();

    secret_->SaveAccountInfo(
        false,
        info->mid, info->name,
        info->token,
        info->refresh_token,
        info->expires,
        info->cookies,
        info->domains,
        true);

    std::cout << "AccountInfo has been initialized!\n";
}

void TestRunner::InitRoomId()
{
    base::CountdownLatch latch(1);
    int64_t room_id = 0;

    GetSecretHandle()->live_streaming_service()->GetRoomInfo(
        GetSecretHandle()->account_info().mid(),
        [&latch, &room_id](bool valid, int code, const std::string& error_msg,
        const secret::LiveStreamingService::RoomInfo& info) {
        if (valid && code == 0) {
            room_id = info.room_id;
            std::cout << "RoomId has been initialized!\n";
        }
        latch.Countdown();
    }).ReplyOn(secret::ReplyScheduler::TaskThread).Call();

    latch.Wait();

    GetSecretHandle()->user_info().set_room_id(room_id);
}

void TestRunner::Teardown()
{
    ClearSecretHandle();

    secret_->UnInit();

    std::cout << "Secret module has been uninitialized!\n";
}

void TestRunner::Run()
{
    base::RunLoop run_loop;

    loop_.PostTask(FROM_HERE, base::Bind(&TestRunner::RunTests));
    //loop_.PostTask(FROM_HERE, run_loop.QuitClosure());

    run_loop.Run();
}

// static
void TestRunner::RunTests()
{
    RUN_ALL_TESTS();
}
