#ifdef TEST_URL_IMG_VIEW_H
#error
#endif
#define TEST_URL_IMG_VIEW_H

#include "base/bind.h"
#include "base/message_loop/message_loop.h"
#include "base/time/time.h"

#include "ui/views/background.h"
#include "ui/views/controls/button/label_button.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/widget/widget.h"


#include "bililive_url_image_view.h"
#include "bililive_url_img_fetch_globals.h"
#include "ui/views/widget/widget_delegate.h"

using namespace views;


static View *url_views[30];
//static BililiveUrlImageView *url_views[30];
//static char const *urls[] = {
//    "http://attach.bbs.miui.com/forum/201409/09/130454p88fyyufmunq8hiu.jpg",
//    "http://att.bbs.duowan.com/forum/201408/23/0002517imynykzym38003q.jpg",
//    "http://img.newyx.net/article/image/201512/17/1a7dae25cc.jpg",
//    "http://www.czechtickle.sexy/i/TICKLING-SUBMISSION%20-%20Tickle%20lickle%20action%20Shelly%20%281%29.jpg",
//    "http://image99.360doc.com/DownloadImg/2016/08/3118/79098393_3.jpg",
//
//    "http://image99.360doc.com/DownloadImg/2016/08/3118/79098420_9.jpg",
//    "http://h9.86.cc/walls/20151221/1920x1080_dc8c93d1f8d7f5b.jpg",
//    "http://www.bz55.com/uploads/allimg/150616/140-1506160Z343.jpg",
//    "http://image99.360doc.com/DownloadImg/2016/08/3118/79098420_12.jpg",
//    "http://image99.360doc.com/DownloadImg/2016/08/3118/79098393_10.jpg",
//
//    "http://image99.360doc.com/DownloadImg/2016/08/3118/79098393_6.jpg",
//    "http://attach.bbs.miui.com/forum/201401/24/105030ttsw9i8i6wwf8idi.jpg",
//    "http://image99.360doc.com/DownloadImg/2016/08/3115/79089846_13.jpg",
//    "http://pic1.win4000.com/wallpaper/8/57184a8f6e6f2.jpg",
//    "http://img15.3lian.com/2015/f2/45/d/13.jpg",
//
//    "http://image99.360doc.com/DownloadImg/2016/08/3115/79089846_1.jpg",
//    "http://www.bz55.com/uploads/allimg/130826/1-130R60U240.jpg",
//    "http://file.mumayi.com/forum/201405/11/155732gyga02pmjepra9ie.jpg",
//    "http://image99.360doc.com/DownloadImg/2016/08/3118/79098420_3.jpg",
//    "http://www.bizhitupian.com/uploadfile/2013/0217/20130217105517274.jpg",
//
//    "http://image99.360doc.com/DownloadImg/2016/08/3118/79098393_1.jpg",
//    "http://www.bz55.com/uploads/allimg/150616/140-1506160Z341.jpg",
//    "http://image99.360doc.com/DownloadImg/2016/08/3115/79089846_5.jpg",
//    "http://image99.360doc.com/DownloadImg/2016/08/3118/79098420_8.jpg",
//    "http://file.mumayi.com/forum/201405/11/155733ll4kjgrjjjjjjjvz.jpg",
//
//    "http://www.bz55.com/uploads/allimg/130826/1-130R60U246.jpg",
//    "http://www.bz55.com/uploads/allimg/150616/140-1506160Z345.jpg",
//    "http://file.mumayi.com/forum/201405/11/155736dbxvawaajalse3my.jpg",
//    "http://file.mumayi.com/forum/201405/11/155737m3mekmp3rmxec9qc.jpg",
//    "http://pic.dd008.com/source/201509/24/b6d033189c.jpg"
//};

