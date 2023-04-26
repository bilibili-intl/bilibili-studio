
#ifndef BILILIVE_SECRET_CORE_ANCHOR_INFO_IMPL_H_
#define BILILIVE_SECRET_CORE_ANCHOR_INFO_IMPL_H_

#include "bililive/secret/public/anchor_info.h"


namespace secret {

class AnchorInfoImpl : public AnchorInfo{

public:
    AnchorInfoImpl();

    ~AnchorInfoImpl() = default;

    const std::string& room_title() const override;
    void set_room_title(const std::string& title) override;

    const std::string& room_cover() const override;
    void set_room_cover(const std::string& title) override;

    const std::string& vert_room_cover() const override;
    void set_vert_room_conver(const std::string& title) override;

    void set_enable_vert_cover(bool enable) override;
    bool enable_vert_cover() override;

    int room_level() const override;
    void set_room_level(int level) override;

    int64_t room_gift() const override;
    void set_room_gift(int64_t gift) override;

    int64_t room_fans() const override;
    void set_room_fans(int64_t fans) override;

    int current_area() const override;
    void set_current_area(int area) override;

    const std::string& current_area_name() const override;
    void set_current_area_name(const std::string& area_name) override;

    const std::string& current_parent_area_name() const override;
    void set_current_parent_area_name(const std::string& area_name) override;

    int current_parent_area() const override;
    void set_current_parent_area(int area) override;

    int64_t current_xp() const override;
    void set_current_xp(int64_t current_xp) override;

    int64_t max_xp() const override;
    void set_max_xp(int64_t max_xp) override;

    int cdn_src() override;
    int cdn_checked() override;

    void set_cdn_src(int src) override;
    void set_cdn_checked(int checked) override;

    void set_recommend_ip(const std::string& ip) override;
    const std::string& recommend_ip() const override;

    int64_t medal_status() const override;
    void set_medal_status(int64_t medal_status) override;

    const std::string& medal_name() const override;
    void set_medal_name(const std::string& medal_name) override;

    int64_t guard_count() const override;
    void set_guard_count(int64_t guard_count) override;

    void set_tag_info(int64_t tag_id, const std::string& tag_name) override;
    int64_t tag_id() const override { return tag_id_; }

    void set_switch_tag_status(bool falg) override {
        switch_tag_live_status_ = falg;
    }

    bool get_switch_tag_status() override {
        return switch_tag_live_status_;
    }

    const std::string& tag_name() const override { return tag_name_; }

    void set_live_key(const std::string& live_key, bool update_last_record = true) override
    {
        live_key_ = live_key;
        if (update_last_record)
        {
            last_live_key_ = live_key_;
        }
    }
    const std::string& live_key() const override { return live_key_; }
    const std::string& last_live_key() const override { return last_live_key_; }

    void set_live_status(int status) override{ live_status_ = status; }
    int live_status() override { return live_status_; }

    void set_sub_session_key(const std::string& sub_session_key) override { sub_session_key_ = sub_session_key; }
    const std::string& sub_session_key() const override { return sub_session_key_; }

    void set_captain_warn_status(int status) override
    {
        is_warn_ = status;
    }

    int get_captain_warn_status() override
    {
        return is_warn_;
    }

    void set_captain_expired(int val) override
    {
        expired_ = val;
    }

    int get_captain_expired() override
    {
        return expired_;
    }

    void set_captain_will_expire(int val) override
    {
        will_expired_ = val;
    }

    int get_captain_will_expire() override
    {
        return will_expired_;
    }

    //�Ƕ������°�ֱ����
    void set_join_slide(int status) override;
    int get_join_slide() override;

    void set_medal_fans(int64_t fans) override;
    int64_t get_medal_fans() override;

    void set_land_scape_model(bool land_scape_model) override;
    bool get_land_scape_model() override;

    // ʵ�������
    void set_identify_status(int identify_status) override;
    int identify_status() override;

    void set_is_adult(bool is_adult) override;
    // ��ǰ�ݲ�ʹ������ֶ�
    // v3.40.3����֮�����ڷ���˶���δ������ж��߼��кܶ���case��ʵ�ʳ�����񡢿����������ȵȣ���
    // ����ж��߼�Ŀǰ��δ�������룬���Ƿŵ������ӿ���������is_adult�Ǵӷ�����Ϣ��ȡ�ӿ��õģ�
    // ��Գ��������ж��߼���δ�񿪲��ӿ�һ��ȫ�棬�ᵼ���û����������ܲ���Web���ƶ��˶�û��������getroominfo��is_adult�ֶν���ǰ��Լ���Ĺ��ܣ���
    // ��PCֱ�������ܲ�����������ԣ������� @ӭ�� �̶���Ŀǰ�ݲ�ʹ��is_adult�ֶΣ����Ͻӿ�һ�ɷ���true������ֶ�Ŀǰֻ��PCֱ�������ã�����Ӱ�������ˣ�
    // �ȹ������˽�δ����Ȩ���жϴӿ����ӿڰ����������ȷ��getroominfo�Ϳ����ӿڵ�Ȩ���ж��߼�һ��֮����ʱ��PCֱ������ʹ������ֶ�
    //bool is_adult() override;

