// Author: SUMINGRUI

#ifndef TASKSCHEDULING_H_
#define TASKSCHEDULING_H_

#include <string>
#include <thread>
#include "share.h"
#include "stubclient.h"
#include <jsonrpccpp/client/connectors/httpclient.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

using namespace std;
using namespace jsonrpc;

// Do not support task queue
int Task_Assignment_Model = 1; // 1: only Xvaier 2: Xvaier and NUC
int Task_Available = 0;

struct TaskInfo
{
    string t_path;
    string t_filename;
    string t_result;
    int t_time;
} TaskInfo_default = {"", "", "none", 0};

// 发送文件
int ScpTaskfile(string path, string taskfile)
{
    FILE *fp = NULL;
    string cmd = "scp " + path + taskfile + " root@192.168.137.2:/home/nvidia/repos/l4ttensorflow/tf_server/AArch64_HSI_Tensorflow_Server/dataset && echo $?";

    fp = popen(cmd.c_str(), "r");
    if (fp == NULL)
    {
        log(error, "popen error!");
        return 0;
    }

    char resp[100];
    while (fgets(resp, sizeof(resp), fp) != NULL)
    {
        log(info, "SCP send task file result echo :" + string(resp));
    }

    if (strcmp(resp, "0\n") == 0)
    {
        // succeed
        pclose(fp);
        return 1;
    }
    // fail
    pclose(fp);
    return 0;
}

// 取xml文件
int GetXMLfile(string taskfile)
{
    FILE *fp = NULL;

    string cmd = "scp root@192.168.137.2:/home/nvidia/repos/l4ttensorflow/tf_server/AArch64_HSI_Tensorflow_Server/results/" 
                    + taskfile.substr(0, taskfile.length() - 4) + "-Xavier.* /repos/Linux_Hyperspectral_Image_Processing_Software/results/" + " && echo $?";

    fp = popen(cmd.c_str(), "r");
    if (fp == NULL)
    {
        log(error, "popen error!");
        return 0;
    }

    char resp[100];
    while (fgets(resp, sizeof(resp), fp) != NULL)
    {
        log(info, "SCP get XML file result echo :" + string(resp));
    }

    if (strcmp(resp, "0\n") == 0)
    {
        // succeed
        pclose(fp);
        return 1;
    }
    // fail
    pclose(fp);
    return 0;
}

void ExecuteTask(string path, string taskfile, int useNUC, double XavierComputeRatio)
{
    HttpClient httpclient("http://192.168.137.2:8383");
    StubClient c(httpclient);
    //1. Test connected
    if (c.sayHello() == "Server has Connected")
    {
        log(info, c.sayHello());
        //2. Transfer data
        if (ScpTaskfile(path, taskfile))
        {
            log(info, "SCP transfer " + taskfile + " succeed.");
            TaskInfo t_info = TaskInfo_default;
            t_info.t_path = path;
            t_info.t_filename = taskfile;

            //3. forward compute
            time_t time_begin;
            time(&time_begin);

            // use NUC and Xavier
            if (useNUC == 1)
            {
                //NUC part

                if (c.exeAlgorithm("tf_2dcnn", XavierComputeRatio, taskfile) == 1)
                {
                    log(info, "Execution algorithm succeed.");
                    time_t time_end;
                    time(&time_end);
                    t_info.t_time = time_end - time_begin;
                    log(info, "Execution algorithm time: " + to_string(t_info.t_time) + "s");
                    // scp get xml file
                    if(GetXMLfile(taskfile) == 1)
                    {
                        log(info, "SCP get " + taskfile.substr(0, taskfile.length() - 4) + "-Xavier.xml succeed.");
                    }
                }
                else
                {
                    log(info, "Execution algorithm failed.");
                }
            }
            // use Xavier
            else
            {
                if (c.exeAlgorithm("tf_2dcnn", 1, taskfile) == 1)
                {
                    log(info, "Execution algorithm succeed.");
                    time_t time_end;
                    time(&time_end);
                    t_info.t_time = time_end - time_begin;
                    log(info, "Execution algorithm time: " + to_string(t_info.t_time));
                    // scp get xml file
                    if(GetXMLfile(taskfile) == 1)
                    {
                        log(info, "SCP get " + taskfile.substr(0, taskfile.length() - 4) + "-Xavier.xml succeed.");
                    }
                }
                else
                {
                    log(info, "Execution algorithm failed.");
                }
            }
        }
    }
    else
    {
        log(info, "Server has not connected");
    }

    Task_Available = 0;
}

// useNUC=0, just use Xvaier
// useNUC=1, use NUC and Xvaier, and add the compute ratio of Xvaier
int AllocatingTask(string path, string taskfile, int useNUC = 0, double XavierComputeRatio = 1)
{
    if (Task_Available)
    {
        log(info, "Task Available.");
        return 0;
    }
    else
    {
        Task_Available = 1;
        thread t_exetask(ExecuteTask, path, taskfile, useNUC, XavierComputeRatio);
        t_exetask.detach();
    }
}

#endif //TASKSCHEDULING_H_