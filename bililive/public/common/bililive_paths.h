#ifndef BILILIVE_PUBLIC_COMMON_BILILIVE_PATHS_H_
#define BILILIVE_PUBLIC_COMMON_BILILIVE_PATHS_H_

namespace bililive {

enum {
    PATH_START = 1000,

    DIR_APP = PATH_START, // Directory where dlls and data reside.
    DIR_LOGS, // Directory where logs should be written.
    DIR_USER_DATA, // Directory where user data can be written.
    DIR_CRASH_DUMPS, // Directory where crash dumps are written.
    DIR_RESOURCES, // Directory containing separate file resources
    DIR_INSPECTOR, // Directory where web inspector is located.
    DIR_APP_DICTIONARIES, // Directory where the global dictionaries are.
    DIR_USER_DOCUMENTS, // Directory for a user's "My Documents".
    DIR_USER_MUSIC, // Directory for a user's music.
    DIR_USER_PICTURES, // Directory for a user's pictures.
    DIR_USER_VIDEOS, // Directory for a user's videos.
    DIR_DEFAULT_DOWNLOADS_SAFE, // Directory for a user's "My Documents/Downloads", (Windows) or
    DIR_DEFAULT_DOWNLOADS, // Directory for a user's downloads.
    DIR_APP_UPDATE, // Directory containing application update related files (such as an installer).
    DIR_UPDATE_REPORT, // Directory containing application update related files (such as an installer).
    FILE_RESOURCE_MODULE, // Full path and filename of the module that
    FILE_RESOURCES_PACK, // Full path to the .pak file containing
    DIR_APP_TEMP, // Directory for app temporary.
    DIR_KV_CACHE, // Directory for kv resource.
    DIR_FIGMA, // Directory for figma files.
    DIR_STORM_CACHE,
	DIR_PK_FIGMA, // Directory for vtuber pk figma files.
    DIR_PK_BK_IMG, //虚拟PK背景图片路径
    DIR_FIGMA_KEY_COMBO, // Directory for figma key combos.
    DIR_UNITY, // Directory for unity application.
    DIR_GAME,   //游戏中心
    DIR_FACE_EFFECT, //脸萌贴纸
    DIR_BILI_DOWNLOAD_CACHE,
    DIR_BILI_PRESET_MATERIAL,
    DIR_BILI_USER_MATERIAL,
    DIR_BILI_BEAUTY_CAMERA_CACHE,
    DIR_WEBVIEW_OFFLINE,
    PATH_END
};

void RegisterPathProvider();

}   // namespace bililive

#endif  // BILILIVE_PUBLIC_COMMON_BILILIVE_PATHS_H_