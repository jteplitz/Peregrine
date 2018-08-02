#include <frame_io.hpp>

extern "C"{
  // Frame Reader
  void* InitFrameReader(const char* data, size_t len) {
    return nullptr;
  }

  float* ReadFrame(void* state) {
    return nullptr;
  }

  void FiniFrameReader(void* state) {
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
