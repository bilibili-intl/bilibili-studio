#ifndef BILILIVE_BILILIVE_LIVEHIME_CEF_OBS_FRONTEND_CALLBACKS_IMPL_H_
#define BILILIVE_BILILIVE_LIVEHIME_CEF_OBS_FRONTEND_CALLBACKS_IMPL_H_

#include "obs/obs-studio/UI/obs-frontend-api/obs-frontend-internal.hpp"
#include "obs/obs-studio/libobs/obs.hpp"

/*
 * obs_frontend_callbacks是用于业务上层和obs-core的obs-browser进行JavaScript数据交互时
 * 提供各种接口回调支持的，应用场景应该是：
 * 一个支持以JavaScript从native获取诸如“当前的obs核心版本、当前使用的场景、推流状态、录制状态”等信息的WEB页
 * 通过js调用从native拿到native信息进行展示或进行更复杂逻辑处理以支持WEB页面更丰富的交互；
 * 同时native也可以通过这些回调主动指示obs-browser执行js脚本以改变WEB页面的内容。
 * 
 * 我们当前没有这方面的业务需要，而且也没有现成的能提供这些js脚本的web页进行测试，所以就不做具体的接口实现了，
 * 以后有需要再按照obs-studio中的obs.exe项目的"obs-studio\UI\api-interface.cpp"的做法来写
 */


struct LivehimeOBSFrontendCallbacksImpl : obs_frontend_callbacks
{
    inline LivehimeOBSFrontendCallbacksImpl() {}

    void *obs_frontend_get_main_window(void) override
    {
        return nullptr;
    }

    void *obs_frontend_get_main_window_handle(void) override
    {
        return nullptr;
    }

    void obs_frontend_get_scenes(
        struct obs_frontend_source_list *sources) override
    {
    }

    obs_source_t *obs_frontend_get_current_scene(void) override
    {
        OBSSource source;
        return source;
    }

    void obs_frontend_set_current_scene(obs_source_t *scene) override
    {
    }

    void obs_frontend_get_transitions(
        struct obs_frontend_source_list *sources) override
    {
    }

    obs_source_t *obs_frontend_get_current_transition(void) override
    {
        OBSSource tr = nullptr;
        return tr;
    }

    void obs_frontend_set_current_transition(
        obs_source_t *transition) override
    {
    }

    void obs_frontend_get_scene_collections(
        std::vector<std::string> &strings) override
    {
    }

    char *obs_frontend_get_current_scene_collection(void) override
    {
        const char *cur_name = nullptr;
        return bstrdup(cur_name);
    }

    void obs_frontend_set_current_scene_collection(
        const char *collection) override
    {
    }

    bool obs_frontend_add_scene_collection(
        const char *name) override
    {
        bool success = false;
        return success;
    }

    void obs_frontend_get_profiles(
        std::vector<std::string> &strings) override
    {
    }

    char *obs_frontend_get_current_profile(void) override
    {
        const char *name = nullptr;
        return bstrdup(name);
    }

    void obs_frontend_set_current_profile(const char *profile) override
    {
    }

    void obs_frontend_streaming_start(void) override
    {
    }

    void obs_frontend_streaming_stop(void) override
    {
    }

    bool obs_frontend_streaming_active(void) override
    {
        return false;
    }

    void obs_frontend_recording_start(void) override
    {
    }

    void obs_frontend_recording_stop(void) override
    {
    }

    bool obs_frontend_recording_active(void) override
    {
        return false;
    }

    void obs_frontend_replay_buffer_start(void) override
    {
    }

    void obs_frontend_replay_buffer_save(void) override
    {
    }

    void obs_frontend_replay_buffer_stop(void) override
    {
    }

    bool obs_frontend_replay_buffer_active(void) override
    {
        return false;
    }

    void *obs_frontend_add_tools_menu_qaction(const char *name) override
    {
        return nullptr;
    }

    void obs_frontend_add_tools_menu_item(const char *name,
                                          obs_frontend_cb callback, void *private_data) override
    {
    }

    void obs_frontend_add_event_callback(obs_frontend_event_cb callback,
                                         void *private_data) override
    {
    }

    void obs_frontend_remove_event_callback(obs_frontend_event_cb callback,
                                            void *private_data) override
    {
    }

    obs_output_t *obs_frontend_get_streaming_output(void) override
    {
        OBSOutput output = nullptr;
        return output;
    }

    obs_output_t *obs_frontend_get_recording_output(void) override
    {
        OBSOutput out = nullptr;
        return out;
    }

    obs_output_t *obs_frontend_get_replay_buffer_output(void) override
    {
        OBSOutput out = nullptr;
        return out;
    }

    config_t *obs_frontend_get_profile_config(void) override
    {
        return nullptr;
    }

    config_t *obs_frontend_get_global_config(void) override
    {
        return nullptr;
    }

    void obs_frontend_save(void) override
    {
    }

    void obs_frontend_defer_save_begin(void) override
    {
    }

    void obs_frontend_defer_save_end(void) override
    {
    }

    void obs_frontend_add_save_callback(obs_frontend_save_cb callback,
                                        void *private_data) override
    {
    }

    void obs_frontend_remove_save_callback(obs_frontend_save_cb callback,
                                           void *private_data) override
    {
    }

    void obs_frontend_add_preload_callback(obs_frontend_save_cb callback,
                                           void *private_data) override
    {
    }

    void obs_frontend_remove_preload_callback(obs_frontend_save_cb callback,
                                              void *private_data) override
    {
    }

    void obs_frontend_push_ui_translation(
        obs_frontend_translate_ui_cb translate) override
    {
    }

    void obs_frontend_pop_ui_translation(void) override
    {
    }

    void obs_frontend_set_streaming_service(obs_service_t *service) override
    {
    }

    obs_service_t *obs_frontend_get_streaming_service(void) override
    {
        return nullptr;
    }

    void obs_frontend_save_streaming_service(void) override
    {
    }

    bool obs_frontend_preview_program_mode_active(void) override
    {
        return false;
    }

    void obs_frontend_set_preview_program_mode(bool enable) override
    {
    }

    bool obs_frontend_preview_enabled(void) override
    {
        return false;
    }

    void obs_frontend_set_preview_enabled(bool enable) override
    {
    }

    obs_source_t *obs_frontend_get_current_preview_scene(void) override
    {
        OBSSource source = nullptr;

        return source;
    }

    void obs_frontend_set_current_preview_scene(obs_source_t *scene) override
    {
    }

    void on_load(obs_data_t *settings) override
    {
    }

    void on_preload(obs_data_t *settings) override
    {
    }

    void on_save(obs_data_t *settings) override
    {
    }

    void on_event(enum obs_frontend_event event) override
    {
    }
};


#endif