#ifndef BILILIVE_BILILIVE_UI_STARTUP_STARTUP_BILILIVE_CREATOR_H
#define BILILIVE_BILILIVE_UI_STARTUP_STARTUP_BILILIVE_CREATOR_H


#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/files/file_path.h"
#include "base/gtest_prod_util.h"
#include "url/gurl.h"
#include "bililive/public/bililive/bililive_process.h"
#include "ui/gfx/image/image.h"
#include "ui/base/resource/resource_bundle.h"
#include "grit/ui_resources.h"
#include "ui/gfx/image/image_skia.h"
#include "bililive/app/bililive_exe_resource.h"
#include "ui/base/win/dpi.h"
#include "ui/gfx/icon_util.h"
#include "ui/base/models/simple_menu_model.h"
#include "grit/generated_resources.h"
#include "ui/base/l10n/l10n_util.h"


class BililiveOBS;
class CommandLine;
class GURL;
class PrefService;


class StartupBililiveCreator
{
public:
    StartupBililiveCreator();
    ~StartupBililiveCreator();

    bool Start(const CommandLine &cmd_line,
               const base::FilePath &cur_dir,
               int *return_code)
    {
        return ProcessCmdLineImpl(cmd_line, cur_dir, true, return_code, this);
    }

    bool LaunchBililive(const CommandLine &command_line,
                        const base::FilePath &cur_dir,
                        int *return_code);

    bool LaunchUploader(const CommandLine &command_line,
        const base::FilePath &cur_dir,
        int *return_code);

private:
    static bool ProcessCmdLineImpl(const CommandLine &command_line,
                                   const base::FilePath &cur_dir,
                                   bool process_startup,
                                   int *return_code,
                                   StartupBililiveCreator *bililive_creator);

    DISALLOW_COPY_AND_ASSIGN(StartupBililiveCreator);
};

#endif