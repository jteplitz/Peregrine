#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <system_error>
#include <new>

#include <tensyr/memory.h>

#include <frame_io.hpp>

extern "C"{
  // TODO(jason): Use userDataParse in HALO SDK (in next release) to get data from config
#define FRAME_SIZE 420
#define INPUT_FD 0
#define OUTPUT_FD 1

struct FrameReader {
  FrameReader(int fd, uint64_t frame_size) :
    fd_(fd),
    pending_buffer_(static_cast<unsigned char*>(HaloMalloc(frame_size))),
    frame_size_(frame_size),
    bytes_written_(0) {
      if (pending_buffer_ == nullptr) { throw std::bad_alloc(); }
      // Make the fd non-blocking
      auto flags = fcntl(fd, F_GETFD);
      if (flags < 0) {
        auto error_code = errno;
        std::cerr << "Unable to get flags of fd " << fd_ << "; " << 
          strerror(error_code) << std::endl;
        std::error_code errc(error_code, std::system_category());
        throw std::system_error(errc);
      }
      if (fcntl(fd, F_SETFD, flags | O_NONBLOCK) == -1) {
        auto error_code = errno;
        std::cerr << "Unable to set flags for fd " << fd_ << "; " << 
          strerror(error_code) << std::endl;
        std::error_code errc(error_code, std::system_category());
        throw std::system_error(errc);
      }
    }

  float* ReadFrame() {
    ssize_t bytes_read = read(fd_, pending_buffer_ + bytes_written_, frame_size_ - bytes_written_);
    if (bytes_read == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
      auto error_code = errno;
      std::cerr << "Error reading from fd " << fd_ << "; " << strerror(error_code) << std::endl;
      std::error_code errc(error_code, std::system_category());
      throw std::system_error(errc);
    }
    bytes_written_ += static_cast<size_t>(bytes_read);
    if (bytes_written_ == frame_size_) {
      std::cout << "Got a frame!" << std::endl;
      auto frame = reinterpret_cast<float*>(pending_buffer_);
      pending_buffer_ = static_cast<unsigned char*>(HaloMalloc(frame_size_));
      if (pending_buffer_ == nullptr) {
        throw std::bad_alloc();
      }
      bytes_written_ = 0;
      return frame;
    }
    return nullptr;
  }

  ~FrameReader() {
    HaloFree(pending_buffer_);
    close(fd_);
  }

  int fd_;
  unsigned char* pending_buffer_; // buffer of size frame_size
  const uint64_t frame_size_;
  uint64_t bytes_written_; // Number of bytes written in buffer
};

// Frame Reader
FrameReader* InitFrameReader(const char* data, size_t len) {
  try {
    return new FrameReader(INPUT_FD, FRAME_SIZE);
  } catch (std::exception& e) {
    std::cerr << "Frame Reader Init failed due to " << e.what() << std::endl;
    // TODO(jason): Signal to the HALO runtime that this device can not recover
    return nullptr;
  }
}

float* ReadFrame(FrameReader* reader) {
  try {
    return reader->ReadFrame();
  } catch (std::exception& e) {
    std::cerr << "Frame Reader failed due to " << e.what() << std::endl;
    // TODO(jason): If unrecoverable, signal to the HALO runtime.
    return nullptr;
  }
}

void FiniFrameReader(FrameReader* reader) {
  delete reader;
}

// Frame Writer
void* InitFrameWriter(const char*, size_t) {
  return nullptr;
}

void WriteFrame(void*, float*) {

}

void FiniFrameWriter(void*) {

}
}
