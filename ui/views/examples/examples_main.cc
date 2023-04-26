#include "examples_window.h"

#include "base/at_exit.h"
#include "base/base_paths.h"
#include "base/base_switches.h"
#include "base/command_line.h"
#include "base/debug/debug_on_start_win.h"
#include "base/debug/debugger.h"
#include "base/debug/stack_trace.h"
#include "base/files/file_path.h"
#include "base/i18n/icu_util.h"
#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "base/path_service.h"
#include "base/process/memory.h"
#include "base/time/time.h"
#include "ui/base/ui_base_paths.h"
#include "ui/base/resource/resource_bundle.h"
#include "base/message_loop/message_loop.h"
#include "base/run_loop.h"
#include <objbase.h>

// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


class TestSuite {
public:
  TestSuite(int argc, char** argv) : initialized_command_line_(false) {
    PreInitialize(argc, argv, true);
  }

  virtual ~TestSuite()
  {
    if (initialized_command_line_)
      CommandLine::Reset();
  }


  int Run()
  {
    Initialize();

    base::MessageLoopForUI message_loop;

	views::examples::ShowExamplesWindow(views::examples::QUIT_ON_CLOSE/*DO_NOTHING_ON_CLOSE*/);

    base::RunLoop run_loop;
    run_loop.Run();

    Shutdown();

    return 0;
  }

protected:
  // This constructor is only accessible to specialized test suite
  // implementations which need to control the creation of an AtExitManager
  // instance for the duration of the test.
  TestSuite(int argc, char** argv, bool create_at_exit_manager);

  virtual void Initialize()
  {
    ::OleInitialize(NULL);

    base::FilePath exe;
    PathService::Get(base::FILE_EXE, &exe);
    base::FilePath log_filename = exe.ReplaceExtension(FILE_PATH_LITERAL("log"));
    logging::LoggingSettings settings;
    settings.logging_dest = logging::LOG_TO_ALL;
    settings.log_file = log_filename.value().c_str();
    settings.delete_old = logging::DELETE_OLD_LOG_FILE;
    logging::InitLogging(settings);
    logging::SetLogItems(true, true, true, true);

    CHECK(base::debug::EnableInProcessStackDumping());

    base::Time::EnableHighResolutionTimer(true);

    icu_util::Initialize();

    ui::RegisterPathProvider();
    ui::ResourceBundle::InitSharedInstanceWithLocale("en-US", NULL);
  }
  virtual void Shutdown()
  {
    ::OleUninitialize();
  }

private:
  void PreInitialize(int argc, char** argv, bool create_at_exit_manager)
  {
    base::EnableTerminationOnHeapCorruption();
    initialized_command_line_ = CommandLine::Init(argc, argv);

    if (create_at_exit_manager)
      at_exit_manager_.reset(new base::AtExitManager);
  }

  scoped_ptr<base::AtExitManager> at_exit_manager_;

  bool initialized_command_line_;

  DISALLOW_COPY_AND_ASSIGN(TestSuite);
};

int main(int argc, char** argv) {

  TestSuite test(argc, argv);
  test.Run();

  return 0;
}
