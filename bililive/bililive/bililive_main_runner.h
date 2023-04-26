#ifndef BILILIVE_BILILIVE_BILILIVE_MAIN_RUNNER_H_
#define BILILIVE_BILILIVE_BILILIVE_MAIN_RUNNER_H_

class CommandLine;

class BililiveMainRunner {
public:
    virtual ~BililiveMainRunner() {}

    static BililiveMainRunner* Create();

    virtual int Initialize(const CommandLine& cmd_line) = 0;

    virtual int Run() = 0;

    virtual void Shutdown() = 0;
};

#endif  // BILILIVE_BILILIVE_BILILIVE_MAIN_RUNNER_H_