#include <libconfig.h++>
#include <cstdlib>
#include <unistd.h>
#include <sstream>
#include <sys/select.h>
#include <sys/inotify.h>
#include <string.h>

#include "bus.h"
#include "share.h"

using namespace libconfig;

extern int searching;

HSIC::Bus::Bus()
{
    if (!ReadAppConfig())
    {
        log(info, "Read config success.");
    }

    // 数据库初始化
    pHSIDB = new SpectralDataSQL(config_.dataPath);
    if (!pHSIDB->Connect(config_.hostName.c_str(), config_.account.c_str(),
                         config_.passwd.c_str(), config_.db.c_str(), config_.port))
    {
        exit(0);
    }
    if (!pHSIDB->Intialize())
    {
        exit(0);
    }
    else
    {
        log(info, "Intialize HSI database.");
    }
}

HSIC::Bus::~Bus()
{
    pHSIDB->Release();
}

// read config file
bool HSIC::Bus::ReadAppConfig()
{
    Config cfg;

    // Read the file. If there is an error, report it and exit.
    try
    {
        cfg.readFile("settings.cfg");
    }
    catch (const FileIOException &fioex)
    {
        //std::cerr << "I/O error while reading file." << std::endl;
        log(error, "I/O error while reading file.");
        return (EXIT_FAILURE);
    }
    catch (const ParseException &pex)
    {
        // std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
        //           << " - " << pex.getError() << std::endl;
        std::stringstream ss;
        ss << "Parse error at " << pex.getFile() << ":" << pex.getLine() << " - " << pex.getError();
        log(error, ss.str());
        return (EXIT_FAILURE);
    }

    try
    {
        string s = cfg.lookup("HostName");
        config_.hostName = s;
    }
    catch (const SettingNotFoundException &nfex)
    {
        log(error, "No 'HostName' setting in configuration file.");
    }

    try
    {
        string s = cfg.lookup("Account");
        config_.account = s;
    }
    catch (const SettingNotFoundException &nfex)
    {
        log(error, "No 'Account' setting in configuration file.");
    }

    try
    {
        string s = cfg.lookup("Passwd");
        config_.passwd = s;
    }
    catch (const SettingNotFoundException &nfex)
    {
        log(error, "No 'Passwd' setting in configuration file.");
    }

    try
    {
        string s = cfg.lookup("DB");
        config_.db = s;
    }
    catch (const SettingNotFoundException &nfex)
    {
        log(error, "No 'DB' setting in configuration file.");
    }

    try
    {
        config_.port = cfg.lookup("Port");
    }
    catch (const SettingNotFoundException &nfex)
    {
        log(error, "No 'Port' setting in configuration file.");
    }

    try
    {
        string s = cfg.lookup("DataPath");
        config_.dataPath = s;
    }
    catch (const SettingNotFoundException &nfex)
    {
        log(error, "No 'DataPath' setting in configuration file.");
    }

    // choose speed level
    try
    {
        config_.speed_level = cfg.lookup("speed_level");
    }
    catch (const SettingNotFoundException &nfex)
    {
        log(error, "No 'speed_level' setting in configuration file.");
    }

    return (EXIT_SUCCESS);
}

void HSIC::Bus::ObjSearch()
{
    int inotify_fd;
    inotify_fd = inotify_init();
    if (inotify_fd < 0)
    {
        log(LOGLEVEL::error, "inotify_init error.");
        exit(0);
    }
    int events = IN_CREATE | IN_DELETE; // 只对创建和删除进行监控
    int wd = inotify_add_watch(inotify_fd, config_.dataPath.c_str(), events);
    if (wd < 0)
    {
        log(LOGLEVEL::error, "Cannot add watch for " + config_.dataPath);
        exit(0);
    }

    while (searching)
    {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(inotify_fd, &rfds);
        int sel = select(inotify_fd + 1, &rfds, NULL, NULL, NULL);

        if (sel > 0)
        {
            int ret;
            char event_buf[1024];
            int event_size;
            int event_pos = 0;
            struct inotify_event *inev;

            ret = read(inotify_fd, event_buf, sizeof(event_buf));

            if (ret < (int)sizeof(*inev))
            {
                log(LOGLEVEL::error, "Cannot get inev: " + string(strerror(errno)));
                break;
            }

            while (ret >= (int)sizeof(*inev))
            {
                inev = (struct inotify_event *)(event_buf + event_pos);

                if (inev->len)
                {
                    if (inev->mask & IN_DELETE)
                    {
                        log(info, "delete file: " + string(inev->name));
                        DelHSIEntry(string(inev->name));
                    }
                    else if (inev->mask & IN_CREATE)
                    {
                        log(info, "create file: " + string(inev->name));
                        AddHSIEntry(string(inev->name));
                    }
                }

                event_size = sizeof(*inev) + inev->len;
                ret -= event_size;
                event_pos += event_size;
            }
        }
    }

    if (close(inotify_fd) < 0)
    {
        log(LOGLEVEL::error, "close (fd) = " + string(strerror(errno)));
    }
    log(info, "object search end.");
    return;
}

// 添加一个条目
int HSIC::Bus::AddHSIEntry(string filename)
{
    // 先判断后缀
    string postfix = filename.substr(filename.length() - 3);
    if (postfix == "raw" || postfix == "dci" || postfix == "hdr")
    {
        string rootname = config_.dataPath + filename.substr(0, filename.length() - 4);
        // raw,dci,hdr都存在添加
        if (access((rootname + ".raw").c_str(), F_OK) == 0 &&
            access((rootname + ".dci").c_str(), F_OK) == 0 &&
            access((rootname + ".hdr").c_str(), F_OK) == 0)
        {
            log(info, "Add entry " + rootname + " to database.");
            return 1;
        }
        return 0;
    }
    return 0;
}

// 删除一个条目
int HSIC::Bus::DelHSIEntry(string filename)
{
    string postfix = filename.substr(filename.length() - 3);
    if (postfix == "raw" || postfix == "dci" || postfix == "hdr")
    {
        string rootname = config_.dataPath + filename.substr(0, filename.length() - 4);
        // raw,dci,hdr都删除了去除该条目
        if (access((rootname + ".raw").c_str(), F_OK) == -1 &&
            access((rootname + ".dci").c_str(), F_OK) == -1 &&
            access((rootname + ".hdr").c_str(), F_OK) == -1)
        {
            log(info, "Delete entry " + rootname + " from database.");
            return 1;
        }
        return 0;
    }
    return 0;
}
