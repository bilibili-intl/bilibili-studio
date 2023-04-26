//#pragma once
//
//#include "bililive/bililive/ui/views/livehime/controls/livehime_mp4_player_viewer.h"
//#include "bililive/bililive/ui/views/livehime/controls/livehime_svga_image_view.h"
//
//class LivehimeMediaPlayerView :
//    public views::View
//{
//public:
//    LivehimeMediaPlayerView();
//    ~LivehimeMediaPlayerView();
//
//    void SetResourceID(uint32_t svga_file_id, uint32_t mp4_file_id, bool enabled, double percentage);
//    void Play();
//    void Stop();
//
//    void SetPreferredSize(const gfx::Size& size);
//    gfx::Size GetPreferredSize() override;
//
//protected:
//    void Layout() override;
//    void OnPaint(gfx::Canvas* canvas) override;
//
//private:
//    bool                        use_mp4_ = false;
//    bool                        enabled_ = false;
//    double                      percentage_ = 0.0;
//    LivehimeMp4PlayerView*      mp4_play_view_ = nullptr;
//    LivehimeSVGAImageView*      svga_play_view_ = nullptr;
//
//    uint32_t                    svga_id_ = {};
//    std::string                 svga_data_;
//
//    gfx::Size                   preferred_size_;
//
//    void InitView();
//};
