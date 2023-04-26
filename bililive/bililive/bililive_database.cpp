#include "bililive/bililive/bililive_database.h"

#include "base/logging.h"
#include "base/memory/scoped_vector.h"
#include "base/path_service.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/threading/thread_restrictions.h"

#include "bilibase/basic_types.h"

#include "sql/statement.h"

#include "url/gurl.h"

#include "bililive/public/bililive/bililive_process.h"
#include "bililive/public/common/bililive_paths.h"
#include "bililive/public/secret/bililive_secret.h"

namespace {

const int kReservedVersion = 0;
const base::FilePath::CharType kDBName[] = L"bililive.db";

base::FilePath AcquireDatabasePath()
{
    base::FilePath user_data_path = GetBililiveProcess()->profile()->GetPath();
    return user_data_path.Append(kDBName);
}

}   // namespace

const int BililiveDatabase::kVersion = 4;

BililiveDatabase::BililiveDatabase()
    : db_path_(AcquireDatabasePath())
{
    // We can do nothing if we couldn't even establish connection to the database, or we
    // failed to create tables we demand on.
    // Just let the application crash if any violations occur.
    CHECK(db_conn_.Open(db_path_));

    if (!db_conn_.DoesTableExist("DatabaseVersion")) {
        OnCreate();
    }

    auto deployed_ver = GetDeployedVersion();
    if (deployed_ver < kVersion) {
        OnUpgrade(deployed_ver, kVersion);
        SetDeployedVersion(kVersion);
    }

    DCHECK(deployed_ver <= kVersion) << "Potential database downgrade!";
}

BililiveDatabase::~BililiveDatabase()
{}

void BililiveDatabase::OnCreate()
{
    const char kCreateVersionTable[] =
        "CREATE TABLE IF NOT EXISTS DatabaseVersion ("
        "Version INTEGER PRIMARY KEY NOT NULL DEFAULT(0)"
        ");";

    DCHECK(db_conn_.IsSQLValid(kCreateVersionTable));

    CHECK(db_conn_.Execute(kCreateVersionTable));

    const auto kInsertInitialVer =
        base::StringPrintf("INSERT INTO DatabaseVersion (Version) VALUES (%d);", kReservedVersion);
    CHECK(db_conn_.Execute(kInsertInitialVer.c_str()));
}

/**
 * For each <db-ver, app-ver> pair
 * <2, 1.5.0> : Add columns CommercialPartnerID, CopyRestricted, TimingSubmit, and SubmitTimestamp.
 */

void BililiveDatabase::OnUpgrade(int old_ver, int new_ver)
{
    if (old_ver < 1 && new_ver >= 1) {
        CHECK(CreateDanmakuTable());
        CHECK(CreateVideoArchiveTable());
        CHECK(CreateVideoUploadTable());
    }

    if (old_ver < 2 && new_ver >= 2) {
        AlterVideoArchiveTableForVer2();
    }

    if (new_ver >= 3)
    {
        AlterDanmakuTableForVer3();
    }

    if (new_ver >= 4)
    {
        AlterDanmakuTableForVer4();
    }
}

int BililiveDatabase::GetDeployedVersion()
{
    const char kQuery[] = "SELECT Version FROM DatabaseVersion;";

    DCHECK(db_conn_.IsSQLValid(kQuery));

    sql::Statement stmt(db_conn_.GetUniqueStatement(kQuery));
    if (stmt.Step()) {
        return stmt.ColumnInt(0);
    }

    CHECK(stmt.Succeeded());

    return kReservedVersion;
}

void BililiveDatabase::SetDeployedVersion(int new_version)
{
    const char kUpdate[] =
        "UPDATE DatabaseVersion SET "
        "Version = ?;";

    DCHECK(db_conn_.IsSQLValid(kUpdate));

    sql::Statement stmt(db_conn_.GetUniqueStatement(kUpdate));
    stmt.BindInt(0, new_version);

    bool succeeded = stmt.Run();
    LOG_IF(WARNING, !succeeded) << "Failed to upgrade database version to " << new_version;
}

