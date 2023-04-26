/*
 @ 0xCCCCCCCC
*/

#ifndef BILILIVE_SECRET_PUBLIC_USER_INFO_H_
#define BILILIVE_SECRET_PUBLIC_USER_INFO_H_

#include <string>
#include <vector>

namespace secret {

using AvatarData = std::vector<unsigned char>;

class UserInfo {
public:
    virtual ~UserInfo() {}

    static const int kInvalidRoomID = -1;

    virtual const std::string& nickname() const = 0;

    virtual void set_nickname(const std::string& name) = 0;

    virtual void set_nft(bool nft) = 0;

    virtual bool nft() = 0;

    virtual const AvatarData& avatar() const = 0;

    virtual void set_avatar(const unsigned char* data, size_t size) = 0;

    virtual bool nft() const = 0;

    virtual int64_t san() const = 0;

    virtual void set_san(int64_t san) = 0;

    virtual int64_t room_id() const = 0;

    virtual void set_room_id(int64_t room_id) = 0;
};

}   // namespace secret

#endif  // BILILIVE_SECRET_PUBLIC_USER_INFO_H_
