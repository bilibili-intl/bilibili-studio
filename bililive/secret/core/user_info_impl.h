
#ifndef BILILIVE_SECRET_CORE_USER_INFO_IMPL_H_
#define BILILIVE_SECRET_CORE_USER_INFO_IMPL_H_

#include "bililive/secret/public/user_info.h"

namespace secret {

class UserInfoImpl : public UserInfo {

public:
    UserInfoImpl();

    ~UserInfoImpl() = default;

    const std::string& nickname() const override;

    void set_nickname(const std::string& name) override;

    void set_nft(bool nft) override;

    bool nft() override;

    const AvatarData& avatar() const override;

    bool nft() const override;

    void set_avatar(const unsigned char *data, size_t size) override;

    int64_t san() const override;

    void set_san(int64_t san) override;

    int64_t room_id() const override;

    void set_room_id(int64_t room_id) override;

private:
    std::string nickname_;
    bool nft_ = false;
    AvatarData avatar_;
    int64_t san_ = 0;
    int64_t room_id_ = kInvalidRoomID;
};

}   // namespace secret

#endif  // BILILIVE_SECRET_CORE_USER_INFO_IMPL_H_