bool BililiveDatabase::InsertDanmaku(const DanmakuInfo &danmaku)
{
    std::string sql;
    sql::Statement stmt;
    if (danmaku.dt != DT_SEND_GIFT)
    {
        sql = "INSERT INTO danmaku_ex (uid,uname,text,stime,manager,vip,svip,barrage,type,ts,ct) VALUES (?,?,?,?,?,?,?,?,?,?,?)";
        stmt.Assign(db_conn_.GetCachedStatement(SQL_FROM_HERE, sql.c_str()));
    }
    else
    {
        sql = "INSERT INTO danmaku_ex (uid,uname,text,stime,manager,vip,svip,barrage,type,ts,ct,"\
            "gift_id, gift_name, gift_count, gift_coin, gift_coin_type) "\
            "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)";
        stmt.Assign(db_conn_.GetCachedStatement(SQL_FROM_HERE, sql.c_str()));
    }
    bool bRet = stmt.BindInt64(0, danmaku.uid);
    bRet &= stmt.BindString(1, danmaku.uname);
    bRet &= stmt.BindString(2, danmaku.danmaku);
    bRet &= stmt.BindInt64(3, danmaku.stime);
    bRet &= stmt.BindInt(4, danmaku.manager);
    bRet &= stmt.BindInt(5, danmaku.vip);
    bRet &= stmt.BindInt(6, danmaku.svip);
    bRet &= stmt.BindInt(7, danmaku.barrage);
    bRet &= stmt.BindInt(8, static_cast<int>(danmaku.dt));
    bRet &= stmt.BindInt64(9, danmaku.ts);
    bRet &= stmt.BindString(10, danmaku.ct);

    if (danmaku.dt == DT_SEND_GIFT)
    {
        bRet &= stmt.BindInt(11, danmaku.gift_id);
        bRet &= stmt.BindString(12, danmaku.giftName);
        bRet &= stmt.BindInt(13, danmaku.num);
        bRet &= stmt.BindInt(14, danmaku.batch_total_coin);
        bRet &= stmt.BindInt(15, static_cast<int>(danmaku.gift_coin_type));
    }

    if (!bRet)
    {
        return false;
    }

    return stmt.Step();
}

int BililiveDatabase::GetDanmakuCount()
{
    int count = 0;

    if (db_conn_.DoesTableExist("danmaku_ex"))
    {
        std::string sql = "SELECT count(id) FROM danmaku_ex";
        sql::Statement stmt(db_conn_.GetCachedStatement(SQL_FROM_HERE, sql.c_str()));
        if (stmt.Step())
        {
            count = stmt.ColumnInt(0);
        }
    }

    return count;
}

bool BililiveDatabase::GetDanmaku(int start, int count, ScopedVector<DanmakuInfo> &vct)
{
    if (db_conn_.DoesTableExist("danmaku_ex"))
    {
        std::string sql = "SELECT uid,uname,text,stime,manager,vip,svip,barrage,type,ts,ct FROM danmaku_ex ORDER BY id LIMIT ?,?";
        sql::Statement stmt(db_conn_.GetCachedStatement(SQL_FROM_HERE, sql.c_str()));
        bool bRet = stmt.BindInt(0, start);
        bRet &= stmt.BindInt(1, count);
        if (bRet)
        {
            while (stmt.Step())
            {
                DanmakuInfo *info = new DanmakuInfo();
                info->uid = stmt.ColumnInt64(0);
                info->uname = stmt.ColumnString(1);
                info->danmaku = stmt.ColumnString(2);
                info->stime = stmt.ColumnInt64(3);
                info->manager = stmt.ColumnInt(4);
                info->vip = stmt.ColumnInt(5);
                info->svip = stmt.ColumnInt(6);
                info->barrage = stmt.ColumnInt(7);
                info->dt = static_cast<DanmakuType>(stmt.ColumnInt(8));
                info->ts = stmt.ColumnInt64(9);
                info->ct = stmt.ColumnString(10);
                vct.push_back(info);
            }
            return true;
        }
    }

    return false;
}

