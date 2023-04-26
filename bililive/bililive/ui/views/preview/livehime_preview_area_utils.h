#ifndef BILILIVE_BILILIVE_UI_VIEWS_PREVIEW_PREVIEW_AREA_UTILS_H
#define BILILIVE_BILILIVE_UI_VIEWS_PREVIEW_PREVIEW_AREA_UTILS_H

#include "ui/views/view.h"



namespace livehime
{
    enum PreviewFocusLessFloatingWidgetHierarchy
    {
        PFLFWH_First,
        PFLFWH_Second,
        PFLFWH_Third,
        PFLFWH_Fourth,
        /*PFLFWH_Fourth,
        PFLFWH_Fifth,*/

        PFLFWH_Count,
    };

    enum PreviewAcceptEventFloatingWidgetHierarchy
    {
        PAEFWH_First,
        PAEFWH_Third_Party,
        PAEFWH_VoiceLinkApply,
        PFLFWH_ChatRoom,
        /*PAEFWH_Third,
        PAEFWH_Fourth,
        PAEFWH_Fifth,*/

        PAEFWH_Count,
    };

    int ShowPreviewArea(PreviewFocusLessFloatingWidgetHierarchy hierarchy, views::View* content_view);
    void HidePreviewArea(PreviewFocusLessFloatingWidgetHierarchy hierarchy, int seque_id, bool delete_content_view);

    int ShowAcceptEventPreviewArea(PreviewAcceptEventFloatingWidgetHierarchy hierarchy, views::View* content_view, void *floating_view_delegate = nullptr);
    void HideAcceptEventPreviewArea(PreviewAcceptEventFloatingWidgetHierarchy hierarchy, int seque_id, bool delete_content_view);
}

#endif