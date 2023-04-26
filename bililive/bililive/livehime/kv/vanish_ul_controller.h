#ifndef BILILIVE_BILILIVE_LIVEHIME_KV_VANISH_UL_CONTROLLER_H_
#define BILILIVE_BILILIVE_LIVEHIME_KV_VANISH_UL_CONTROLLER_H_


namespace livehime {

    class VanishULController {
    public:
        static void RequestULVanishingStatus();
        static bool IsULVanished();

    private:
        static void OnGetULVanishingSwitch(
            bool valid, int code, const std::string& err_msg, bool vanished);

        static bool ul_vanished_;
    };

}

#endif  // BILILIVE_BILILIVE_LIVEHIME_KV_VANISH_UL_CONTROLLER_H_