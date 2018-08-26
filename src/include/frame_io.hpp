#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif
  // Forward declare FrameReader since it's a C++ types.
  // Definition is in src/frame_io.cpp
  typedef struct FrameReader FrameReader;
  typedef struct FrameWriter FrameWriter;

  // Frame Reader
  FrameReader* InitFrameReader(const char*, size_t);
  float* ReadFrame(FrameReader*);
  void FiniFrameReader(FrameReader*);

  // Frame Writer
  FrameWriter* InitFrameWriter(const char*, size_t);
  void WriteFrame(FrameWriter*, float*);
  void FiniFrameWriter(FrameWriter*);

#ifdef __cplusplus
}
#endif