bool BililiveDatabase::SearchDanmaku(const std::string& content, int64 time_range, int max_count,
    ScopedVector<DanmakuInfo>& vct) {
    if (db_conn_.DoesTableExist("danmaku_ex"))
    {
        std::string search_content = "%";
        search_content.append(content).append("%");

        std::string sql = "SELECT id,uid,uname,text,stime,manager,vip,svip,barrage,type,ts,ct \
        FROM danmaku_ex WHERE (uname like ? OR text like ?) AND stime >= ? ORDER BY id LIMIT ?";

        sql::Statement stmt(db_conn_.GetCachedStatement(SQL_FROM_HERE, sql.c_str()));
        bool bRet = stmt.BindString(0, search_content);
        bRet &= stmt.BindString(1, search_content);
        bRet &= stmt.BindInt64(2, time_range);
        bRet &= stmt.BindInt(3, max_count);

        while (stmt.Step())
        {
            DanmakuInfo *info = new DanmakuInfo();
            info->id = stmt.ColumnInt(0);
            info->uid = stmt.ColumnInt64(1);
            info->uname = stmt.ColumnString(2);
            info->danmaku = stmt.ColumnString(3);
            info->stime = stmt.ColumnInt64(4);
            info->manager = stmt.ColumnInt(5);
            info->vip = stmt.ColumnInt(6);
            info->svip = stmt.ColumnInt(7);
            info->barrage = stmt.ColumnInt(8);
            info->dt = static_cast<DanmakuType>(stmt.ColumnInt(9));
            info->ts = stmt.ColumnInt64(10);
            info->ct = stmt.ColumnString(11);
            vct.push_back(info);
        }

        return true;
    }

    return false;
}

bool BililiveDatabase::GetGiftDanmaku(int start, int count, int64 start_stime, int64 end_stime, ScopedVector<DanmakuInfo>& vct,
    GiftCoinType coin_type/* = GCT_GOLD*/)
{
    if (db_conn_.DoesTableExist("danmaku_ex"))
    {
        std::string sql = "SELECT uid,uname,stime,gift_id,gift_name,gift_count,gift_coin "\
            "FROM danmaku_ex WHERE type == ? AND (stime BETWEEN ? AND ?) AND gift_coin_type == ? ORDER BY stime DESC LIMIT ?,?";
        sql::Statement stmt(db_conn_.GetCachedStatement(SQL_FROM_HERE, sql.c_str()));
        bool bRet = stmt.BindInt(0, DT_SEND_GIFT);
        bRet &= stmt.BindInt64(1, start_stime);
        bRet &= stmt.BindInt64(2, end_stime);
        bRet &= stmt.BindInt(3, static_cast<int>(coin_type));
        bRet &= stmt.BindInt(4, start);
        bRet &= stmt.BindInt(5, count);
        if (bRet)
        {
            while (stmt.Step())
            {
                DanmakuInfo *info = new DanmakuInfo();
                info->uid = stmt.ColumnInt64(0);
                info->uname = stmt.ColumnString(1);
                info->stime = stmt.ColumnInt64(2);
                info->gift_id = stmt.ColumnInt(3);
                info->giftName = stmt.ColumnString(4);
                info->num = stmt.ColumnInt(5);
                info->batch_total_coin = stmt.ColumnInt(6);
                info->gift_coin_type = coin_type;
                vct.push_back(info);
            }
            return true;
        }
    }

    return false;
}

