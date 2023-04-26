#ifndef BILILIVE_BILILIVE_LIVEHIME_GIFT_IMAGE_IMAGE_RECORDER_H_
#define BILILIVE_BILILIVE_LIVEHIME_GIFT_IMAGE_IMAGE_RECORDER_H_

#include <map>


namespace livehime {

    /**
     * ��������ͼ��ļ�¼��
     * ��ǰ���ѱ��浽�����ļ��е�����ͼ��� id �� url �ɶԼ�¼�����������������ļ��С�
     * �������ļ�����һ���ļ�ͷ���Լ���������������������
     * ��ʹ��ʱ�������ȵ��� open �Խ��к��������������� close �Խ��޸ı��浽�ļ���
     * ���������л���� close��
     */
    class ImageRecorder {
    public:
        struct ImageData
        {
            std::string id;
            std::string url;
        };

    public:
        ImageRecorder();
        ~ImageRecorder();

        /**
         * ��ȡ�����ļ��������ļ�ͷ��������ڴ档
         * ����ڶ�ȡ�г����κδ��󣬽�ɾ�������ļ����������ļ�ͷ��
         * ��ȡ��ɾ������ͷ��ļ�������һֱռ�ø��ļ���
         * @param path �����ļ���λ��
         */
        void open(const std::wstring& path);

        /**
         * ��һ������������ڴ棬�ȴ�д���ļ���
         * ����ǰδ���� open ������ path ����Ϊ�գ��������Ч��
         * @param id ���� id������Ϊ������ָ���Ѵ��ڵ���������Ϊ�޸�
         * @param url ���� url���� id �ɶԱ����Ա���δ����� url �Ƿ�仯��
         */
        void put(const std::string& id, const ImageData& data);

        /**
         * ���ڴ��л�ȡһ����¼�
         * ����ǰδ���� open ������ path ����Ϊ�գ��������Ч������ false��
         * @param id ���� id������Ϊ����
         * @param url ���� url�������� false���ò������ᱻ�޸�
         * @return ���� true ��ʾ�ҵ� id ��Ӧ�ļ�¼��url ����ָ���Ӧ�� url��
         *         ���� false ��ʾδ�ҵ�
         */
        bool get(const std::string& id, ImageData* data);

        /**
         * ���ڴ����Ƴ��� id ָ���ļ�¼��
         * ����ǰδ���� open ������ path ����Ϊ�գ��������Ч��
         * @param id ���� id
         */
        void remove(const std::string& id);

        /**
         * ���ڴ��е��ļ�ͷ�ͼ�¼д���ļ�������� path��
         * �����д���г����κδ��󣬽�ɾ�������ļ���
         * ����ǰδ���� open ������ path ����Ϊ�գ��������Ч��
         */
        void close();

    private:
        struct Header {
            // �ļ�ͷ�������� 'R'
            char magic;
            // ���汾����ǰ�� 0
            uint16_t major_ver;
            // �ΰ汾����ǰ�� 1
            uint16_t minor_ver;
            // �ļ��б���ļ�¼��
            uint32_t total_count;
        };

        bool readFile(const std::wstring& path);
        bool writeFile(const std::wstring& path);

        Header header_;
        std::wstring path_;
        std::map<std::string, ImageData> contents_;
    };
}

#endif  // BILILIVE_BILILIVE_LIVEHIME_GIFT_IMAGE_IMAGE_RECORDER_H_