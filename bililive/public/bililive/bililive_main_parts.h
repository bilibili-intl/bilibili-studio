#ifndef BILILIVE_PUBLIC_BILILIVE_BILILIVE_MAIN_PARTS_H_
#define BILILIVE_PUBLIC_BILILIVE_BILILIVE_MAIN_PARTS_H_

class BililiveMainExtraParts;

class BililiveMainParts {
public:
    virtual void AddParts(BililiveMainExtraParts* parts) = 0;

    BililiveMainParts() {}

    virtual ~BililiveMainParts() {}

    virtual void PreEarlyInitialization() {}

    virtual void PostEarlyInitialization() {}

    virtual void PreMainMessageLoopStart() {}

    virtual void PostMainMessageLoopStart() {}

    virtual void ToolkitInitialized() {}

    virtual int PreCreateThreads()
    {
        return 0;
    }

    virtual void PreMainMessageLoopRun() {}

    virtual bool MainMessageLoopRun(int* result_code)
    {
        return false;
    }

    virtual void PostMainMessageLoopRun() {}

    virtual void PostDestroyThreads() {}
};

#endif  // BILILIVE_PUBLIC_BILILIVE_BILILIVE_MAIN_PARTS_H_