bool BililiveDatabase::CreateDanmakuTable()
{
    // Table building
    std::string sql =
        "CREATE TABLE IF NOT EXISTS danmaku_ex ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "uid INT64 NOT NULL,"
        "uname NVARCHAR(32) NOT NULL,"
        "manager INTEGER DEFAULT(0),"
        "vip INTEGER DEFAULT(0),"
        "svip INTEGER DEFAULT(0),"
        "text NVARCHAR(260) NOT NULL,"
        "stime INT64 NOT NULL,"
        "rtime DATETIME DEFAULT(datetime(CURRENT_TIMESTAMP,'localtime')),"
        "type INTEGER DEFAULT(0),"
        "barrage INTEGER DEFAULT(0),"
        "ts INT64 DEFAULT(0),"
        "ct NVARCHAR(128) NOT NULL"
        ")";
    if (db_conn_.Execute(sql.c_str()))
    {
        // 20170115 Added a bullet-type field to the bullet-type information table
        if (!db_conn_.DoesColumnExist("danmaku_ex", "type"))
        {
            std::string alter = "ALTER TABLE danmaku_ex ADD type INTEGER DEFAULT(0)";
            if (!db_conn_.Execute(alter.c_str()))
            {
                return false;
            }
        }

        // 20170405 Adds a barrage field to the barrage info table
        if (!db_conn_.DoesColumnExist("danmaku_ex", "barrage"))
        {
            std::string alter = "ALTER TABLE danmaku_ex ADD barrage INTEGER DEFAULT(0)";
            if (!db_conn_.Execute(alter.c_str()))
            {
                return false;
            }
        }

        // Build indexes to improve query speed
        std::string index = "CREATE INDEX IF NOT EXISTS danmaku_ex_stime_index ON danmaku_ex(stime)";
        db_conn_.Execute(index.c_str());

        return true;
    }

    return false;
}

void BililiveDatabase::AlterDanmakuTableForVer3()
{
    // Add gift number, gift ID, gift name, gift value fields for gift box requirements
    if (!db_conn_.DoesColumnExist("danmaku_ex", "gift_name"))
    {
        std::string alter = "ALTER TABLE danmaku_ex ADD COLUMN gift_name NVARCHAR(32) DEFAULT('')";
        if (!db_conn_.Execute(alter.c_str()))
        {
            LOG(WARNING) << "Alter table danmaku_ex failure!" << db_conn_.GetErrorMessage();
        }
    }

    std::vector<std::string> filters = { "gift_id", "gift_count", "gift_coin", "gift_coin_type" };
    for (const std::string &str : filters)
    {
        if (!db_conn_.DoesColumnExist("danmaku_ex", str.c_str()))
        {
            std::string alter = base::StringPrintf("ALTER TABLE danmaku_ex ADD COLUMN %s INTEGER DEFAULT(0)", str.c_str());
            if (!db_conn_.Execute(alter.c_str()))
            {
                LOG(WARNING) << "Alter table danmaku_ex failure!" << db_conn_.GetErrorMessage();
            }
        }
    }
}

void BililiveDatabase::AlterDanmakuTableForVer4(){
    // 20200119 Added ts and ct fields to the bullet-screen information table
    if (!db_conn_.DoesColumnExist("danmaku_ex", "ts"))
    {
        std::string alter = "ALTER TABLE danmaku_ex ADD ts INT64 DEFAULT(0)";
        if (!db_conn_.Execute(alter.c_str()))
        {
            LOG(WARNING) << "Alter table danmaku_ex failure!" << db_conn_.GetErrorMessage();
        }
    }

    if (!db_conn_.DoesColumnExist("danmaku_ex", "ct"))
    {
        std::string alter = "ALTER TABLE danmaku_ex ADD ct NVARCHAR(128) DEFAULT('')";
        if (!db_conn_.Execute(alter.c_str()))
        {
            LOG(WARNING) << "Alter table danmaku_ex failure!" << db_conn_.GetErrorMessage();
        }
    }
}

// -*- Video Archive Tables -*-

