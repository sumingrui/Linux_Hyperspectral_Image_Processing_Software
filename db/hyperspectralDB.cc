#include "hyperspectralDB.h"
#include "DBtools.h"
#include "../share.h"

#include <fstream>

using std::string;
using std::vector;

SpectralDataSQL::SpectralDataSQL(string path) : dbPath_(path)
{
    mysql_init(&hyperspectral_sql_);
}

SpectralDataSQL::~SpectralDataSQL()
{
    mysql_close(&hyperspectral_sql_);
}

void SpectralDataSQL::Release()
{
    delete this;
}

// 连接SQL
int SpectralDataSQL::Connect(const char *host,
                             const char *user,
                             const char *passwd,
                             const char *db,
                             unsigned int port)
{
    if (mysql_real_connect(&hyperspectral_sql_, host, user, passwd, db, port, NULL, 0) == NULL)
    {
        log(error, mysql_error(&hyperspectral_sql_));
        return 0;
    }
    else
    {
        log(info, "Connect HSI database success.");
        mysql_options(&hyperspectral_sql_, MYSQL_SET_CHARSET_NAME, "utf8");
        return 1;
    }
}

// 初始化SQL
int SpectralDataSQL::Intialize()
{
    if (!GetFilesInDir(dbPath_, ofiles_))
    {
        log(error, "Cannot access database files.");
        return 0;
    }

    for (auto f : ofiles_)
    {
        char sqlquery[1024];
        snprintf(sqlquery, 512, "SELECT * FROM hyperspectral_data WHERE filename=\'%s\';", f.c_str());
        if (mysql_query(&hyperspectral_sql_, sqlquery))
        {
            log(error, string("Query error:") + mysql_error(&hyperspectral_sql_));
            return 0;
        }
        MYSQL_RES *res;
        res = mysql_store_result(&hyperspectral_sql_);
        if (mysql_num_rows(res) == 0)
        {
            InsertOneRow(f);
        }
        else
        {
            log(info, "Data available: " + f);
        }
        mysql_free_result(res);
    }
    return 1;
}

// 插入一行数据,filename: newrawSinglefile20190710142942
int SpectralDataSQL::InsertOneRow(string filename)
{
    HINFO hinfo;
    if (ReadConfig(dbPath_, filename, hinfo))
    {
        char sqlquery[1024];
        snprintf(sqlquery, 512, "INSERT INTO hyperspectral_data VALUES (\'%s\',%u,%u,%u,\'%s\',%u,%f,%f,%f,%f);",
                 hinfo.filename.c_str(), hinfo.resolution_w, hinfo.resolution_h, hinfo.bands, hinfo.date.c_str(),
                 hinfo.datatype, hinfo.longitude, hinfo.latitude, hinfo.altitude, hinfo.height);
        if (mysql_query(&hyperspectral_sql_, sqlquery))
        {
            log(error, string("Insert data error: ") + filename + mysql_error(&hyperspectral_sql_));
            return 0;
        }
        else
        {
            return 1;
        }
    }
    else
        return 0;
}

// 删除一行数据
int SpectralDataSQL::DeleteOneRow(string filename)
{
    char sqlquery[1024];
    snprintf(sqlquery, 512, "DELETE FROM hyperspectral_data WHERE filename=\'%s\';", filename.c_str());
    if (mysql_query(&hyperspectral_sql_, sqlquery))
    {
        log(error, string("Delete data error: ") + filename + mysql_error(&hyperspectral_sql_));
        return 0;
    }
    else
    {
        return 1;
    }
}

// 导出数据
int SpectralDataSQL::ExportOneRow(string infile, string &outfile)
{
    char sqlquery[1024];
    snprintf(sqlquery, 512, "SELECT * FROM hyperspectral_data WHERE filename=\'%s\';", infile.c_str());
    if (mysql_query(&hyperspectral_sql_, sqlquery))
    {
        log(error, string("Export data error: ") + mysql_error(&hyperspectral_sql_));
        return 0;
    }
    else
    {
        MYSQL_RES *res;
        MYSQL_ROW row;
        outfile = dbPath_ + infile + ".txt";
        std::ofstream ofile;
        ofile.open(outfile, std::ios::out | std::ios::app);
        res = mysql_store_result(&hyperspectral_sql_);
        while (row = mysql_fetch_row(res))
        {
            for (int i = 0; i < 10; i++)
            {
                ofile << row[i] << "\n";
            }
        }
        mysql_free_result(res);
        return 1;
    }
}

// 查询数据


string SpectralDataSQL::GetTaskfile()
{
    return task_file_;
}
