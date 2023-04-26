#include "bililive/secret/core/anchor_info_impl.h"

namespace secret {

AnchorInfoImpl::AnchorInfoImpl()
    : room_level_(0),
      room_gift_(0),
      room_fans_(0),
      current_area_(-1),
      current_xp_(0),
      max_xp_(0),
      cdn_src_(-1),
      cdn_checked_(1),
      medal_status_(0),
      guard_count_(0),
      tag_id_(0){
}

const std::string& AnchorInfoImpl::room_title() const {
    return room_title_;
}

void AnchorInfoImpl::set_room_title(const std::string& title) {
    room_title_ = title;
}

const std::string& AnchorInfoImpl::room_cover() const {
    return room_cover_;
}

void AnchorInfoImpl::set_room_cover(const std::string& cover) {
    room_cover_ = cover;
}

const std::string& AnchorInfoImpl::vert_room_cover() const
{
    return vert_room_cover_;
}

void AnchorInfoImpl::set_vert_room_conver(const std::string& title)
{
    vert_room_cover_ = title;
}

void AnchorInfoImpl::set_enable_vert_cover(bool enable)
{
    enable_vert_room_cover_ = enable;
}

bool AnchorInfoImpl::enable_vert_cover()
{
    return enable_vert_room_cover_;
}

int AnchorInfoImpl::room_level() const {
    return room_level_;
}

void AnchorInfoImpl::set_room_level(int level) {
    room_level_ = level;
}

int64_t AnchorInfoImpl::room_gift() const {
    return room_gift_;
}

void AnchorInfoImpl::set_room_gift(int64_t gift) {
    room_gift_ = gift;
}

int64_t AnchorInfoImpl::room_fans() const {
    return room_fans_;
}

void AnchorInfoImpl::set_room_fans(int64_t fans) {
    room_fans_ = fans;
}

int AnchorInfoImpl::current_area() const {
    return current_area_;
}

void AnchorInfoImpl::set_current_area(int area) {
    current_area_ = area;
}

int64_t AnchorInfoImpl::current_xp() const {
    return current_xp_;
}

void AnchorInfoImpl::set_current_xp(int64_t current_xp) {
    current_xp_ = current_xp;
}

int64_t AnchorInfoImpl::max_xp() const {
    return max_xp_;
}

void AnchorInfoImpl::set_max_xp(int64_t max_xp) {
    max_xp_ = max_xp;
}

int AnchorInfoImpl::cdn_src() {
    return cdn_src_;
}

int AnchorInfoImpl::cdn_checked() {
    return cdn_checked_;
}

void AnchorInfoImpl::set_cdn_src(int src) {
    cdn_src_ = src;
}

void AnchorInfoImpl::set_cdn_checked(int checked) {
    cdn_checked_ = checked;
}

void AnchorInfoImpl::set_recommend_ip(const std::string& ip)
{
    recommend_ip_ = ip;
}

const std::string& AnchorInfoImpl::recommend_ip() const
{
    return recommend_ip_;
}

const std::string& AnchorInfoImpl::current_area_name() const {
    return area_name_;
}

void AnchorInfoImpl::set_current_area_name(const std::string& area_name) {
    area_name_ = area_name;
}

const std::string& AnchorInfoImpl::current_parent_area_name() const {
    return area_parent_name_;
}

void AnchorInfoImpl::set_current_parent_area_name(const std::string& area_name) {
    area_parent_name_ = area_name;
}

int AnchorInfoImpl::current_parent_area() const
{
    return current_parent_area_;
}

void AnchorInfoImpl::set_current_parent_area(int area)
{
    current_parent_area_ = area;
}

int64_t AnchorInfoImpl::medal_status() const {
    return medal_status_;
}

void AnchorInfoImpl::set_medal_status(int64_t medal_status) {
    medal_status_ = medal_status;
}

const std::string& AnchorInfoImpl::medal_name() const {
    return medal_name_;
}

void AnchorInfoImpl::set_medal_name(const std::string& medal_name) {
    medal_name_ = medal_name;
}

int64_t AnchorInfoImpl::guard_count() const {
    return guard_count_;
}

void AnchorInfoImpl::set_guard_count(int64_t guard_count) {
    guard_count_ = guard_count;
}

void AnchorInfoImpl::set_tag_info(int64_t tag_id, const std::string& tag_name)
{
    tag_id_ = tag_id;
    tag_name_ = tag_name;
}

void AnchorInfoImpl::set_join_slide(int status)
{
    join_slide_ = status;
}

int AnchorInfoImpl::get_join_slide()
{
    return join_slide_;
}

void AnchorInfoImpl::set_medal_fans(int64_t fans) {
    medal_fans_ = fans;
}

int64_t AnchorInfoImpl::get_medal_fans() {
    return medal_fans_;
}

void AnchorInfoImpl::set_land_scape_model(bool land_scape_model)
{
    land_scape_model_ = land_scape_model;
}

bool AnchorInfoImpl::get_land_scape_model() {
    return land_scape_model_;
}

void AnchorInfoImpl::set_identify_status(int identify_status)
{
    identify_status_ = identify_status;
}

int AnchorInfoImpl::identify_status()
{
    return identify_status_;
}

void AnchorInfoImpl::set_is_adult(bool is_adult)
{
    is_adult_ = is_adult;
}

//bool AnchorInfoImpl::is_adult()
//{
//    return is_adult_;
//}

void AnchorInfoImpl::set_have_live(int have_live)
{
    have_live_ = have_live;
}

int AnchorInfoImpl::have_live()
{
    return have_live_;
}

}   // namespace secret
