#ifndef BILILIVE_BILILIVE_BILILIVE_THREAD_DELEGATE_H
#define BILILIVE_BILILIVE_BILILIVE_THREAD_DELEGATE_H


class BililiveThreadDelegate
{
public:
    virtual ~BililiveThreadDelegate() {}

    virtual void Init() = 0;

    virtual void InitAsync() = 0;

    virtual void CleanUp() = 0;
};


#endif