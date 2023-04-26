
#ifndef BILILIVE_SECRET_PUBLIC_ANCHOR_INFO_H_
#define BILILIVE_SECRET_PUBLIC_ANCHOR_INFO_H_

#include <string>
namespace secret {

class AnchorInfo {
public:
    virtual ~AnchorInfo() {}

    virtual const std::string& room_title() const = 0;
    virtual void set_room_title(const std::string& title) = 0;

    virtual const std::string& room_cover() const = 0;
    virtual void set_room_cover(const std::string& title) = 0;

    virtual const std::string& vert_room_cover() const = 0;
    virtual void set_vert_room_conver(const std::string& title) = 0;

    virtual void set_enable_vert_cover(bool enable) = 0;
    virtual bool enable_vert_cover() = 0;

    virtual int room_level() const = 0;
    virtual void set_room_level(int level) = 0;

    virtual int64_t room_gift() const = 0;
    virtual void set_room_gift(int64_t gift) = 0;

    virtual int64_t room_fans() const = 0;
    virtual void set_room_fans(int64_t fans) = 0;

    virtual int current_area() const = 0;
    virtual void set_current_area(int area) = 0;

    virtual const std::string& current_area_name() const = 0;
    virtual void set_current_area_name(const std::string& area_name) = 0;

    virtual const std::string& current_parent_area_name() const  = 0;
    virtual void set_current_parent_area_name(const std::string& area_name) = 0;

    virtual int current_parent_area() const = 0;
    virtual void set_current_parent_area(int area) = 0;

    virtual int64_t current_xp() const = 0;
    virtual void set_current_xp(int64_t current_xp) = 0;

    virtual int64_t max_xp() const = 0;
    virtual void set_max_xp(int64_t max_xp) = 0;

    virtual int cdn_src() = 0;
    virtual int cdn_checked() = 0;

    virtual void set_cdn_src(int src) = 0;
    virtual void set_cdn_checked(int checked) = 0;

    virtual void set_recommend_ip(const std::string& ip) = 0;
    virtual const std::string& recommend_ip() const = 0;

    virtual int64_t medal_status() const = 0;
    virtual void set_medal_status(int64_t medal_status) = 0;

    virtual const std::string& medal_name() const = 0;
    virtual void set_medal_name(const std::string& medal_name) = 0;

    virtual int64_t guard_count() const = 0;
    virtual void set_guard_count(int64_t guard_count) = 0;

    virtual void set_tag_info(int64_t tag_id, const std::string& tag_name) = 0;
    virtual int64_t tag_id() const = 0;
    virtual const std::string& tag_name() const = 0;

    virtual void set_live_key(const std::string& live_key, bool update_last_record = true) = 0;
    virtual const std::string& live_key() const = 0;
    virtual const std::string& last_live_key() const = 0;

    virtual void set_live_status(int status) = 0;
    virtual int live_status() = 0;

    virtual void set_sub_session_key(const std::string& sub_session_key) = 0;
    virtual const std::string& sub_session_key() const = 0;

    //�Ƿ�չʾͷ�����ϽǺ��
    virtual void set_captain_warn_status(int status) = 0;
    virtual int get_captain_warn_status() = 0;

    //�Ѿ����ڴ�Ա��
    virtual void set_captain_expired(int val) = 0;
    virtual int get_captain_expired() = 0;

    //�������ڴ�Ա��
    virtual void set_captain_will_expire(int val) = 0;
    virtual int get_captain_will_expire() = 0;

    //�Ƕ������°�ֱ����
    virtual void set_join_slide(int status) = 0;
    virtual int get_join_slide() = 0;

    // �����ķ�˿��
    virtual void set_medal_fans(int64_t fans) = 0;
    virtual int64_t get_medal_fans() = 0;

    virtual void set_land_scape_model(bool land_scape_model) = 0;
    virtual bool get_land_scape_model() = 0;

    // ʵ�������
    virtual void set_identify_status(int identify_status) = 0;
    virtual int identify_status() = 0;

    virtual void set_is_adult(bool is_adult) = 0;
    //virtual bool is_adult() = 0;

    virtual void set_have_live(int have_live) = 0;
    virtual int have_live() = 0;

    virtual void set_watched_switch(bool watched_switch) = 0;
    virtual bool get_watched_switch() = 0;

    //180�쿪��С��30����
	virtual void set_is_new_anchor(bool is_new) = 0;
	virtual bool is_new_anchor() = 0;
    //Ԥ���زĴ����ӦĬ�Ϸ���id
	virtual void set_default_area_map(const std::map<std::string,int>& default_area_map) = 0;
	virtual const std::map<std::string, int>& get_default_area_map() = 0;

	struct TaskScheduleInfo//��������
	{
        enum class TouchType
        {
            Begin_Live = 1,
            Close_Live = 2
        };
		int id = 0;//����id
		int touch_type = 0;//�������ͣ�1-���뿪����ʱ�� 2-�ز�
		int live_time_sec = 0;//�࿪���������
	};
	virtual void set_anchor_task_schedule_list(const std::vector<TaskScheduleInfo>& anchor_task_schedule_list) = 0;
	virtual const std::vector<TaskScheduleInfo>& get_anchor_task_schedule_list() = 0;

	struct CoreUserConfig//�����û�(��R)���������Ϣ
	{
		int carousel_show_time = 10;//  �ֲ�չʾʱ��
		int entry_show_time = 5;//  ����չʾʱ��
		int carousel_max_count = 5;//  ����ֲ��û��� topN
		int carousel_queue_count = 100;//  �ֲ����г���
		int entry_queue_count = 100;//  �������г���
		int entry_queue_time = 60;//  ������������ʱ��(��)
		int auto_min_time = 5;//  X��֮���Զ�������С̬
	};
	virtual void set_core_user_func_open(bool is_open) = 0;
	virtual bool get_core_user_func_open() = 0;
    virtual void set_core_user_config(const CoreUserConfig &config) = 0;
    virtual const CoreUserConfig& get_core_user_config() = 0;

    virtual void set_switch_tag_status(bool falg) = 0;
    virtual bool get_switch_tag_status() = 0;

	//����������
	struct PopularRankInfo
	{
		PopularRankInfo()
			:exist(false)
			, rank(0)
			, countdown(0)
			, timestamp(0)
			, url("")
			, on_rank_name("")
			, rank_name("")

		{
		}
		bool exist;        //�Ƿ����
		int64_t rank;     //����0  ��ʾû���ڰ��ϣ�Ϊ0ʱ�������赹��ʱ
		int64_t countdown; //����ʱ
		int64_t timestamp;
		std::string url;
		std::string on_rank_name; //û�з���Ĭ��������(�ϰ�ʱ�İ�����)
		std::string rank_name;   // û�з���Ĭ���������� (δ�ϰ�ʱ�İ�����)
	};
    virtual void set_popular_rank_info(const PopularRankInfo& info) = 0;
    virtual const PopularRankInfo& get_popular_rank_info() = 0;


};

}   // namespace secret

#endif  // BILILIVE_SECRET_PUBLIC_ANCHOR_INFO_H_
