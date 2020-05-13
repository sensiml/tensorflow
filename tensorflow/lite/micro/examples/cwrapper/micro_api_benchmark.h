
#ifndef TENSORFLOW_LITE_MICRO_API_BENCHMARK_H_
#define TENSORFLOW_LITE_MICRO_API_BENCHMARK_H_

#include "tensorflow/lite/micro/examples/cwrapper/micro_api.h"
#include "tensorflow/lite/schema/schema_generated.h"

static int test_allocate(const unsigned char* tflite_buffer, size_t arena_size) {
  int ret;

  uint8_t* tensor_arena = (uint8_t*)malloc(arena_size);
  if (!tensor_arena) {
    return malloc_failed;
  }

  ret = tf_micro_model_setup(tflite_buffer, tensor_arena, arena_size);

  free(tensor_arena);

  return ret;
}

static int test_invoke(const unsigned char* tflite_buffer, size_t arena_size,
                       float* input_data, int num_inputs, float* results,
                       int num_outputs) {
  int ret;

  uint8_t* tensor_arena = (uint8_t*)malloc(arena_size);

  if (!tensor_arena) {
    return malloc_failed;
  }

  ret = tf_micro_model_setup(tflite_buffer, tensor_arena, arena_size);

  if (ret != success) {
    return ret;
  }

  ret = tf_micro_model_invoke(input_data, num_inputs, results, num_outputs);

      free(tensor_arena);

  return ret;
}

static int find_arena_size(const unsigned char* tflite_buffer, int* arena_size, size_t a_low,
                           size_t a_high) {
  size_t low = a_low;
  size_t high = a_high;
  size_t curr = low + ((high - low) / 2);
  size_t last_success = high;
  size_t steps = 0;

  while (steps < 100) {
    ++steps;
    int r = test_allocate(tflite_buffer, curr);

    if (r != success) {
      last_success = curr;
    }

    if (r == invoke_failed) {  // too low
      low = curr;
      curr = low + ((high - low) / 2);
    } else {
      high = curr;
      curr = low + ((high - low) / 2);
    }

    if (low == curr || high == curr) {
      break;
    }
  }

  if (last_success == a_high || steps >= 99) {
    return allocate_failed;
  }

  return last_success;
}

#endif  // TENSORFLOW_LITE_MICRO_API_BENCHMARK_H_