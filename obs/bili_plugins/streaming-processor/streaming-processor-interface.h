#ifndef OBS_BILI_PLUGINS_STREAMING_PROCESSOR_STREAMING_PROCESSOR_INTERFACE_H_
#define OBS_BILI_PLUGINS_STREAMING_PROCESSOR_STREAMING_PROCESSOR_INTERFACE_H_
#include <string>

namespace streaming_processor
{
    enum ProcessStatus
    {
        OPEN_LOCAL_ERROR = 0, //�򿪱��ؽ���������ʧ��

        // ���ڼ���/���ڵȴ����������ӣ�����ʱ��û�е�������������
        WAIT_FOR_TP_LINK,
        // �Ѿ��յ����������ˣ����������Ѿ����ϲ��Ѿ��ɹ������ƹ����ˣ�һ������Ӧ��ֻ֪ͨһ�Σ��ϲ��յ����֪ͨ��ȥ���������õõ�CND��ַ��
        TP_STREAM_RUNNING,
        // ������������
        TP_STREAMING_STOPPED,

        // ��������ʧ�ܣ��̶߳�û��
        START_CDN_STREAMING_FAILED,
        // ���ڷ����CDN������
        START_CDN_STREAMING,
        // ����CDNʧ���ˣ��ϲ���ܻ����¸�һ���µ�ַ�ü������ԣ�û�к󱸵�ַ�˾ͻ�ز��ˣ�
        CDN_STREAMING_CONNNECT_FAILED,
        // ������CDN������
        CDN_STREAMING_RUNNING,
        // ��CDN������ֹ�ˣ������Ǵ��������Ŀ������ϲ�����stop�ģ���ʱ�ϲ�Ҫ�ز��ˣ���CDN����ֹ��û��Ҫ�յ�����������Ҳû��Ҫ�������ˣ�
        CDN_STREAMING_STOPPED,

        // ֹͣ�����ײ�Ĺ����У�����ֹͣ�Żᴥ��
        STOP_STREAMING,

        // ���������߳��˳��ˣ�����ֹͣ�Żᴥ��
        STREAMING_STOPPED
    };

    class ProcessorCallback
    {
    public:
        virtual ~ProcessorCallback() {}

        virtual void ProcessorStatus(ProcessStatus status) = 0;

        virtual void OnSpeedTestStart() {}

        //************************************
        // Method:    OnSpeedTestProgressed
        // FullName:  streaming_processor::ProcessorCallback::OnSpeedTestProgressed
        // Access:    virtual public
        // Returns:   void
        // Qualifier:
        // Parameter: double progress: ���Խ���
        // Parameter: int cur_speed: �ٶ�ֵ����λKBps
        //************************************
        virtual void OnSpeedTestProgressed(double progress, int cur_speed) {}

        // speed: �ٶ�ֵ����λKBps
        //************************************
        // Method:    OnSpeedTestEnd
        // FullName:  streaming_processor::ProcessorCallback::OnSpeedTestEnd
        // Access:    virtual public
        // Returns:   void
        // Qualifier:
        // Parameter: int err_code
        // Parameter: int speed: �ٶ�ֵ����λKBps
        // Parameter: int duration: ����ʱ��������ʵ�ʵ�ʱ��������ֻ�Ƿ������ʱָ��������ʱ����
        //************************************
        virtual void OnSpeedTestEnd(int err_code, int speed, int duration) {}
    };

    class IStreamingProcessor : public std::enable_shared_from_this<IStreamingProcessor>
    {
    public:
        virtual ~IStreamingProcessor() {}

        virtual void AddObserver(ProcessorCallback* callback) = 0;
        virtual void RemoveObserver(ProcessorCallback* callback) = 0;

        // ���ɵ������������ؽ����ն���Ϣ
        // �����ҿ�ʼ�ڴ˵�ַ�Ͻ��м������ȴ��Զ���������
        virtual int InitProcess() = 0;

        // ����CDN��ַ���յ��ı�������ʼת���������ַ
        virtual bool StartProcess(const std::string& rtmp) = 0;

        // ��ֹ�������̣�ͣ������ͣ����
        virtual void StopProcess() = 0;

        // �Ƿ����ڹ�����
        virtual bool IsRunning() const = 0;

        // ������Ҫ�����SEI��Ϣ
        virtual void SetSeiInfo(const uint8_t* data, int len) = 0;

        // ����
        virtual bool SpeedTest(const unsigned char* data, size_t size, const std::string& addr,
            int base_on_kbps, int duration_per_addr) = 0;
        virtual void StopSpeedTest() = 0;
        virtual bool IsSpeedTesting() const = 0;

        // ��ȡflvͷ�����������Ϣ
        virtual const std::string& GetEncoderInfo() = 0;

        //����������ͨ��SEI�ӿ�
        virtual void AddSEIInfo(const uint8_t* data, int size) = 0;
    };

    extern "C" _declspec(dllexport) IStreamingProcessor* CreateStreamingProcessor(bool custom_rtmp = false);

}//namespace
#endif