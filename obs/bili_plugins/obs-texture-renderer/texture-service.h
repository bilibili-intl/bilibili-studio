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
     * �·�����Ľӿڡ�
     * ����Դʵ�ָýӿڣ������·���������ʾ��
     */
    class ITextureDispatcher {
    public:
        virtual ~ITextureDispatcher() = default;

        /**
         * �����µ�������ʱ�ᴥ���ýӿڡ�
         */
        virtual void OnTexture(const obs_source_frame* frame) = 0;
    };


    /**
     * ����Դ֪ͨ�ϲ�ʱʹ�õĽӿڡ�
     * ����Դ����ĳЩ�¼�����ʱ֪ͨ�ϲ㡣ÿ������Դ��ͨ������ӿڽ���֪ͨ��
     * ��ǰδ���־������ĸ�����Դ֪ͨ�ġ�
     */
    class ITextureServiceCallback :
        public std::enable_shared_from_this<ITextureServiceCallback> {
    public:
        virtual ~ITextureServiceCallback() = default;

        /**
         * ֪ͨ�ϲ㣺��Ҫ���ײ�ι������ΪҪô������ӵ�����Դ��Ҫô
         * ��������Դ�ɷǼ���״̬תΪ����״̬��
         *
         * {first} �����������������ϴ��Ա�״̬֮���Ƿ��ǵ�һ��Ҫ��ιʳ��
         */
        virtual void OnRefreshTexture(bool first) = 0;

        /**
         * ֪ͨ�ϲ㣺֮��������ι������ΪҪô�ײ��Ѳ���������Դ��Ҫô
         * ��������Դ�����ڷǼ���״̬��
         */
        virtual void OnAutism() = 0;
    };


    /**
     * ����Դ����Ϊ�ϲ��ṩ����������Դ�Ľӿڡ�
     * �ö���ʵ��ȫ��Ψһ������Ҫʱ�ᱻ������������������Դ��
     */
    class ITextureService {
    public:
        using CallbackPtr = std::shared_ptr<ITextureServiceCallback>;
        using CallbackWPtr = std::weak_ptr<ITextureServiceCallback>;
        using VideoFramePtr = std::shared_ptr<obs_source_frame>;

        virtual ~ITextureService() = default;

        /**
         * ��װ�ص������������߳�
         */
        virtual bool Launch(const CallbackPtr& callback) = 0;

        /**
         * �жϵ�ǰ�Ƿ���ڽ��������ʵ�塣
         * ��ʵ�� ITextureDispatcher �ӿڵĶ��󣬼�����Դ��
         */
        virtual bool HasDispatcher() = 0;

        /**
         * �жϵ�ǰ�Ƿ�����Ѽ��������Դ��
         */
        virtual bool HasActivatedSource() = 0;

        /**
         * ��֡���ݣ�λͼ���ݣ������ײ��·���
         * �����ݽ����·���ÿһ��ʵ�� IDmkhimeTextureDispatcher �ӿڵĶ��󣬼�����Դ��
         * ���۵ײ��Ƿ��������Դ��Ҳ��������Դ�Ƿ񼤻���ø÷����󶼻᳢���·���
         */
        virtual void Dispatch(const VideoFramePtr& frame, TextureType type, int id) = 0;
    };


    class ITextureServiceFactory {
    public:
        virtual ~ITextureServiceFactory() = default;

        /**
         * ��ȡ ITextureService ����
         * ��һ�ε���ʱ�ᴴ������
         */
        virtual std::shared_ptr<ITextureService> Fetch() = 0;
    };

}

extern "C"
{
    DLLIMPORT livehime::ITextureServiceFactory* GetTextureServiceFactory();
};

#endif  // OBS_TEXTURE_RENDERER_TEXTURE_SERVICE_H_