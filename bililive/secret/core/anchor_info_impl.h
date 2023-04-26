
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

    //是都开启新版直播间
    void set_join_slide(int status) override;
    int get_join_slide() override;

    void set_medal_fans(int64_t fans) override;
    int64_t get_medal_fans() override;

    void set_land_scape_model(bool land_scape_model) override;
    bool get_land_scape_model() override;

    // 实名与成年
    void set_identify_status(int identify_status) override;
    int identify_status() override;

    void set_is_adult(bool is_adult) override;
    // 当前暂不使用这个字段
    // v3.40.3发出之后，由于服务端对于未成年的判断逻辑有很多种case（实际成年与否、开播白名单等等），
    // 这个判断逻辑目前并未单独抽离，而是放到开播接口里；而这里的is_adult是从房间信息获取接口拿的，
    // 其对成年与否的判断逻辑并未像开播接口一样全面，会导致用户在其它端能播（Web和移动端都没有做根据getroominfo的is_adult字段进行前置约束的功能），
    // 在PC直播姬不能播的情况。所以，与服务端 @迎风 商定，目前暂不使用is_adult字段，线上接口一律返回true（这个字段目前只有PC直播姬在用，不会影响其他端）
    // 等过后服务端将未成年权限判断从开播接口剥离出来，以确保getroominfo和开播接口的权限判断逻辑一致之后，那时候PC直播姬再使用这个字段
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

	//预设素材大类对应默认分区id
    void set_default_area_map(const std::map<std::string, int>& default_area_map) { default_area_map_ = default_area_map; }
    const std::map<std::string, int>& get_default_area_map() { return default_area_map_; }
    //主播任务列表
    void set_anchor_task_schedule_list(const std::vector<TaskScheduleInfo>& anchor_task_schedule_list)
    {
        anchor_task_schedule_list_ = anchor_task_schedule_list;
    }
    const std::vector<TaskScheduleInfo>& get_anchor_task_schedule_list()
    {
        return anchor_task_schedule_list_;
    }
    //核心用户(大R)
    void set_core_user_func_open(bool is_open)
    {
        is_core_user_func_open_ = false;// is_open;//核心用户显示一期功能下线，二期是在弹幕区显示
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
    int is_warn_ = 0; //是否显示头像右上方的红点
    int expired_ = 0; //到期船员数
    int will_expired_ = 0; //即将到期船员数
    int join_slide_ = 0; //表示未加入，1表示加入
    int64_t medal_fans_ = 0; // 点亮的粉丝勋章数
    bool land_scape_model_ = true; //默认全屏
    int identify_status_ = -1;    // 实名认证状态：0 申请中，1 已通过，2 驳回，3 未申请
    bool switch_tag_live_status_ = false;
    bool is_adult_ = false; // 是否成年 枚举：false-未成年；true-已成年
    int have_live_ = -1;    // 是否开播过：0 否，1 是
    bool watched_switch_ = false; // true: "看过", false:"人气"
    bool is_new_anchor_ = false;//180天开播小于30分钟为新主播
    std::map<std::string, int> default_area_map_;
    std::vector<TaskScheduleInfo> anchor_task_schedule_list_;
    bool is_core_user_func_open_ = false;//核心用户(大R)开关
    CoreUserConfig core_user_config_;//核心用户(大R)配置
    PopularRankInfo  popular_rank_info_; //


};

}   // namespace secret

#endif  // BILILIVE_SECRET_CORE_ANCHOR_INFO_IMPL_H_
