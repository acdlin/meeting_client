#include "netheader.h"
#include <QtEndian>
QByteArray packMessage(const MESG& msg)
{
    QByteArray ret ;
    int byte_len = 1 + 2 + 4  + 4  + msg.data.size() + 1 ;
    ret.reserve(byte_len);

    ret.append('$');
    quint16 n_type = qToBigEndian(static_cast<quint16>(msg.msg_type));
    ret.append(reinterpret_cast<const char *>(&n_type), sizeof(n_type));
    quint32 n_ip = qToBigEndian(msg.ip);
    ret.append(reinterpret_cast<const char *>(&n_ip) , sizeof(n_ip));
    quint32 n_size = qToBigEndian(static_cast<quint32>(msg.data.size()));
    ret.append(reinterpret_cast<const char *>(&n_size),sizeof(n_size));
    ret.append(msg.data);
    ret.append('#');
    return ret;
}