static char const *urls[] = {
    "http://i0.hdslb.com/bfs/archive/3063baf59b998e7011018047487f836fa8cbf826.jpg_205x128.jpg",
    "http://i1.hdslb.com/bfs/archive/19367d2b31abee9f3bbe7ee85ea7e20a332b5a08.jpg_205x128.jpg",
    "http://i2.hdslb.com/bfs/archive/15b38c2bd0a3cbb2fd765d3f352926534666e747.jpg_205x128.jpg",
    "http://i0.hdslb.com/bfs/archive/c2b7d5e2e7d6f9bdb83af1b6527822f797547bc2.jpg_205x128.jpg",
    "http://i2.hdslb.com/bfs/archive/5ccc30cdd024b47ab14952e6aaf3776fb73c4117.jpg_205x128.jpg",

    "http://i1.hdslb.com/bfs/archive/ac681d854e066ce4a7b139733f1aae47499c098f.jpg_205x128.jpg",
    "http://i0.hdslb.com/bfs/archive/e9cc78ae42ae0a52d90e42b93853a0a217388528.jpg_205x128.jpg",
    "http://i1.hdslb.com/bfs/archive/ad6ff4c6313f0fbe873148c17c393c3dddc3e875.jpg_205x128.jpg",
    "http://i0.hdslb.com/bfs/bangumi/a3ed558aa5ba0d3d914f9f80d6a429abdab36121.jpg_260x246.jpg",
    "http://i0.hdslb.com/bfs/bangumi/6c5a3687a0ce0d6ed612092594486ecf46612b6d.jpg_260x302.jpg",

    "http://i0.hdslb.com/bfs/bangumi/32dd5db651270a6cec33228a73cf62562053ac2c.png_160x210.png",
    "http://i0.hdslb.com/bfs/bangumi/ddc86b191128d978aa13103744caed34715eb83f.png_160x210.png",
    "http://i0.hdslb.com/bfs/bangumi/4b74b5b0f489956e633b3fef15dda49dcf47921b.jpg_160x210.jpg",
    "http://i0.hdslb.com/bfs/bangumi/695227e850708b7b9ae0940b816dbb04a8d6ce07.jpg_160x210.jpg",
    "http://i0.hdslb.com/bfs/bangumi/1af09ef72a34df3c569bd4b83baa9b535afed87c.jpg_160x210.jpg",

    "http://i0.hdslb.com/bfs/bangumi/aa8aba574cedb639dad6cfcb9d2ad0eb08a9b03b.png_160x210.png",
    "http://i0.hdslb.com/bfs/bangumi/162d30b0d5e686269ad1d4f65bbeaa9a40ae4d68.jpg_160x210.jpg",
    "http://i0.hdslb.com/bfs/bangumi/45cdcdebf89fbfcd3f2a6d3eb6eaa805dd86298c.jpg_160x210.jpg",
    "http://i0.hdslb.com/bfs/bangumi/b832323839a75634073f2b77573f9a22be03f206.jpg_160x210.jpg",
    "http://i0.hdslb.com/bfs/bangumi/3195b96f13d35bafdcf45c2ab7a8ba5d6cc04be4.png_160x210.png",

    "http://i0.hdslb.com/bfs/bangumi/e712cdcbac0508e37704346d763b2781d08e21d9.jpg_160x210.jpg",
    "http://i0.hdslb.com/bfs/bangumi/80053e7dc3101577eb47c296b2bd117d8d32270f.jpg_160x210.jpg",
    "http://i0.hdslb.com/bfs/bangumi/f9930fa4efcca2ffa13889aa2a77593f3824f97f.jpg_160x210.jpg",
    "http://i0.hdslb.com/bfs/bangumi/493e73ec3069a669e96ad9051883e6d3b6363656.jpg_160x210.jpg",
    "http://i0.hdslb.com/bfs/bangumi/89d13d5b6ada2f7a67bbb6d380f13b25c8f5f0bb.jpg_160x210.jpg",

    "http://i0.hdslb.com/320_200/video/7f/7f6070b06456b4c79abd86e34111c975.jpg",
    "http://i1.hdslb.com/320_200/u_user/4687a1f725b53fd2519f2d2b4ad030ed.jpg",
    "http://i2.hdslb.com/320_200/video/ae/aeab07d028ab963888e2fd0e2ba46f0c.jpg",
    "http://i0.hdslb.com/320_200/video/44/445b65c6123663bd21cb8ca0824793a4.jpg",
    "http://i2.hdslb.com/320_200/video/d4/d47158d4df5b14de56ed409e6e2a44a2.jpg"
};

