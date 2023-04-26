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
     * �·���Ļ������Ľӿڡ�
     * ��Ļ��Դʵ�ָýӿڣ������·���������ʾ��
     */
    class IDmkhimeTextureDispatcher {
    public:
        virtual ~IDmkhimeTextureDispatcher() = default;

        /**
         * �����µĵ�Ļ������ʱ�ᴥ���ýӿڡ�
         */
        virtual void OnTexture(const obs_source_frame* frame) = 0;
    };


    /**
     * ��Ļ��Դ֪ͨ�ϲ�ʱʹ�õĽӿڡ�
     * ��Ļ��Դ����ĳЩ�¼�����ʱ֪ͨ�ϲ㡣ÿ����Ļ��Դ��ͨ������ӿڽ���֪ͨ��
     * ��ǰδ���־������ĸ���Ļ��Դ֪ͨ�ġ�
     */
    class IDmkhimeTexServiceCallback :
        public std::enable_shared_from_this<IDmkhimeTexServiceCallback> {
    public:
        virtual ~IDmkhimeTexServiceCallback() = default;

        /**
         * ֪ͨ�ϲ㣺��Ҫ���ײ�ι��Ļ��������ΪҪô������ӵĵ�Ļ��Դ��Ҫô
         * ���ڵ�Ļ��Դ�ɷǼ���״̬תΪ����״̬��
         *
         * {first} �����������������ϴ��Ա�״̬֮���Ƿ��ǵ�һ��Ҫ��ιʳ��
         */
        virtual void OnRefreshTexture(bool first) = 0;

        /**
         * ֪ͨ�ϲ㣺֮��������ι��Ļ��������ΪҪô�ײ��Ѳ����ڵ�Ļ��Դ��Ҫô
         * ���е�Ļ��Դ�����ڷǼ���״̬��
         */
        virtual void OnAutism() = 0;
    };


    /**
     * ��Ļ��Դ����Ϊ�ϲ��ṩ��������Ļ��Դ�Ľӿڡ�
     * �ö���ʵ��ȫ��Ψһ������Ҫʱ�ᱻ�������������е�Ļ��Դ��
     */
    class IDmkhimeTexService {
    public:
        using CallbackPtr = std::shared_ptr<IDmkhimeTexServiceCallback>;
        using CallbackWPtr = std::weak_ptr<IDmkhimeTexServiceCallback>;
        using VideoFramePtr = std::shared_ptr<obs_source_frame>;

        virtual ~IDmkhimeTexService() = default;

        /**
         * ��װ�ص������������߳�
         */
        virtual bool Launch(const CallbackPtr& callback) = 0;

        /**
         * �жϵ�ǰ�Ƿ���ڽ��������ʵ�塣
         * ��ʵ�� IDmkhimeTextureDispatcher �ӿڵĶ��󣬼���Ļ��Դ��
         */
        virtual bool HasDispatcher() = 0;

        /**
         * �жϵ�ǰ�Ƿ�����Ѽ���ĵ�Ļ��Դ��
         */
        virtual bool HasActivatedSource() = 0;

        /**
         * ����Ļ����֡���ݣ�λͼ���ݣ������ײ��·���
         * �����ݽ����·���ÿһ��ʵ�� IDmkhimeTextureDispatcher �ӿڵĶ��󣬼���Ļ��Դ��
         * ���۵ײ��Ƿ���ڵ�Ļ��Դ��Ҳ���۵�Ļ��Դ�Ƿ񼤻���ø÷����󶼻᳢���·���
         */
        virtual void Dispatch(const VideoFramePtr& frame) = 0;
    };


    class IDmkhimeServiceFactory {
    public:
        virtual ~IDmkhimeServiceFactory() = default;

        /**
         * ��ȡ IDmkhimeTexService ����
         */
        virtual IDmkhimeTexService* Get() = 0;
    };

}

extern "C"
{
    DLLIMPORT bililive::IDmkhimeServiceFactory* GetDmkhimeServiceFactory();
};

#endif  // OBS_DMKHIME_DMKHIME_SERVICE_H_