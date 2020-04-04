#include <fstream>
#include <dirent.h>
#include <string.h>

#include "DBtools.h"
#include "../share.h"

using std::ifstream;
using std::ios;
using std::string;
using std::vector;
using std::wstring;

int GetFilesInDir(string dbPath, vector<string> &ofiles)
{
    DIR *dirptr = NULL;
    struct dirent *dirp;

    //打开目录失败
    if ((dirptr = opendir(dbPath.c_str())) == NULL)
    {
        return 0;
    }

    while ((dirp = readdir(dirptr)) != NULL)
    {
        //判断是否为文件以及文件后缀名
        if ((dirp->d_type == DT_REG) && 0 == (strcmp(strchr(dirp->d_name, '.'), ".raw")))
        {
            ofiles.push_back((string(dirp->d_name)).substr(0, string(dirp->d_name).length() - 4));
        }
    }
    closedir(dirptr);

    return 1;
}

//读取dci和hdr，填充数据库
int ReadConfig(string dbPath, string filename, HINFO &hinfo)
{

    string hinfo_dci = dbPath + filename + ".dci";
    string hinfo_hdr = dbPath + filename + ".hdr";

    ifstream infile_dci, infile_hdr;
    infile_dci.open(hinfo_dci, ios::in);
    infile_hdr.open(hinfo_hdr, ios::in);
    if (!infile_dci.is_open() || !infile_hdr.is_open())
    {
        log(error, "Parameter file can not open: " + filename);
        return 0;
    }
    else
    {
        hinfo.filename = filename;
        char strDate[1024];
        snprintf(strDate, 512, "%s-%s-%s %s:%s:%s", filename.substr(16, 4).c_str(), filename.substr(20, 2).c_str(), filename.substr(22, 2).c_str(),
                 filename.substr(24, 2).c_str(), filename.substr(26, 2).c_str(), filename.substr(28).c_str());
        hinfo.date = strDate;
        string line_dci;

        while (getline(infile_dci, line_dci))
        {
            string str1(" altitude = "), str2(" latitude = "), str3(" longitude = "), str4(" height = ");
            float num;
            if (line_dci.find(str1) != line_dci.npos)
            {
                num = stof(line_dci.substr(12));
                hinfo.altitude = num;
            }
            else if (line_dci.find(str2) != line_dci.npos)
            {
                num = stof(line_dci.substr(12));
                hinfo.latitude = num;
            }
            else if (line_dci.find(str3) != line_dci.npos)
            {
                num = stof(line_dci.substr(13));
                hinfo.longitude = num;
            }
            else if (line_dci.find(str4) != line_dci.npos)
            {
                num = stof(line_dci.substr(10));
                hinfo.height = num;
            }
        }

        string line_hdr;

        while (getline(infile_hdr, line_hdr))
        {
            string str1("samples = "), str2("lines = "), str3("bands = "), str4("data type = ");
            int num;
            if (line_hdr.find(str1) != line_hdr.npos)
            {
                num = stoi(line_hdr.substr(10));
                hinfo.resolution_w = num;
            }
            else if (line_hdr.find(str2) != line_hdr.npos)
            {
                num = stoi(line_hdr.substr(8));
                hinfo.resolution_h = num;
            }
            else if (line_hdr.find(str3) != line_hdr.npos)
            {
                num = stoi(line_hdr.substr(8));
                hinfo.bands = num;
            }
            else if (line_hdr.find(str4) != line_hdr.npos)
            {
                num = stoi(line_hdr.substr(12));
                hinfo.datatype = num;
            }
        }
        infile_dci.close();
        infile_hdr.close();
        return 1;
    }
}
