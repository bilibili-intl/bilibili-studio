#ifndef OBS_TEXTURE_RENDERER_TEXTURE_SERVICE_H_
#define OBS_TEXTURE_RENDERER_TEXTURE_SERVICE_H_

#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>


struct obs_source_frame;

#ifdef OBSTEXTURERENDERER_EXPORTS
#define DLLIMPORT __declspec(dllexport)
#else
#define DLLIMPORT __declspec(dllimport)
#endif


namespace livehime {

    enum class TextureType {
        DanmakuHime,
        Vote,
    };

    /**
     * 下发纹理的接口。
     * 纹理源实现该接口，接收下发的纹理并显示。
     */
    class ITextureDispatcher {
    public:
        virtual ~ITextureDispatcher() = default;

        /**
         * 当有新的纹理到来时会触发该接口。
         */
        virtual void OnTexture(const obs_source_frame* frame) = 0;
    };


    /**
     * 纹理源通知上层时使用的接口。
     * 纹理源会在某些事件发生时通知上层。每个纹理源均通过这个接口进行通知。
     * 当前未区分具体是哪个纹理源通知的。
     */
    class ITextureServiceCallback :
        public std::enable_shared_from_this<ITextureServiceCallback> {
    public:
        virtual ~ITextureServiceCallback() = default;

        /**
         * 通知上层：需要往底层喂纹理，因为要么有新添加的纹理源，要么
         * 存在纹理源由非激活状态转为激活状态。
         *
         * {first} 参数可用来区分由上次自闭状态之后，是否是第一次要求喂食。
         */
        virtual void OnRefreshTexture(bool first) = 0;

        /**
         * 通知上层：之后无需再喂纹理，因为要么底层已不存在纹理源，要么
         * 所有纹理源均处于非激活状态。
         */
        virtual void OnAutism() = 0;
    };


    /**
     * 纹理源服务，为上层提供管理多个纹理源的接口。
     * 该对象实例全局唯一，在需要时会被创建，管理所有纹理源。
     */
    class ITextureService {
    public:
        using CallbackPtr = std::shared_ptr<ITextureServiceCallback>;
        using CallbackWPtr = std::weak_ptr<ITextureServiceCallback>;
        using VideoFramePtr = std::shared_ptr<obs_source_frame>;

        virtual ~ITextureService() = default;

        /**
         * 安装回调，开启工作线程
         */
        virtual bool Launch(const CallbackPtr& callback) = 0;

        /**
         * 判断当前是否存在接收纹理的实体。
         * （实现 ITextureDispatcher 接口的对象，即纹理源）
         */
        virtual bool HasDispatcher() = 0;

        /**
         * 判断当前是否存在已激活的纹理源。
         */
        virtual bool HasActivatedSource() = 0;

        /**
         * 将帧数据（位图数据）送至底层下发。
         * 该数据将被下发至每一个实现 IDmkhimeTextureDispatcher 接口的对象，即纹理源。
         * 无论底层是否存在纹理源，也无论纹理源是否激活，调用该方法后都会尝试下发。
         */
        virtual void Dispatch(const VideoFramePtr& frame, TextureType type, int id) = 0;
    };


    class ITextureServiceFactory {
    public:
        virtual ~ITextureServiceFactory() = default;

        /**
         * 获取 ITextureService 对象。
         * 第一次调用时会创建对象。
         */
        virtual std::shared_ptr<ITextureService> Fetch() = 0;
    };

}

extern "C"
{
    DLLIMPORT livehime::ITextureServiceFactory* GetTextureServiceFactory();
};

#endif  // OBS_TEXTURE_RENDERER_TEXTURE_SERVICE_H_