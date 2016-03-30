#include "DatabaseWorker.h"
#include "MySQLConnection.h"
#include "MySQLConnectionInfo.h"
#include "MySQLHelper.h"

DatabaseWorker::DatabaseWorker(MySQLConnectionInfo &connectionInfo, uint8 numThreads, MySQLConnectionInitHook initHookFnPtr)
    : m_queue(HIGH_WATERMARK, LOW_WATERMARK)
    , m_connectionInfo(connectionInfo)
    , m_initHookFnPtr(initHookFnPtr)
{
    ACE_Task_Base::activate(THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED, numThreads);
}

DatabaseWorker::~DatabaseWorker()
{
    m_queue.deactivate();
    wait();
}

int DatabaseWorker::enqueue(SQLOperation *op)
{
    return m_queue.enqueue(op);
}

int DatabaseWorker::svc()
{
    MySQLConnection thrConn;
    if (!thrConn.Open(m_connectionInfo, m_initHookFnPtr))
        return 0;

    while (1)
    {
        SQLOperation *request;
        if (m_queue.dequeue(request) == -1)
            break;
        request->execute(&thrConn);
        delete request;
    }

    MySQLHelper::stopThread();
    return 0;
}
