#include "bililive/secret/services/json_parse.h"

const std::set<std::string> log_interface_blacklist{
    // ���Ҷ���ѯ�ӿ�
    "battle-info",
    // δ��������Ϣÿ������ѯ�ӿ�
    "livemsg-unread-count",
    // ���� PK ��ѯ�ӿ�
    "get-pk-status",
    // ��ϷPK��ѯ�ӿ�
    "game-pk-status",
    // �����籩��ѯ�ӿ�
    "get-storm-result",
    //��ƵPK��ѯ�ӿ�
    "video-pk-info",
    // ���ķ�״̬ͬ��
    "sync-chat-seat-status"
};