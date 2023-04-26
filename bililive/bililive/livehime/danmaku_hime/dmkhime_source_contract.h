#ifndef BILILIVE_BILILIVE_LIVEHIME_DANMAKU_HIME_DMKHIME_SOURCE_CONTRACT_H_
#define BILILIVE_BILILIVE_LIVEHIME_DANMAKU_HIME_DMKHIME_SOURCE_CONTRACT_H_


namespace gfx {
    class Canvas;
}

namespace contracts {

    class DmkhimeSourcePresenter {
    public:
        virtual ~DmkhimeSourcePresenter() = default;

        virtual bool Initialize() = 0;
        virtual void Render(gfx::Canvas* canvas) = 0;
    };

    class DmkhimeSourceView {
    public:
        virtual ~DmkhimeSourceView() = default;

        virtual void OnOutgoing(bool first) = 0;
        virtual void OnAutism() = 0;
    };

}

#endif  // BILILIVE_BILILIVE_LIVEHIME_DANMAKU_HIME_DMKHIME_SOURCE_CONTRACT_H_