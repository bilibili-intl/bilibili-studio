// Copyright (c) 2020 ucclkp <ucclkp@gmail.com>.
// This file is part of media project.
//
// This program is licensed under GPLv3 license that can be
// found in the LICENSE file.

#ifndef MEDIA_ISO_ISO_MEDIA_PARSER_H_
#define MEDIA_ISO_ISO_MEDIA_PARSER_H_

#include <string>
#include <fstream>
#include <vector>


namespace media {
namespace iso {

    class Box;

    /**
     * ISO_IEC_14496-12_2015
     */
    class ISOMediaParser {
    public:
        explicit ISOMediaParser(std::istream& s);

        bool parse();

        const std::vector<std::shared_ptr<Box>>& getBoxes() const;

    private:
        std::istream& iso_file_;
        std::vector<std::shared_ptr<Box>> boxes_;
    };

}
}

#endif  // MEDIA_ISO_ISO_MEDIA_PARSER_H_