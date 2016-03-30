#ifndef MYSQL_HELPER_H
#define MYSQL_HELPER_H

namespace MySQLHelper {

void startLibrary();
void stopLibrary();
void stopThread();
bool libraryThreadSafe();

} // namespace MySQLHelper

#endif // MYSQL_HELPER_H
