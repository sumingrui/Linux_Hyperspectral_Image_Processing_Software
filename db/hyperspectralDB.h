// author: sumingrui
// This is a database IO to hyperspectral images

#ifndef HYPERSPECTRALDB_H_
#define HYPERSPECTRALDB_H_

#include <string>
#include <mysql.h>
#include <stdio.h>
#include <vector>

using std::string;
using std::vector;

class SpectralDataSQL
{
public:
    SpectralDataSQL(string path);
    ~SpectralDataSQL();
    void Release();

    int Connect(const char *host, const char *user, const char *passwd, const char *db, unsigned int port); //返回值用int表示
    int Intialize();
    int CheckNewData();
    int InsertOneRow(string filename);
    int ExportOneRow(string infile, string &outfile);
    string GetTaskfile();

private:
    MYSQL hyperspectral_sql_;
    string dbPath_;
    vector<string> ofiles_;
    string task_file_;
};

#endif // HYPERSPECTRALDB_H_