#ifndef _MESSAGEREADER_H_
#define _MESSAGEREADER_H_

#include <sys/types.h>
#include <stdint.h>

#include "types.h"

class MessageReader {
protected:
  int _fd;
  uint8_t *_buffer;
  size_t _bufsize, _typepos, _lenpos, _bufpos;
  uint16_t _msglen;
  uint8_t _type;
  
public:
  MessageReader(int fd, size_t initialSize = 256);
  MessageReader(const MessageReader&);
  ~MessageReader();

  bool doRead(MessageType *type, size_t *len, const void **buffer);
};

#endif
