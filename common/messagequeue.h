#ifndef _MESSAGEQUEUE_H_
#define _MESSAGEQUEUE_H_

#include <stdexcept>

#include <sys/types.h>
#include <stdint.h>
#include <cstdlib>
#include <unistd.h>
#include <cerrno>
#include <arpa/inet.h>

#include <google/protobuf/message_lite.h>

#include "types.h"

class MessageQueue {
protected:
  int _fd;
  int _bufsize, _appendpt, _writept;
  uint8_t *_buffer;

public:
  MessageQueue(int fd, size_t prealloc = 256);
  MessageQueue(const MessageQueue&);
  ~MessageQueue();

  void push(MessageType typeTag, const google::protobuf::MessageLite &message);

  bool doWrite();
  inline int remaining() const { return _appendpt - _writept; }
  inline bool writing() const { return remaining(); }
};

#endif
