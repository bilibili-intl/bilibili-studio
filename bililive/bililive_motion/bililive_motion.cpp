#include "bililive_motion.h"

#include "mp4_motion.h"


livehime::motion::MP4Motion* CreateBililiveMotion(livehime::motion::Mp4PlayerCallback* callback) {
    return new livehime::motion::MP4MotionImpl(callback);
}