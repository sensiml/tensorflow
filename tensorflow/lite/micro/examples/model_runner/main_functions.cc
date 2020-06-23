/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "tensorflow/lite/micro/examples/model_runner/micro_api.h"
#include "tensorflow/lite/micro/examples/model_runner/model.h"
#include "tensorflow/lite/micro/examples/model_runner/test_data.h"
#include "tensorflow/lite/micro/examples/model_runner/memory_benchmark.h"
#include "tensorflow/lite/micro/examples/model_runner/output_handler.h"
#include "tensorflow/lite/micro/debug_log.h"

// Globals, used for compatibility with Arduino-style sketches.
namespace {
// Create an area of memory to use for input, output, and intermediate arrays.\
// The size of this will depend on the model you're using, and may need to be
// determined by experimentation.
constexpr int kTensorArenaSize_max = 60 * 1024;
constexpr int kTensorArenaSize_min = 2 * 1024;
constexpr int kTensorArenaSize_tolerance = 1024;
unsigned char tensor_arena[kTensorArenaSize_max];
float tf_results[MODEL_OUTPUTS];
}  // namespace

// The name of this function is important for Arduino compatibility.
void setup() {
  
  int arena_size = kTensorArenaSize_max;
  int* arena_size_p = &arena_size;
  int ret;

  ret = find_arena_size(g_model, tensor_arena, arena_size_p,
                        kTensorArenaSize_min, kTensorArenaSize_max,
                        kTensorArenaSize_tolerance);

  if (ret == 1) {
   DebugLog("ALLOCATION FAILED.\n");
  }
  else if (ret == 2) {
    DebugLog("UNSUPPORTED VERSION.\n");
  }
  
  model_setup(g_model, tensor_arena, arena_size);
}

void loop() {
  int result_index=0;
  float max_value = 0.0f;

  tflite::ErrorReporter* error_reporter = (tflite::ErrorReporter *)get_micro_api_error_reporter();
  
  for (int index = 0; index < TEST_DATA_LENGTH; index++) {
    for (int i = 0; i < MODEL_OUTPUTS; i++) 
    {
        tf_results[i] = 0.0;
    }
    model_invoke(test_data[index], MODEL_INPUTS, results, MODEL_OUTPUTS);

    max_value = results[0];
    result_index=0;
    for (int i = 1; i < MODEL_OUTPUTS; i++) 
    {
      if (results[i] > max_value)
      {
        max_value = results[i];
        result_index = i;
      }
    }
    // Produce an output
    HandleOutput(error_reporter, result_index);
  }
   DebugLog("ALL_TESTS_PASSED\n");
}
