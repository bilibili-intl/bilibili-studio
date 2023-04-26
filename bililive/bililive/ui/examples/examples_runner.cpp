#include "examples_runner.h"

#include "base/run_loop.h"

#include "bililive/bililive/ui/examples/examples_frame_view.h"


int ExamplesRunner::Run() {
    examples::ExamplesFrameView::ShowWindow();
    base::RunLoop run_loop;
    run_loop.Run();

    return 0;
}