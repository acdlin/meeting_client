#ifndef NETHEADER_H
#define NETHEADER_H

#include <QtGlobal>
#include <QByteArray>


enum class msgType: quint16
{
    IMG_SEND = 0,
    IMG_RECV,
    AUDIO_SEND,
    AUDIO_RECV,
    TEXT_SEND,
    TEXT_RECV,
    CREATE_MEETING,
    JOIN_MEETING,
    CLOSE_CAMERA,
    EXIT_MEETING,

    CREATE_MEETING_RESPONSE = 20,
    PARTNER_EXIT = 21,
    PARTNER_JOIN = 22,
    JOIN_MEETING_RESPONSE = 23,
    PARTNER_JOIN2 = 24,
    RemoteHostClosedError = 40,
    OtherNetError = 41
};

/*
 * $            1
 * type         2
 * ip           4
 * len          4
 * data         len
 * #            1
 *
 */

struct MESG
{
    msgType msg_type;
    quint32 ip;
    QByteArray data;
};

QByteArray packMessage(const MESG& msg);
#endif // NETHEADER_H