    void set_have_live(int have_live) override;
    int have_live() override;

    void set_watched_switch(bool watched_switch)override {
        watched_switch_ = watched_switch;
    }
    bool get_watched_switch() override {
        return watched_switch_;
    }

    void set_is_new_anchor(bool is_new) { is_new_anchor_ = is_new; }
    bool is_new_anchor() { return is_new_anchor_; }

	//Ԥ���زĴ����ӦĬ�Ϸ���id
    void set_default_area_map(const std::map<std::string, int>& default_area_map) { default_area_map_ = default_area_map; }
    const std::map<std::string, int>& get_default_area_map() { return default_area_map_; }
    //���������б�
    void set_anchor_task_schedule_list(const std::vector<TaskScheduleInfo>& anchor_task_schedule_list)
    {
        anchor_task_schedule_list_ = anchor_task_schedule_list;
    }
    const std::vector<TaskScheduleInfo>& get_anchor_task_schedule_list()
    {
        return anchor_task_schedule_list_;
    }
    //�����û�(��R)
    void set_core_user_func_open(bool is_open)
    {
        is_core_user_func_open_ = false;// is_open;//�����û���ʾһ�ڹ������ߣ��������ڵ�Ļ����ʾ
    }
    bool get_core_user_func_open()
    {
        return is_core_user_func_open_;
    }
    void set_core_user_config(const CoreUserConfig& config)
    {
        core_user_config_ = config;
    }
    const CoreUserConfig& get_core_user_config()
    {
        return core_user_config_;
    }
    void set_popular_rank_info(const PopularRankInfo& info) override
    {
        popular_rank_info_ = info;
    }
    const PopularRankInfo& get_popular_rank_info()override
    {
        return popular_rank_info_;
    }

private:
    std::string room_title_;
    std::string room_cover_;
    bool enable_vert_room_cover_ = false;
    std::string vert_room_cover_;
    std::string area_name_;
    std::string area_parent_name_;
    int room_level_;
    int64_t room_gift_;
    int64_t room_fans_;
    int current_area_ = -1;
    int current_parent_area_ = -1;
    int64_t current_xp_;
    int64_t max_xp_;
    int cdn_src_ = -1;
    int cdn_checked_ = 1;
    std::string recommend_ip_;
    int64_t medal_status_;
    std::string medal_name_;
    int64_t guard_count_;
    int64_t tag_id_;
    std::string tag_name_;
    std::string live_key_;
    std::string last_live_key_;
    int live_status_ = 0;
    std::string sub_session_key_;
    int is_warn_ = 0; //�Ƿ���ʾͷ�����Ϸ��ĺ��
    int expired_ = 0; //���ڴ�Ա��
    int will_expired_ = 0; //�������ڴ�Ա��
    int join_slide_ = 0; //��ʾδ���룬1��ʾ����
    int64_t medal_fans_ = 0; // �����ķ�˿ѫ����
    bool land_scape_model_ = true; //Ĭ��ȫ��
    int identify_status_ = -1;    // ʵ����֤״̬��0 �����У�1 ��ͨ����2 ���أ�3 δ����
    bool switch_tag_live_status_ = false;
    bool is_adult_ = false; // �Ƿ���� ö�٣�false-δ���ꣻtrue-�ѳ���
    int have_live_ = -1;    // �Ƿ񿪲�����0 ��1 ��
    bool watched_switch_ = false; // true: "����", false:"����"
    bool is_new_anchor_ = false;//180�쿪��С��30����Ϊ������
    std::map<std::string, int> default_area_map_;
    std::vector<TaskScheduleInfo> anchor_task_schedule_list_;
    bool is_core_user_func_open_ = false;//�����û�(��R)����
    CoreUserConfig core_user_config_;//�����û�(��R)����
    PopularRankInfo  popular_rank_info_; //


};

}   // namespace secret

#endif  // BILILIVE_SECRET_CORE_ANCHOR_INFO_IMPL_H_
