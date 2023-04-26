#ifndef BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_GAME_CONNECTION_GAME_CONN_TYPES_H_
#define BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_GAME_CONNECTION_GAME_CONN_TYPES_H_


namespace livehime {
namespace game {

    enum class PagedListStatus {
        Loading,
        Failed,
        Succeeded,
        Bottom
    };

    enum class GameInviteStatus {
        InviteOk =1,        //��������
        NotLive,            //δ����
        GameLeave,          //��Ϸδ����
        GamePking,          //��ϷPk��
    };

    struct GameCandidateData {
        int64_t uid = 0;
        int64_t room_id = 0;
        std::string avatar;
        string16 uname;
        string16 part_name;
        int64_t audience;
        int64_t fans;
        GameInviteStatus invite_type;

    };
}
}

#endif  // BILILIVE_BILILIVE_UI_VIEWS_LIVEHIME_GAME_CONNECTION_GAME_CONN_TYPES_H_