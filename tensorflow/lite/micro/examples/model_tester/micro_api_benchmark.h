
#ifndef TENSORFLOW_LITE_MICRO_API_BENCHMARK_H_
#define TENSORFLOW_LITE_MICRO_API_BENCHMARK_H_

#include "tensorflow/lite/schema/schema_generated.h"



int find_arena_size(const unsigned char* tflite_buffer, uint8_t * tensor_arena, int* arena_size_p, size_t a_low,
                           size_t a_high, size_t delta);


#endif  // TENSORFLOW_LITE_MICRO_API_BENCHMARK_H_