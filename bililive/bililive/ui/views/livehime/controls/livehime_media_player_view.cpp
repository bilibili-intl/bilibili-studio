//#include "livehime_media_player_view.h"
//
//#include "bililive/common/bililive_logging.h"
//#include "ui/base/resource/resource_bundle.h"
//
//
//LivehimeMediaPlayerView::LivehimeMediaPlayerView()
//{
//    InitView();
//}
//
//LivehimeMediaPlayerView::~LivehimeMediaPlayerView()
//{
//}
//
//void LivehimeMediaPlayerView::SetResourceID(uint32_t svga_file_id, uint32_t mp4_file_id, bool enabled, double percentage)
//{
//    enabled_ = enabled;
//    percentage_ = percentage;
//
//    auto &rb = ui::ResourceBundle::GetSharedInstance();
//
//    svga_data_ = rb.GetRawDataResource(svga_file_id).as_string();
//    auto mp4_data = rb.GetRawDataResource(mp4_file_id).as_string();
//
//    svga_id_ = svga_file_id;
//    mp4_play_view_->SetMp4Buffer(mp4_data.data(), mp4_data.size());
//}
//
//void LivehimeMediaPlayerView::Play()
//{
//    if (use_mp4_)
//    {
//        mp4_play_view_->Play();
//        LOG(INFO) << "LivehimeMediaPlayerView:: using mp4";
//    }
//    else
//    {
//        auto dir_path = bililive::GetBililiveUserDataDirectory();
//        if (!dir_path.empty())
//        {
//            dir_path = dir_path.Append(L"Cache").Append(L"Motions").Append(std::to_wstring(svga_id_));
//        }
//
//        svga_play_view_->setSVGAData(svga_data_, dir_path.AsUTF16Unsafe());
//        svga_play_view_->prepareToPercentage(enabled_, percentage_);
//
//        LOG(INFO) << "LivehimeMediaPlayerView:: using svga";
//    }
//}
//
//void LivehimeMediaPlayerView::Stop()
//{
//    if (use_mp4_)
//    {
//        mp4_play_view_->Destroy();
//    }
//    else
//    {
//        svga_play_view_->stopParsing();
//        svga_play_view_->stopAnimation(true);
//    }
//}
//
//void LivehimeMediaPlayerView::SetPreferredSize(const gfx::Size& size)
//{
//    preferred_size_ = size;
//}
//
//gfx::Size LivehimeMediaPlayerView::GetPreferredSize()
//{
//    return preferred_size_;
//}
//
//void LivehimeMediaPlayerView::Layout()
//{
//    auto rt = GetContentsBounds();
//    if (use_mp4_)
//    {
//        mp4_play_view_->SetVisible(true);
//        mp4_play_view_->SetBoundsRect(rt);
//        svga_play_view_->SetVisible(false);
//    }
//    else
//    {
//        svga_play_view_->SetVisible(true);
//        svga_play_view_->SetBoundsRect(rt);
//        mp4_play_view_->SetVisible(false);
//    }
//}
//
//void LivehimeMediaPlayerView::OnPaint(gfx::Canvas* canvas)
//{
//    __super::OnPaint(canvas);
//
//    if (use_mp4_)
//    {
//        mp4_play_view_->OnPaint(canvas);
//    }
//    else
//    {
//        svga_play_view_->OnPaint(canvas);
//    }
//}
//
//void LivehimeMediaPlayerView::InitView()
//{
//    mp4_play_view_ = new LivehimeMp4PlayerView();
//    svga_play_view_ = new LivehimeSVGAImageView();
//
//    use_mp4_ = mp4_play_view_->IsSupportMp4();
//
//    AddChildView(mp4_play_view_);
//    AddChildView(svga_play_view_);
//}
