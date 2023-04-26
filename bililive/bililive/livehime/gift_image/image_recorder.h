#ifndef BILILIVE_BILILIVE_LIVEHIME_GIFT_IMAGE_IMAGE_RECORDER_H_
#define BILILIVE_BILILIVE_LIVEHIME_GIFT_IMAGE_IMAGE_RECORDER_H_

#include <map>


namespace livehime {

    /**
     * 用于礼物图标的记录。
     * 当前将已保存到缓存文件夹的礼物图标的 id 和 url 成对记录到单独二进制数据文件中。
     * 该数据文件包含一个文件头，以及紧随其后的零个或多个数据项。
     * 在使用时，必须先调用 open 以进行后续操作；最后调用 close 以将修改保存到文件。
     * 析构函数中会调用 close。
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
         * 读取数据文件，解析文件头和数据项到内存。
         * 如果在读取中出现任何错误，将删除数据文件并生成新文件头。
         * 读取完成就立即释放文件，不会一直占用该文件。
         * @param path 数据文件的位置
         */
        void open(const std::wstring& path);

        /**
         * 将一条数据项放入内存，等待写入文件。
         * 若当前未调用 open 方法或 path 参数为空，则调用无效。
         * @param id 礼物 id，将作为索引。指定已存在的索引将视为修改
         * @param url 礼物 url，与 id 成对保存以便在未来检查 url 是否变化。
         */
        void put(const std::string& id, const ImageData& data);

        /**
         * 从内存中获取一条记录项。
         * 若当前未调用 open 方法或 path 参数为空，则调用无效，返回 false。
         * @param id 礼物 id，将作为索引
         * @param url 礼物 url。若返回 false，该参数不会被修改
         * @return 返回 true 表示找到 id 对应的记录，url 参数指向对应的 url；
         *         返回 false 表示未找到
         */
        bool get(const std::string& id, ImageData* data);

        /**
         * 从内存中移除由 id 指定的记录。
         * 若当前未调用 open 方法或 path 参数为空，则调用无效。
         * @param id 礼物 id
         */
        void remove(const std::string& id);

        /**
         * 将内存中的文件头和记录写入文件。并清除 path。
         * 如果在写入中出现任何错误，将删除数据文件。
         * 若当前未调用 open 方法或 path 参数为空，则调用无效。
         */
        void close();

    private:
        struct Header {
            // 文件头，必须是 'R'
            char magic;
            // 主版本，当前是 0
            uint16_t major_ver;
            // 次版本，当前是 1
            uint16_t minor_ver;
            // 文件中保存的记录数
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