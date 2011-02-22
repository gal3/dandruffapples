#include "messagereader.h"

#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>

#include "except.h"

MessageReader::MessageReader(int fd, size_t initialSize) : _fd(fd), _bufsize(initialSize), _typepos(0), _lenpos(0), _bufpos(0) {
  _buffer = (uint8_t*)malloc(_bufsize);
}

MessageReader::MessageReader(const MessageReader& m) {
  _fd = m._fd;
  _bufsize = m._bufsize;
  _typepos = m._typepos;
  _lenpos = m._lenpos;
  _bufpos = m._bufpos;
  _buffer = (uint8_t*)malloc(_bufsize);
  memcpy(_buffer, m._buffer, _bufpos);
}

MessageReader::~MessageReader() {
  free(_buffer);
}

bool MessageReader::doRead(MessageType *type, int *len, const void **buffer) {
  ssize_t bytes;
  // Read type, if necessary
  if(_typepos < sizeof(_type)) {
    // We're reading a new message; get just its type
    do {
      bytes = read(_fd, &_type + _typepos, sizeof(_type) - _typepos);
    } while(bytes < 0 && errno == EINTR);
    if(bytes < 0) {
      if(errno == EAGAIN || errno == EWOULDBLOCK) {
        return false;
      }
      throw SystemError("Failed to read message type");
    } else if(bytes == 0) {
      throw EOFError();
    }
    
    _typepos += bytes;

    if(_typepos == sizeof(_type)) {
      // No byte reordering necessary because it's one byte.
    } else {
      return false;
    }
  }

  if(_type >= MSG_MAX) {
    throw UnknownMessageError();
  }

  // Read length, if necessary
  if(_lenpos < sizeof(_msglen)) {
    // We're (still) reading a new message; get just its length
    do {
      bytes = read(_fd, &_msglen + _lenpos, sizeof(_msglen) - _lenpos);
    } while(bytes < 0 && errno == EINTR);
    if(bytes < 0) {
      if(errno == EAGAIN || errno == EWOULDBLOCK) {
        return false;
      }
      throw SystemError("Failed to read message size");
    } else if(bytes == 0) {
      throw EOFError();
    }
    
    _lenpos += bytes;

    if(_lenpos == sizeof(_msglen)) {
      // We just finished reading message length
      // Handle byte ordering
      _msglen = ntohl(_msglen);
      // Be sure we have enough space for the entire message
      while(_bufsize < _msglen) {
        _bufsize *= 2;
        _buffer = (uint8_t*)realloc(_buffer, _bufsize);
      }
    } else {
      return false;
    }
  }

  if(_msglen == 0) {
    throw ZeroLengthMessageError();
  }

  // Read message body
  do {
    bytes = read(_fd, _buffer + _bufpos, _msglen - _bufpos);
  } while(bytes < 0 && errno == EINTR);
  if(bytes < 0) {
    if(errno == EAGAIN || errno == EWOULDBLOCK) {
      return false;
    }
    throw SystemError("Failed to read message body");
  } else if(bytes == 0) {
    throw EOFError();
  }

  // Keep track of how much we've read
  _bufpos += bytes;
  
  if(_bufpos == _msglen) {
    // We've got the complete message.
    // Reset internal state to enable reuse
    _typepos = 0;
    _lenpos = 0;
    _bufpos = 0;

    // Make our data available
    if(type) {
      *type = (MessageType)_type;
    }
    *len = _msglen;
    *buffer = _buffer;

    return true;
  }
  
  return false;
}
