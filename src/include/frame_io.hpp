#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

  // Frame Reader
  void* InitFrameReader(const char*, size_t);
  float* ReadFrame(void*);
  void FiniFrameReader(void*);

  // Frame Writer
  void* InitFrameWriter(const char*, size_t);
  void WriteFrame(void*, float*);
  void FiniFrameWriter(void*);

#ifdef __cplusplus
}
#endif
