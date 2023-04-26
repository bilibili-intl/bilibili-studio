// Copyright (c) 2020 ucclkp <ucclkp@gmail.com>.
// This file is part of media project.
//
// This program is licensed under GPLv3 license that can be
// found in the LICENSE file.

#ifndef MEDIA_ISO_BOX_ISO_BOX_H_
#define MEDIA_ISO_BOX_ISO_BOX_H_

#include <string>
#include <functional>
#include <memory>
#include <vector>


namespace media {
namespace iso {

    class Box {
    public:
        virtual ~Box() = default;

        uint32_t getBoxFieldSize() const;
        const std::vector<std::shared_ptr<Box>>& getChildren() const;

        virtual bool parse(std::istream& s);
        virtual bool isContainer() const;
        virtual std::string toString() const;

        uint64_t size;
        uint32_t type;

        // if type == 'uuid'
        uint8_t user_type[16];

    protected:
        bool parseContainer(
            std::istream& s, std::function<bool(const Box&)>&& f);

        std::vector<std::shared_ptr<Box>> children_;

    private:
        uint32_t box_field_size_ = 0;
    };


    class FullBox : public Box {
    public:
        explicit FullBox(const Box& b);

        bool parse(std::istream& s) override;

        uint8_t version;
        uint32_t flags;
    };

}
}

#endif  // MEDIA_ISO_BOX_ISO_BOX_H_