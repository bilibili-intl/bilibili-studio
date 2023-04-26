#ifndef BILILIVE_BILILIVE_BILILIVE_DATABASE_H_
#define BILILIVE_BILILIVE_BILILIVE_DATABASE_H_

#include <vector>

#include "base/files/file_path.h"

#include "sql/connection.h"

#include "bililive/bililive/livehime/server_broadcast/broadcast_service.h"


class BililiveDatabase {
public:
    BililiveDatabase();

    ~BililiveDatabase();

    bool InsertDanmaku(const DanmakuInfo &danmaku);

    int GetDanmakuCount();

    bool GetDanmaku(int start, int count, ScopedVector<DanmakuInfo>& vct);

    bool SearchDanmaku(const std::string& content, int64 time_range, int max_count,
        ScopedVector<DanmakuInfo>& vct);
  
    // gift
    bool GetGiftDanmaku(int start, int count, int64 start_stime, int64 end_stime, ScopedVector<DanmakuInfo>& vct,
        GiftCoinType coin_type = GCT_GOLD);

private:
    void OnCreate();

    // `old_ver` is the version of the database previously deployed on the user; while
    // `new_ver` is the version currently defined by application, i.e. BililiveDatabase::kVersion.
    void OnUpgrade(int old_ver, int new_ver);

    int GetDeployedVersion();

    void SetDeployedVersion(int new_version);

    bool CreateDanmakuTable();

    bool CreateVideoArchiveTable();

    void AlterVideoArchiveTableForVer2();

    bool CreateVideoUploadTable();

    void AlterDanmakuTableForVer3();

    void AlterDanmakuTableForVer4();

    DISALLOW_COPY_AND_ASSIGN(BililiveDatabase);

private:
    static const int kVersion;
    base::FilePath db_path_;
    sql::Connection db_conn_;
};

#endif  // BILILIVE_BILILIVE_BILILIVE_DATABASE_H_