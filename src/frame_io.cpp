#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <system_error>
#include <new>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

#include <tensyr/memory.h>

#include <common.hpp>
#include <frame_io.hpp>

namespace Peregrine {
static void MakeFdNonBlocking(int fd) {
  auto flags = fcntl(fd, F_GETFD);
  if (flags < 0) {
    auto error_code = errno;
    std::cerr << "Unable to get flags of fd " << fd << "; " << 
      strerror(error_code) << std::endl;
    std::error_code errc(error_code, std::system_category());
    throw std::system_error(errc);
  }
  if (fcntl(fd, F_SETFD, flags | O_NONBLOCK) == -1) {
    auto error_code = errno;
    std::cerr << "Unable to set flags for fd " << fd << "; " << 
      strerror(error_code) << std::endl;
    std::error_code errc(error_code, std::system_category());
    throw std::system_error(errc);
  }
}

// TODO(jason): Some saner way to shutdown
// Perhaps signal to HALO that this device will never produce more output?
static void Shutdown() {
  kill(getppid(), SIGINT);
}

template <class T>
class RingBuffer {
public:
  RingBuffer(size_t capacity) :
    capacity_(capacity),
    head_(0),
    tail_(capacity_ - 1),
    ring_(new T const *[capacity]) {
      if ((capacity & (capacity - 1))  != 0)  {
        std::cerr << "Ring buffer capacity must be a power of 2" << std::endl;
        raise(SIGABRT);
      }
      memset(ring_, 0, sizeof(T*) * capacity_);
    }

  /// Push a new element into the ring buffer (will overwrite an old element if
  // the buffer is full)
  void Push(const T* obj) {
    auto idx = ++tail_ & (capacity_ - 1);
    if (ring_[idx]) {
      //HaloReleaseReadOnlyObject(ring_[idx]);
      // TODO(jason): remove copying and use HaloFree
      free(const_cast<unsigned char*>(ring_[idx]));
    }
    ring_[idx] = obj;
    tail_ = idx;
  }

  /// Pop an element off the front of the ring buffer
  /// or return nullptr if the ring buffer is empty
  const T* Pop() {
    auto obj = ring_[head_];
    if (obj) {
      ring_[head_] = nullptr;
      head_ = (head_ + 1) & (capacity_ - 1);
    }
    return obj;
  }

private:
  size_t capacity_;
  size_t head_;
  size_t tail_;
  T const **ring_;
};

} // namespace Peregrine

extern "C"{
  // TODO(jason): Use userDataParse in HALO SDK (in next release) to get data from config
#define FRAME_SIZE 691200
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
      Peregrine::MakeFdNonBlocking(fd_);
    }

  unsigned char* ReadFrame() {
    ssize_t bytes_read = read(fd_, pending_buffer_ + bytes_written_, frame_size_ - bytes_written_);
    if (bytes_read == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
      auto error_code = errno;
      std::cerr << "Error reading from fd " << fd_ << "; " << strerror(error_code) << std::endl;
      std::error_code errc(error_code, std::system_category());
      throw std::system_error(errc);
    }
    if (bytes_read == 0) {
      Peregrine::Shutdown();
    }
    bytes_written_ += static_cast<size_t>(bytes_read);
    if (bytes_written_ == frame_size_) {
      auto frame = reinterpret_cast<unsigned char*>(pending_buffer_);
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

class FrameWriter {
public:
  FrameWriter(int fd, uint64_t frame_size) 
    : fd_(fd),
    frame_size_(frame_size),
    pending_frames_(kMaxFramesBuffered),
    current_frame_(nullptr),
    bytes_written_(0) {
      Peregrine::MakeFdNonBlocking(fd_);
    }

  void WriteFrame(const unsigned char* frame) {
    // TODO(jason): Use HaloRetain here instead of copying
    unsigned char* private_frame = new unsigned char[frame_size_];
    memcpy(private_frame, frame, frame_size_);
    pending_frames_.Push(private_frame);
    Drain();
  }

  ~FrameWriter() {
    try {
      // Give it one last try
      Drain();
      if (current_frame_) {
        HaloReleaseReadOnlyObject(current_frame_);
      }
      while (true) {
        auto obj = pending_frames_.Pop();
        if (!obj) { break; }
        HaloReleaseReadOnlyObject(obj);
      }
    } catch (std::exception& e) {
      std::cerr << "Frame Writer failed to shutdown due to " << e.what() << std::endl;
    }
  }

private:
  // TODO: If we're keeping this frame writer, we should run this in a
  // background thread
  void Drain() {
    if (!current_frame_) {
      current_frame_ = pending_frames_.Pop();
    }

    while (current_frame_) {
      auto written = write(fd_, current_frame_ + bytes_written_,
                           frame_size_ - bytes_written_);
      if (written == -1) {
        auto error_code = errno;
        if (error_code == EAGAIN || error_code == EWOULDBLOCK) {
          break;
        } else {
          std::error_code errc(error_code, std::system_category());
          throw std::system_error(errc);
        }
      }
      bytes_written_ += written;
      if (bytes_written_ == frame_size_) {
        //HaloReleaseReadOnlyObject(current_frame_);
        // TODO(jason): remove copying and use HaloFree
        free(const_cast<unsigned char*>(current_frame_));
        current_frame_ = pending_frames_.Pop();
        bytes_written_ = 0;
      }
    }
  }

  static constexpr uint32_t kMaxFramesBuffered = 32;
  const int fd_;
  const uint64_t frame_size_;
  Peregrine::RingBuffer<unsigned char> pending_frames_;
  const unsigned char* current_frame_;
  uint64_t bytes_written_;
};

// Frame Reader
FrameReader* InitFrameReader(const char* data, size_t len) {
  CONSUME_UNUSED(data);
  CONSUME_UNUSED(len);
  try {
    return new FrameReader(INPUT_FD, FRAME_SIZE);
  } catch (std::exception& e) {
    std::cerr << "Frame Reader Init failed due to " << e.what() << std::endl;
    // TODO(jason): Signal to the HALO runtime that this device can not recover
    return nullptr;
  }
}

unsigned char* ReadFrame(FrameReader* reader) {
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
FrameWriter* InitFrameWriter(const char* data, size_t len) {
  CONSUME_UNUSED(data);
  CONSUME_UNUSED(len);
  try {
    return new FrameWriter(OUTPUT_FD, FRAME_SIZE);
  } catch (std::exception& e) {
    std::cerr << "Frame Writer Init failed due to " << e.what() << std::endl;
    // TODO(jason): Signal to the HALO runtime that this device can not recover
    return nullptr;
  }
}

void WriteFrame(FrameWriter* writer, unsigned char* frame) {
  try {
    return writer->WriteFrame(reinterpret_cast<unsigned char*>(frame));
  } catch (std::exception& e) {
    std::cerr << "Frame Writer failed due to " << e.what() << std::endl;
  }
}

void FiniFrameWriter(FrameWriter* writer) {
  delete writer;
}
}
