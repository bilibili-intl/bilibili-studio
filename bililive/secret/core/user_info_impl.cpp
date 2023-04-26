#include "bililive/secret/core/user_info_impl.h"

namespace secret {

UserInfoImpl::UserInfoImpl(){
}

const std::string& UserInfoImpl::nickname() const {
    return nickname_;
}

const AvatarData& UserInfoImpl::avatar() const {
    return avatar_;
}

bool UserInfoImpl::nft() const
{
    return nft_;
}

void UserInfoImpl::set_nft(bool nft)
{
    nft_ = nft;
}

void UserInfoImpl::set_avatar(const unsigned char* data, size_t size) {
    avatar_.assign(data, data + size);
}

void UserInfoImpl::set_nickname(const std::string& name) {
    nickname_ = name;
}

bool UserInfoImpl::nft()
{
    return nft_;
}

int64_t UserInfoImpl::san() const {
    return san_;
}

void UserInfoImpl::set_san(int64_t san) {
    san_ = san;
}

int64_t UserInfoImpl::room_id() const {
    return room_id_;
}

void UserInfoImpl::set_room_id(int64_t room_id) {
    room_id_ = room_id;
}

}   // namespace secret
