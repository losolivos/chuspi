#include "MySQLConnectionInfo.h"
#include "Util.h"

MySQLConnectionInfo::MySQLConnectionInfo(std::string const &infoString)
{
    Tokenizer tokens(infoString, ';');

    if (tokens.size() != 5)
        return;

    uint8 i = 0;

    host.assign(tokens[i++]);
    port_or_socket.assign(tokens[i++]);
    user.assign(tokens[i++]);
    password.assign(tokens[i++]);
    database.assign(tokens[i++]);
}
