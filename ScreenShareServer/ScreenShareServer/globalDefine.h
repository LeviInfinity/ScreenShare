#ifndef GLOBALDEFINE_H
#define GLOBALDEFINE_H
enum MSGTYPE{ _TXT, _FILE };
typedef struct _header
{
	char msg[16];
	char fileName[64];
	long totalLen;
	MSGTYPE msg_type;
}Header;

#define HEADLEN  (sizeof(long) + 64 + 16 +  sizeof(MSGTYPE))

#endif