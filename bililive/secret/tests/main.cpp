/*
 @ 0xCCCCCCCC
*/

#include "base/at_exit.h"
#include "base/command_line.h"

#include "bililive/secret/tests/test_runner.h"

int main(int argc, char* argv[])
{
    base::AtExitManager exit_manager;
    CommandLine::Init(0, nullptr);

    TestRunner runner;
    runner.Setup(argc, argv);
    runner.Run();
    runner.Teardown();
}