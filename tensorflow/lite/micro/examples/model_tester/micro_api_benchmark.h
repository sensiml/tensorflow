
#ifndef TENSORFLOW_LITE_MICRO_API_BENCHMARK_H_
#define TENSORFLOW_LITE_MICRO_API_BENCHMARK_H_

#include "tensorflow/lite/micro/examples/model_tester/micro_api.h"
#include "tensorflow/lite/schema/schema_generated.h"



extern "C" {
int __exidx_start(){return -1;}; //some library wants these set, note sure what, exceptions should be disabled. https://forum.pjrc.com/threads/57192-Teensy-4-0-linker-issues-with-STL-libraries
int __exidx_end(){return -1;};
}

static bool set_static = false;


static int find_arena_size(const unsigned char* tflite_buffer, uint8_t * tensor_arena, int* arena_size_p, size_t a_low,
                           size_t a_high, size_t delta) {


  int low = a_low;
  int high = a_high;
  int curr = low + ((high - low) / 2);
  int last_success = high;
  int steps = 0;
  int ret;

  while (steps < 100) {
    ++steps;
    ret = tf_micro_model_setup(tflite_buffer, tensor_arena, curr, set_static);
    printf("%d %d %d\n", ret, last_success, curr);

    if (ret == success) {
      last_success = curr;
    }

    if (ret == allocate_failed) {  // too low
      low = curr;
      curr = low + ((high - low) /2);
    } else {
      high = curr;
      curr = high - ((high - low) /2);
    }

    if (low == curr || high == curr || (high - low) < delta) {
      break;
    }

    if (last_success >= a_high || steps >= 99 ) {
      return allocate_failed;
    }

  }

  *arena_size_p = last_success;

  return success;
}


static int test_invoke(const unsigned char* tflite_buffer, uint8_t* tensor_arena,  int arena_size,
                       float* input_data, int num_inputs, float* results,
                       int num_outputs) {
  int ret = success;


  if (set_static == false)
  {
    ret = tf_micro_model_setup(tflite_buffer, tensor_arena, arena_size, true);

    set_static=true;
    
    if (ret != success) {
      return ret;
    }
  }

  ret = tf_micro_model_invoke(input_data, num_inputs, results, num_outputs);

  return ret;
}


#endif  // TENSORFLOW_LITE_MICRO_API_BENCHMARK_H_