static void DelayTask() 
{
    static bool enable = false;
    static int count = 0;
    for (int i = 0; i < 15; i++) {
        ((BililiveUrlImageView*)url_views[(i + count) % 30])->SetImageUrl(std::string(urls[i]));
    }

    count++;
}


class TestView 
        : public WidgetDelegateView,
        public ButtonListener {
public:
    TestView()
    {
        set_background(Background::CreateSolidBackground(188, 188, 188, 255));

        views::GridLayout *layout = new GridLayout(this);
        

        views::ColumnSet *col_set = layout->AddColumnSet(0);
        col_set->AddColumn(GridLayout::FILL, GridLayout::FILL, 1, GridLayout::USE_PREF, 0, 0);
        col_set->AddColumn(GridLayout::FILL, GridLayout::FILL, 1, GridLayout::USE_PREF, 0, 0);
        col_set->AddColumn(GridLayout::FILL, GridLayout::FILL, 1, GridLayout::USE_PREF, 0, 0);
        col_set->AddColumn(GridLayout::FILL, GridLayout::FILL, 1, GridLayout::USE_PREF, 0, 0);
        col_set->AddColumn(GridLayout::FILL, GridLayout::FILL, 1, GridLayout::USE_PREF, 0, 0);
        col_set->AddColumn(GridLayout::FILL, GridLayout::FILL, 1, GridLayout::USE_PREF, 0, 0);


        col_set = layout->AddColumnSet(1);
        col_set->AddColumn(GridLayout::LEADING, GridLayout::LEADING, 1, GridLayout::USE_PREF, 0, 0);

        layout->StartRow(0, 1);
        layout->AddView(new LabelButton(this, L"X"));

        for (int i = 0; i < 5; i++) {
            layout->StartRow(1, 0);
            for (int j = 0; j < 6; j++) {
                View *v = url_views[i * 6 + j] = new BililiveUrlImageView(NULL);
                v->set_border(Border::CreateSolidBorder(1, SK_ColorRED));
                layout->AddView(v);
            }
        }

        SetLayoutManager(layout);


        Widget *wgt = new Widget();
        views::Widget::InitParams params(views::Widget::InitParams::TYPE_WINDOW);
        params.parent = 0;
        params.delegate = this;
        params.opacity = Widget::InitParams::INFER_OPACITY;
        params.remove_standard_frame = true;
        wgt->Init(params);

        wgt->SetBounds(gfx::Rect(50, 50, 1200, 1010));

        wgt->Show();


        base::MessageLoop::current()->PostDelayedTask(
            FROM_HERE,
            base::Bind(DelayTask),
            base::TimeDelta::FromSeconds(5));

        /*base::MessageLoop::current()->PostDelayedTask(
            FROM_HERE,
            base::Bind(DelayTask),
            base::TimeDelta::FromSeconds(15));*/

        //base::MessageLoop::current()->PostDelayedTask(
        //    FROM_HERE,
        //    base::Bind(DelayTask),
        //    base::TimeDelta::FromSeconds(20));

        //base::MessageLoop::current()->PostDelayedTask(
        //    FROM_HERE,
        //    base::Bind(DelayTask),
        //    base::TimeDelta::FromSeconds(30));

        //base::MessageLoop::current()->PostDelayedTask(
        //    FROM_HERE,
        //    base::Bind(DelayTask),
        //    base::TimeDelta::FromSeconds(40));

        //base::MessageLoop::current()->PostDelayedTask(
        //    FROM_HERE,
        //    base::Bind(DelayTask),
        //    base::TimeDelta::FromSeconds(50));

        //base::MessageLoop::current()->PostDelayedTask(
        //    FROM_HERE,
        //    base::Bind(DelayTask),
        //    base::TimeDelta::FromSeconds(60));

        //base::MessageLoop::current()->PostTask(
        //    FROM_HERE,
        //    base::Bind(DelayTask));

        bililive_main_dll::UrlImgFetchInit();
    }
    ~TestView()
    {
        base::MessageLoop::current()->PostTask(
                FROM_HERE,
                base::Bind(bililive_main_dll::UrlImgFetchUnInit));
    }

    views::View* GetContentsView() OVERRIDE
    {
        return this;
    }



    void ButtonPressed(Button* sender, const ui::Event& event) override {
        GetWidget()->Close();
    }
};