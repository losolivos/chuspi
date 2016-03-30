#include "MySQLHelper.h"
#include <mysql.h>

namespace MySQLHelper {

void startLibrary()
{
    mysql_library_init(0, nullptr, nullptr);
}

void stopLibrary()
{
    mysql_library_end();
}

void stopThread()
{
    mysql_thread_end();
}

bool libraryThreadSafe()
{
    return mysql_thread_safe() == 1;
}

} // namespace MySQLHelper
