#include <io.h>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <iomanip>

#include "DBtools.h"
#include "../share.h"

using std::ifstream;
using std::ios;
using std::string;
using std::vector;
using std::wstring;

//wstring to string
string wstringToString(const wstring &wstr)
{
    LPCWSTR pwszSrc = wstr.c_str();
    int nLen = WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, NULL, 0, NULL, NULL);
    if (nLen == 0)
        return string("");

    char *pszDst = new char[nLen];
    if (!pszDst)
        return string("");

    WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, pszDst, nLen, NULL, NULL);
    string str(pszDst);
    delete[] pszDst;
    pszDst = NULL;

    return str;
}

//string to wstring
wstring stringToWstring(const string &str)
{
    LPCSTR pszSrc = str.c_str();
    int nLen = MultiByteToWideChar(CP_ACP, 0, pszSrc, -1, NULL, 0);
    if (nLen == 0)
        return wstring(L"");

    wchar_t *pwszDst = new wchar_t[nLen];
    if (!pwszDst)
        return wstring(L"");

    MultiByteToWideChar(CP_ACP, 0, pszSrc, -1, pwszDst, nLen);
    wstring wstr(pwszDst);
    delete[] pwszDst;
    pwszDst = NULL;

    return wstr;
}

int GetFilesInDir(string dbPath, vector<string> &ofiles)
{


    
}

int GetFilesInDir(string dbPath, vector<string> &ofiles)
{
    if (!DirPathExist(dbPath))
    {
        return 0;
    }

    wstring PostFix = L"*.raw";

    HANDLE hFind;
    WIN32_FIND_DATA data;

    ofiles.clear();

    wstring SearchName = stringToWstring(dbPath) + PostFix;

    hFind = FindFirstFile(SearchName.c_str(), &data);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            // add the path and file name together
            string FileRoot = wstringToString(data.cFileName);
            FileRoot = FileRoot.substr(0, FileRoot.length() - 4);
            ofiles.push_back(FileRoot);
        } while (FindNextFile(hFind, &data));
        FindClose(hFind);
    }
    return 1;
}

//检查DirPath是否存在
int DirPathExist(const string DirPath)
{
    if (!_access(DirPath.c_str(), 0))
        return 1;
    return 0;
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
        log(error, "config file can not open: " + filename);
        return 0;
    }
    else
    {
        hinfo.filename = filename;
        char strDate[1024];
        sprintf_s(strDate, 512, "%s-%s-%s %s:%s:%s", filename.substr(16, 4).c_str(), filename.substr(20, 2).c_str(), filename.substr(22, 2).c_str(),
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
