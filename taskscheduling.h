// Author: SUMINGRUI

#ifndef TASKSCHEDULING_H_
#define TASKSCHEDULING_H_

#include <string>
#include <thread>
#include "share.h"
#include "stubclient.h"
#include <jsonrpccpp/client/connectors/httpclient.h>

using namespace std;
using namespace jsonrpc;

// Do not support task queue
int Task_Assignment_Model = 1; // 1: only Xvaier 2: Xvaier and NUC
int Task_Available = 0;

struct TaskInfo
{
    string path;
    string filename;
    string result;
} TaskInfo_default = {"", "", ""};

void ExecuteTask(string taskfile)
{
    HttpClient httpclient("http://192.168.137.2:8383");
    StubClient c(httpclient);
    if (c.sayHello()=="Server has Connected")
    {
        log(info,c.sayHello());
    }
    // try
    // {
    //     log(info,c.sayHello());
    // }
    // catch (JsonRpcException e)
    // {
    //     log(error,e.what());
    // }
    Task_Available = 0;
}

int AllocatingTask(string taskfile)
{
    if (Task_Available)
    {
        log(info, "Task Available.");
        return 0;
    }
    else
    {
        Task_Available = 1;
        thread t_exetask(ExecuteTask,taskfile);
        t_exetask.detach();
    }
}



#endif //TASKSCHEDULING_H_