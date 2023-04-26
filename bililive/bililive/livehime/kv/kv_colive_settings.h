#ifndef BILILIVE_BILILIVE_LIVEHIME_KV_KV_COLIVE_SETTINGS_H_
#define BILILIVE_BILILIVE_LIVEHIME_KV_KV_COLIVE_SETTINGS_H_

#include <string>


namespace livehime {

    class KVColiveSettings {
    public:
        struct Settings {
            bool colive_anchor_voice_switch = false;

            /**
             * ���ڸ�������������ǿ����/���߱��غ����Ŀ��ء�
             * 0: �����ܿ�������
             * 1: ǿ���߱��غ��������ܼ������Դʹ�������
             * 2: ǿ�Ʋ��߱��غ�����
             * �������� 0 һ����
             */
            int force_self_publish = 0;

            /**
             * ���غ����ܿ��أ�Ӱ��������������������ǿ�����õģ���
             * true: ���ݼ������Դʹ���������ֵ�ȽϾ����Ƿ񱾵غ�����
             * false: ���߱��غ�����
             */
            bool is_self_publish = true;

            bool phantom_gpu_result = false;
            int max_sample_count = 0;
            int max_cpu_rate = -1;
            int max_gpu_rate = -1;
            int max_mem_occupied = -1;
            int max_ndf_rate = -1;
            int max_rdf_rate = -1;
            int max_edf_rate = -1;

            bool reduce_bitrate_enabled = false;
            // ����������ʱ����ֱ�������ʣ���λ Kbps
            int reduce_bitrate = 0;
            // ����������ʱ��ֱ�������ʵ����ޣ���λ Kbps
            int min_reduced_bitrate = 0;

            /**
             * ָ��������ֱ���ͱ��غ�������֮���л�ʱ��
             * �Ƿ�Ӧ�ò��������ơ�
             */
            bool is_spp_continued = false;

            //�Ƿ������ϵͳ����
            bool system_voice = false;

            bool chat_system_voice = false;

            //�Ƿ��ϵͳ��������ϵͳ�汾��
            bool system_voice_windows_ver_switch = false;

            int64_t windows_build_ver = 0;
        };

        static void RequestColiveSettings();
        static const Settings& GetSettings();

        static void SetChatRoomStatus(bool open);
    private:
        static void OnGetColiveSettings(
            bool valid, int code, const std::string& err_msg, const std::string& data);

        static void ResetSettings(Settings* settings);

        static Settings colive_settings_;
        static bool chat_system_white_;
    };

}

#endif  // BILILIVE_BILILIVE_LIVEHIME_KV_KV_COLIVE_SETTINGS_H_