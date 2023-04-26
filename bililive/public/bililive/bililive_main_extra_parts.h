#ifndef BILILIVE_PUBLIC_BILILIVE_BILILIVE_MAIN_EXTRA_PARTS_H_
#define BILILIVE_PUBLIC_BILILIVE_BILILIVE_MAIN_EXTRA_PARTS_H_

class BililiveMainExtraParts {
public:
    virtual ~BililiveMainExtraParts() {}

    virtual void PreEarlyInitialization() {}

    virtual void PostEarlyInitialization() {}

    virtual void ToolkitInitialized() {}

    virtual void PreMainMessageLoopStart() {}

    virtual void PostMainMessageLoopStart() {}

    virtual void PreCreateThreads() {}

    virtual void PreMainMessageLoopRun() {}

    virtual void PreProfileInit() {}

    virtual void PostProfileInit() {}

    virtual void PreBililiveStart() {}

    virtual void PostBililiveStart() {}

    virtual void BililiveTearDown() {}

    virtual void PostMainMessageLoopRun() {}
};

#endif  // BILILIVE_PUBLIC_BILILIVE_BILILIVE_MAIN_EXTRA_PARTS_H_