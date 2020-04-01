#include <libconfig.h++>
#include <cstdlib>
#include <unistd.h>
#include <sstream>

#include "bus.h"
#include "share.h"

using namespace libconfig;

HSIC::Bus::Bus()
{
    if (!ReadAppConfig())
    {
        log(info, "Read config success.");
    }

    // 数据库初始化
    pHSIDB = new SpectralDataSQL(config_.dataPath);
    if (!pHSIDB->Connect(config_.hostName, config_.account, config_.passwd, config_.db, config_.port))
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
    DestoryExportObj(pHsiDB);
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
        config_.hostName = cfg.lookup("HostName");
    }
    catch (const SettingNotFoundException &nfex)
    {
        log(error, "No 'HostName' setting in configuration file.");
    }

    try
    {
        config_.account = cfg.lookup("Account");
    }
    catch (const SettingNotFoundException &nfex)
    {
        log(error, "No 'Account' setting in configuration file.");
    }

    try
    {
        config_.passwd = cfg.lookup("Passwd");
    }
    catch (const SettingNotFoundException &nfex)
    {
        log(error, "No 'Passwd' setting in configuration file.");
    }

    try
    {
        config_.db = cfg.lookup("DB");
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
        config_.dataPath = cfg.lookup("DataPath");
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

void HSIC::Bus::MultiModule()
{
    ObjSearch();
}

void HSIC::Bus::ObjSearch()
{
    while (1)
    {
        string sendImgPath;

        // 选择算法，读取参数（如果有更新）
        try
        {
            config_.speed_level = cfg.lookup("speed_level");
        }
        catch (const SettingNotFoundException &nfex)
        {
            log(error, "Cannot update 'speed_level' setting in configuration file.");
        }
        
        switch (config_.speed_level)
        {
        case 1:
            // slow
            log(info, "Choose algorithm speed 1.");

            break;

        case 2:
            // middle
            log(info, "Choose algorithm speed 2.");
            //TF_2dcnn(config_.dataPath, pHsiDB->GetTaskfile(), sendImgPath);
            //log(info, "save recognition image: " + sendImgPath);
            break;

        case 3:
            //fast
            log(info, "Choose algorithm speed 3.");

            break;

        default:
            log(warnning, "No such speed level.");
            break;
        }

        sleep(5);

        // 详查算法
    }
}
