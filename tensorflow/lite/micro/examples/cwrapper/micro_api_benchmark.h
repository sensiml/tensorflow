
#ifndef TENSORFLOW_LITE_MICRO_API_BENCHMARK_H_
#define TENSORFLOW_LITE_MICRO_API_BENCHMARK_H_

#include "tensorflow/lite/micro/examples/cwrapper/micro_api.h"
#include "tensorflow/lite/schema/schema_generated.h"

static bool set_static=false;
static int test_allocate(const unsigned char* tflite_buffer, int arena_size) {
  int ret;

  uint8_t* tensor_arena = (uint8_t*)malloc(arena_size);
  if (!tensor_arena) {
    return malloc_failed;
  }

  ret = tf_micro_model_setup(tflite_buffer, tensor_arena, arena_size, set_static);

  free(tensor_arena);

  return ret;
}

static int test_invoke(const unsigned char* tflite_buffer, uint8_t* tensor_arena,  int arena_size,
                       float* input_data, int num_inputs, float* results,
                       int num_outputs) {
  int ret;


  if (set_static == false)
    ret = tf_micro_model_setup(tflite_buffer, tensor_arena, arena_size, true);

    set_static=true;
    
    if (ret != success) {
      printf("MODEL FAILED TO BE SET UP %d\n", ret);
      return ret;
    }

  ret = tf_micro_model_invoke(input_data, num_inputs, results, num_outputs);

  return ret;
}

static int find_arena_size(const unsigned char* tflite_buffer, int* arena_size_p, size_t a_low,
                           size_t a_high) {
  int low = a_low;
  int high = a_high;
  int curr = low + ((high - low) / 2);
  int last_success = high;
  int steps = 0;

  while (steps < 100) {
    ++steps;
    int r = test_allocate(tflite_buffer, curr);
    printf("%d %d %d\n", r, last_success, curr);

    if (r == success) {
      last_success = curr;
    }

    if (r == allocate_failed) {  // too low
      low = curr;
      curr = low + ((high - low) / 2);
    } else {
      high = curr;
      curr = low + ((high - low) / 2);
    }

    if (low == curr || high == curr) {
      break;
    }

    if (last_success == a_high || steps >= 99) {
      return allocate_failed;
    }

  }

  *arena_size_p = last_success;

  return success;
}

#endif  // TENSORFLOW_LITE_MICRO_API_BENCHMARK_H_