#ifndef _TYPES_H_
#define _TYPES_H_

enum MessageType {
  MSG_TIMESTEPUPDATE,
  MSG_TIMESTEPDONE,
  MSG_REGIONINFO,
  MSG_REGIONVIEW,
  MSG_SENDMOREWORLDVIEWS,
  MSG_WORLDINFO,
  MSG_SERVERROBOT,
  MSG_CLIENTROBOT,
  MSG_PUCKSTACK,
  MSG_CLAIMTEAM,
  MSG_CLAIM,
  MSG_BOUNCEDROBOT,
  MSG_MAX
};

#endif
