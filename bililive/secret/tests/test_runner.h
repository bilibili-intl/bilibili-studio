/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_SECRET_TESTS_TEST_RUNNER_H_
#define BILILIVE_SECRET_TESTS_TEST_RUNNER_H_

#include <memory>

#include "base/basictypes.h"
#include "base/message_loop/message_loop.h"

#include "bililive/public/secret/bililive_secret.h"
#include "bililive/secret/tests/fake_bililive_process.h"

class TestRunner {
public:
    TestRunner();

    ~TestRunner() = default;

    void Setup(int argc, char* argv[]);

    void Teardown();

    void Run();

    void InitAccountInfo();

    void InitRoomId();

private:
    static void RunTests();

    DISALLOW_COPY_AND_ASSIGN(TestRunner);

private:
    base::MessageLoop loop_;
    std::unique_ptr<FakeBililiveProcess> process_;
    scoped_refptr<BililiveSecret> secret_;
};

#endif  // BILILIVE_SECRET_TESTS_TEST_RUNNER_H_
