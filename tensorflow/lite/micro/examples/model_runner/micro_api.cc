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

#include "tensorflow/lite/micro/examples/model_runner/main_functions.h"
#include "tensorflow/lite/micro/examples/model_runner/model.h"
#include "tensorflow/lite/micro/examples/model_runner/output_handler.h"
#include "tensorflow/lite/micro/examples/test_data.h"

#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"


#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"

// Globals, used for compatibility with Arduino-style sketches.
namespace {
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* model_input = nullptr;
int input_length;
int test_vector;

// Create an area of memory to use for input, output, and intermediate arrays.
// The size of this will depend on the model you're using, and may need to be
// determined by experimentation.
constexpr int kTensorArenaSize = 60 * 1024;
uint8_t tensor_arena[kTensorArenaSize];
}  // namespace

// The name of this function is important for Arduino compatibility.
void setup() {
  // Set up logging. Google style is to avoid globals or statics because of
  // lifetime uncertainty, but since this has a trivial destructor it's okay.
  static tflite::MicroErrorReporter micro_error_reporter;  // NOLINT
  error_reporter = &micro_error_reporter;

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(g_model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Model provided is schema version %d not equal "
                         "to supported version %d.",
                         model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  // Only Pull in functions that are needed by the model
  static tflite::MicroMutableOpResolver<4> resolver;
    resolver.AddBuiltin(tflite::BuiltinOperator_DEQUANTIZE,
             tflite::ops::micro::Register_DEQUANTIZE());
    resolver.AddBuiltin(tflite::BuiltinOperator_FULLY_CONNECTED,
             tflite::ops::micro::Register_FULLY_CONNECTED());
    resolver.AddBuiltin(tflite::BuiltinOperator_QUANTIZE, tflite::ops::micro::Register_QUANTIZE());
    resolver.AddBuiltin(tflite::BuiltinOperator_RELU, tflite::ops::micro::Register_RELU());

  static tflite::MicroInterpreter static_interpreter(
      model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors.
  interpreter->AllocateTensors();

  // Obtain pointer to the model's input tensor.
  model_input = interpreter->input(0);
  
  return;
  }

  input_length = model_input->bytes / sizeof(float);
  
}

void loop() {
  // Attempt to read new data from the accelerometer.


  for (int i =0; i< input_length; i++)
  {
    model_input->data.f[i] = test_data[test_vector][i];
  }
  
  // Run inference, and report any error.
  TfLiteStatus invoke_status = interpreter->Invoke();

  if (invoke_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed on index: %d\n",
                         begin_index);
    return;
  }

  // Produce an output
  HandleOutput(error_reporter, gesture_index);

  if (test_vector >= TEST_DATA_LENGTH)
  {
    test_vector=0;
  }
}