bool BililiveDatabase::CreateVideoArchiveTable()
{
    const char kCreateTable[] =
        "CREATE TABLE IF NOT EXISTS VideoArchive ("
        "ID INTEGER PRIMARY KEY,"
        "Title TEXT NOT NULL,"
        "CoverURL TEXT,"
        "Description TEXT,"
        "Type INTEGER NOT NULL,"
        "EventID INTEGER NOT NULL,"
        "ChannelID INTEGER NOT NULL,"
        "ReproductionRemark TEXT,"
        "Tags TEXT,"
        "AutoSubmit INTEGER NOT NULL"
        ");";

    const char kBuildIndex[] = "CREATE INDEX IF NOT EXISTS VideoArchiveIDIndex ON VideoArchive(ID);";

    DCHECK(db_conn_.IsSQLValid(kCreateTable));
    if (!db_conn_.Execute(kCreateTable)) {
        LOG(ERROR) << "Failed to create table VideoArchive: " << db_conn_.GetErrorMessage();
        return false;
    }

    DCHECK(db_conn_.IsSQLValid(kBuildIndex));
    if (!db_conn_.Execute(kBuildIndex)) {
        LOG(ERROR) << "Failed to create index for table VideoArchive: " << db_conn_.GetErrorMessage();
        return false;
    }

    return true;
}

void BililiveDatabase::AlterVideoArchiveTableForVer2()
{
    // SQLite doesn't support adding multiple columns at once, so shame.
    const char kAlter[] =
        "ALTER TABLE VideoArchive ADD COLUMN CommercialPartnerID INTEGER NOT NULL DEFAULT(0);"
        "ALTER TABLE VideoArchive ADD COLUMN CopyRestricted INTEGER NOT NULL DEFAULT(0);"
        "ALTER TABLE VideoArchive ADD COLUMN TimingSubmit INTEGER NOT NULL DEFAULT(0);"
        "ALTER TABLE VideoArchive ADD COLUMN SubmitTimestamp INTEGER NOT NULL DEFAULT(0);";

    bool succeeded = db_conn_.Execute(kAlter);

    // We still have chance though.
    if (!succeeded) {
        LOG(WARNING) << "Alter table VideoArchive failure!" << db_conn_.GetErrorMessage();
    }

    succeeded = db_conn_.DoesColumnExist("VideoArchive", "CommercialPartnerID") &&
                db_conn_.DoesColumnExist("VideoArchive", "CopyRestricted") &&
                db_conn_.DoesColumnExist("VideoArchive", "TimingSubmit") &&
                db_conn_.DoesColumnExist("VideoArchive", "SubmitTimestamp");

    CHECK(succeeded) << "Some columns were missing!";
}

// -*- Video Upload Task Tables -*-

bool BililiveDatabase::CreateVideoUploadTable()
{
    const char kCreateTable[] =
        "CREATE TABLE IF NOT EXISTS VideoUpload ("
        "TaskID INTEGER PRIMARY KEY,"
        "DisplayIndex INTEGER NOT NULL,"
        "VideoPath TEXT NOT NULL,"
        "UploadURL TEXT,"
        "CompleteURL TEXT,"
        "Filename TEXT,"
        "TransmittedChunks INTEGER NOT NULL,"
        "TotalChunks INTEGER NOT NULL,"
        "Completed INTEGER NOT NULL,"
        "ArchiveID INTEGER NOT NULL,"
        "Title TEXT NOT NULL,"
        "Desc TEXT,"
        "RequirePreview INTEGER NOT NULL"
        ");";

    const char kBuildIndex[] =
        "CREATE INDEX IF NOT EXISTS VideoUploadIndex ON VideoUpload(TaskID, DisplayIndex);";

    DCHECK(db_conn_.IsSQLValid(kCreateTable));
    if (!db_conn_.Execute(kCreateTable)) {
        LOG(ERROR) << "Failed to create table VideoUpload: " << db_conn_.GetErrorMessage();
        return false;
    }

    DCHECK(db_conn_.IsSQLValid(kBuildIndex));
    if (!db_conn_.Execute(kBuildIndex)) {
        LOG(ERROR) << "Failed to build index for table VideoUpload: " << db_conn_.GetErrorMessage();
        return false;
    }

    return true;
}
