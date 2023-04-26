#ifndef OBS_DMKHIME_DMKHIME_SERVICE_H_
#define OBS_DMKHIME_DMKHIME_SERVICE_H_

#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>


struct obs_source_frame;

#ifdef OBSDMKHIME_EXPORTS
#define DLLIMPORT __declspec(dllexport)
#else
#define DLLIMPORT __declspec(dllimport)
#endif


namespace bililive {

    /**
     * 下发弹幕姬纹理的接口。
     * 弹幕姬源实现该接口，接收下发的纹理并显示。
     */
    class IDmkhimeTextureDispatcher {
    public:
        virtual ~IDmkhimeTextureDispatcher() = default;

        /**
         * 当有新的弹幕姬纹理时会触发该接口。
         */
        virtual void OnTexture(const obs_source_frame* frame) = 0;
    };


    /**
     * 弹幕姬源通知上层时使用的接口。
     * 弹幕姬源会在某些事件发生时通知上层。每个弹幕姬源均通过这个接口进行通知。
     * 当前未区分具体是哪个弹幕姬源通知的。
     */
    class IDmkhimeTexServiceCallback :
        public std::enable_shared_from_this<IDmkhimeTexServiceCallback> {
    public:
        virtual ~IDmkhimeTexServiceCallback() = default;

        /**
         * 通知上层：需要往底层喂弹幕姬纹理，因为要么有新添加的弹幕姬源，要么
         * 存在弹幕姬源由非激活状态转为激活状态。
         *
         * {first} 参数可用来区分由上次自闭状态之后，是否是第一次要求喂食。
         */
        virtual void OnRefreshTexture(bool first) = 0;

        /**
         * 通知上层：之后无需再喂弹幕姬纹理，因为要么底层已不存在弹幕姬源，要么
         * 所有弹幕姬源均处于非激活状态。
         */
        virtual void OnAutism() = 0;
    };


    /**
     * 弹幕姬源服务，为上层提供管理多个弹幕姬源的接口。
     * 该对象实例全局唯一，在需要时会被创建，管理所有弹幕姬源。
     */
    class IDmkhimeTexService {
    public:
        using CallbackPtr = std::shared_ptr<IDmkhimeTexServiceCallback>;
        using CallbackWPtr = std::weak_ptr<IDmkhimeTexServiceCallback>;
        using VideoFramePtr = std::shared_ptr<obs_source_frame>;

        virtual ~IDmkhimeTexService() = default;

        /**
         * 安装回调，开启工作线程
         */
        virtual bool Launch(const CallbackPtr& callback) = 0;

        /**
         * 判断当前是否存在接收纹理的实体。
         * （实现 IDmkhimeTextureDispatcher 接口的对象，即弹幕姬源）
         */
        virtual bool HasDispatcher() = 0;

        /**
         * 判断当前是否存在已激活的弹幕姬源。
         */
        virtual bool HasActivatedSource() = 0;

        /**
         * 将弹幕姬的帧数据（位图数据）送至底层下发。
         * 该数据将被下发至每一个实现 IDmkhimeTextureDispatcher 接口的对象，即弹幕姬源。
         * 无论底层是否存在弹幕姬源，也无论弹幕姬源是否激活，调用该方法后都会尝试下发。
         */
        virtual void Dispatch(const VideoFramePtr& frame) = 0;
    };


    class IDmkhimeServiceFactory {
    public:
        virtual ~IDmkhimeServiceFactory() = default;

        /**
         * 获取 IDmkhimeTexService 对象。
         */
        virtual IDmkhimeTexService* Get() = 0;
    };

}

extern "C"
{
    DLLIMPORT bililive::IDmkhimeServiceFactory* GetDmkhimeServiceFactory();
};

#endif  // OBS_DMKHIME_DMKHIME_SERVICE_H_