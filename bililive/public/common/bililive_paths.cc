#include "bililive/public/common/bililive_paths.h"

#include "base/file_util.h"
#include "base/logging.h"
#include "base/mac/bundle_locations.h"
#include "base/path_service.h"
#include "base/strings/string_util.h"
#include "base/threading/thread_restrictions.h"

#include "bililive/public/common/bililive_constants.h"
#include "bililive/public/common/bililive_paths_internal.h"

namespace bililive {

bool PathProvider(int key, base::FilePath *result)
{
    switch (key) {
        case bililive::DIR_APP:
            return PathService::Get(base::DIR_MODULE, result);

        case bililive::DIR_LOGS:
            return PathService::Get(bililive::DIR_USER_DATA, result);

        case bililive::FILE_RESOURCE_MODULE:
            return PathService::Get(base::FILE_MODULE, result);

        default:
            break;
    }

    bool create_dir = false;
    base::FilePath path;
    switch (key) {
        case bililive::DIR_USER_DATA:
            if (!GetDefaultUserDataDirectory(&path)) {
                NOTREACHED();
                return false;
            }

            create_dir = true;
            break;

        case bililive::DIR_USER_DOCUMENTS:
            if (!GetUserDocumentsDirectory(&path)) {
                return false;
            }

            create_dir = true;
            break;

        case bililive::DIR_USER_MUSIC:
            if (!GetUserMusicDirectory(&path)) {
                return false;
            }

            break;

        case bililive::DIR_USER_PICTURES:
            if (!GetUserPicturesDirectory(&path)) {
                return false;
            }

            break;

        case bililive::DIR_USER_VIDEOS:
            if (!GetUserVideosDirectory(&path)) {
                return false;
            }

            break;

        case bililive::DIR_DEFAULT_DOWNLOADS_SAFE:
            if (!GetUserDownloadsDirectorySafe(&path)) {
                return false;
            }

            break;

        case bililive::DIR_DEFAULT_DOWNLOADS:
            if (!GetUserDownloadsDirectory(&path)) {
                return false;
            }

            break;

        case bililive::DIR_CRASH_DUMPS:
            if (!GetDefaultUserDataDirectory(&path)) {
                return false;
            }

            path = path.Append(FILE_PATH_LITERAL("Crash Reports"));
            create_dir = true;
            break;

        case bililive::DIR_RESOURCES:
            if (!PathService::Get(bililive::DIR_APP, &path)) {
                return false;
            }

            path = path.Append(FILE_PATH_LITERAL("resources"));
            break;

        case bililive::DIR_INSPECTOR:
            if (!PathService::Get(bililive::DIR_RESOURCES, &path)) {
                return false;
            }

            path = path.Append(FILE_PATH_LITERAL("inspector"));
            break;

        case bililive::DIR_APP_DICTIONARIES:
            if (!PathService::Get(base::DIR_EXE, &path)) {
                return false;
            }

            path = path.Append(FILE_PATH_LITERAL("Dictionaries"));
            create_dir = true;
            break;

        case bililive::DIR_APP_UPDATE:
            if (!PathService::Get(base::DIR_TEMP, &path)) {
                return false;
            }

            path = path.Append(kAppUpdateDirName);
            create_dir = true;
            break;

        case bililive::DIR_UPDATE_REPORT:
            if (!GetDefaultUserDataDirectory(&path))
            {
                return false;
            }

            path = path.Append(FILE_PATH_LITERAL("Update"));
            create_dir = true;
            break;

        case bililive::FILE_RESOURCES_PACK:
            if (!PathService::Get(base::DIR_MODULE, &path)) {
                return false;
            }

            path = path.Append(FILE_PATH_LITERAL("resources.pak"));
            break;

        case bililive::DIR_APP_TEMP:
            if (!PathService::Get(base::DIR_TEMP, &path))
            {
                return false;
            }

            path = path.Append(kAppTempDirName);
            create_dir = true;
            break;

        case bililive::DIR_KV_CACHE:
            if (!GetDefaultUserDataDirectory(&path))
            {
                return false;
            }

            path = path.Append(FILE_PATH_LITERAL("KV Cache"));
            create_dir = true;
            break;

        case bililive::DIR_FIGMA:
            if (!GetDefaultUserDataDirectory(&path))
            {
                return false;
            }

            path = path.Append(FILE_PATH_LITERAL("Figma"));
            create_dir = true;
            break;

        case bililive::DIR_STORM_CACHE:
            if (!GetDefaultUserDataDirectory(&path))
            {
                return false;
            }

            path = path.Append(FILE_PATH_LITERAL("Cache")).Append(FILE_PATH_LITERAL("Storm"));
            create_dir = true;
            break;

        case bililive::DIR_PK_FIGMA:
            if (!GetDefaultUserDataDirectory(&path))
            {
                return false;
            }

            path = path.Append(FILE_PATH_LITERAL("Cache")).Append(FILE_PATH_LITERAL("PK Figma"));
            create_dir = true;
            break;

        case bililive::DIR_PK_BK_IMG:
            if (!GetDefaultUserDataDirectory(&path))
            {
                return false;
            }
            path = path.Append(FILE_PATH_LITERAL("Cache")).Append(FILE_PATH_LITERAL("VtuberPKBG"));
            create_dir = true;
            break;

        case bililive::DIR_FIGMA_KEY_COMBO:
            if (!GetDefaultUserDataDirectory(&path))
            {
                return false;
            }

            path = path.Append(FILE_PATH_LITERAL("Figma KeyCombos"));
            create_dir = true;
            break;

        case bililive::DIR_UNITY:
            if (!PathService::Get(bililive::DIR_APP, &path)) {
                return false;
            }

            path = path.Append(FILE_PATH_LITERAL("unity"));
            create_dir = true;
            break;
        case bililive::DIR_GAME:
            if (!GetDefaultUserDataDirectory(&path))
            {
                return false;
            }

            path = path.Append(FILE_PATH_LITERAL("Game File"));
            create_dir = true;
            break;

        case bililive::DIR_BILI_DOWNLOAD_CACHE:
            if (!GetDefaultUserDataDirectory(&path))
            {
                return false;
            }
            path = path.Append(FILE_PATH_LITERAL("Cache")).Append(FILE_PATH_LITERAL("Download"));
            create_dir = true;
            break;

        case  bililive::DIR_BILI_PRESET_MATERIAL:
            if (!PathService::Get(bililive::DIR_APP, &path)) {
                return false;
            }

            path = path.Append(FILE_PATH_LITERAL("preset_material"));
            create_dir = true;
            break;

        case bililive::DIR_BILI_USER_MATERIAL:
            if (!GetDefaultUserDataDirectory(&path))
            {
                return false;
            }
            path = path.Append(FILE_PATH_LITERAL("Cache")).Append(FILE_PATH_LITERAL("user_material"));
            create_dir = true;
            break;

        case bililive::DIR_BILI_BEAUTY_CAMERA_CACHE:
            if (!GetDefaultUserDataDirectory(&path))
            {
                return false;
            }

            path = path.Append(FILE_PATH_LITERAL("Cache")).Append(FILE_PATH_LITERAL("Beauty Camera"));
            create_dir = true;
            break;

        case bililive::DIR_FACE_EFFECT:
            if (!GetDefaultUserDataDirectory(&path))
            {
                return false;
            }

            path = path.Append(FILE_PATH_LITERAL("Cache")).Append(FILE_PATH_LITERAL("Face Effect"));
            create_dir = true;
            break;

        case bililive::DIR_WEBVIEW_OFFLINE:
            if (!GetDefaultUserDataDirectory(&path))
            {
                return false;
            }

            path = path.Append(FILE_PATH_LITERAL("Cache")).Append(FILE_PATH_LITERAL("Webview Offline"));
            create_dir = true;
            break;

        default:
            return false;
    }

    base::ThreadRestrictions::ScopedAllowIO allow_io;
    if (create_dir && !base::PathExists(path) &&
        !file_util::CreateDirectory(path))
    {
        return false;
    }

    *result = path;
    return true;
}

void RegisterPathProvider()
{
    PathService::RegisterProvider(PathProvider, PATH_START, PATH_END);
}

}   // namespace bililive