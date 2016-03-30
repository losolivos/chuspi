/*
 * Copyright (C) 2008-2013 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _WORKERTHREAD_H
#define _WORKERTHREAD_H

#include "Define.h"
#include "MySQLFwd.h"
#include "SQLOperation.h"

#include <ace/Task.h>
#include <ace/Message_Queue.h>

class DatabaseWorker : protected ACE_Task_Base
{
    typedef ACE_Message_Queue_Ex<SQLOperation, ACE_MT_SYNCH> MessageQueueType;

    enum
    {
        HIGH_WATERMARK = 8 * 1024 * 1024,
        LOW_WATERMARK  = 8 * 1024 * 1024
    };

public:
    DatabaseWorker(MySQLConnectionInfo &connectionInfo, uint8 numThreads, MySQLConnectionInitHook initHookFnPtr);
    ~DatabaseWorker();

    int enqueue(SQLOperation *op);

private:
    virtual int svc();

    MessageQueueType m_queue;
    MySQLConnectionInfo &m_connectionInfo;
    MySQLConnectionInitHook m_initHookFnPtr;
};

#endif
