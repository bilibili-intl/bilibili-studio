#include "bililive/secret/services/json_parse.h"

const std::set<std::string> log_interface_blacklist{
    // 大乱斗轮询接口
    "battle-info",
    // 未读主播消息每分钟轮询接口
    "livemsg-unread-count",
    // 连麦 PK 轮询接口
    "get-pk-status",
    // 游戏PK轮询接口
    "game-pk-status",
    // 热力风暴轮询接口
    "get-storm-result",
    //视频PK轮询接口
    "video-pk-info",
    // 语聊房状态同步
    "sync-chat-seat-status"
};