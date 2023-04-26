#ifndef BILILIVE_BILILIVE_PROFILES_PROFILE_H
#define BILILIVE_BILILIVE_PROFILES_PROFILE_H


#include <string>

#include "base/basictypes.h"
#include "base/containers/hash_tables.h"
#include "base/logging.h"
#include "base/files/file_path.h"
#include "bililive/bililive/themes/theme_service.h"

class PrefService;


namespace base
{
    class SequencedTaskRunner;
    class Time;
}

class Profile
{
public:

    class Delegate
    {
    public:
        virtual ~Delegate();

        virtual void OnProfileCreated(Profile *profile,
                                      bool success,
                                      bool is_new_profile) = 0;
    };

    static const char kProfileKey[];

    Profile();
    virtual ~Profile();

    static Profile* CreateProfile(const base::FilePath &path,
                                  Delegate *delegate,
                                  base::SequencedTaskRunner *sequenced_task_runner);

    static Profile* CreateGlobalProfile(const base::FilePath &path,
                                        Delegate *delegate,
                                        base::SequencedTaskRunner *sequenced_task_runner);

    virtual const base::FilePath& GetPath() const = 0;
    virtual PrefService *GetPrefs() = 0;
    virtual ThemeService *GetThemeService() = 0;
private:

    DISALLOW_COPY_AND_ASSIGN(Profile);
};

#endif