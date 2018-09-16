#pragma once
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

// Header before a list of boxes in memory
typedef struct Boxes {
  uint64_t num_boxes;
} Boxes;

typedef struct Box {
  int x;
  int y;
  int width;
  int height;
} Box;

Boxes* DetectMotion(const unsigned char* frame);
unsigned char* DrawBoxes(unsigned char* frame, const Boxes* boxes);

#ifdef __cplusplus
}
#endif
