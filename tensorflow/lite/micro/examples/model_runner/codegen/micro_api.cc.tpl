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

#include "tensorflow/lite/micro/micro_api.h"
#include "tensorflow/lite/micro/examples/model_runner/output_handler.h"


#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
//FILL_MICRO_MUTABLE_OPS_RESOLVER_HEADER

// Globals, used for compatibility with Arduino-style sketches.
namespace {
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* model_input = nullptr;
TfLiteTensor* model_output = nullptr;

// An area of memory to use for input, output, and intermediate arrays.
constexpr int kTensorArenaSize = 48 * 1024;
//FILL_TENSOR_ARENA_SIZE
uint8_t tensor_arena[kTensorArenaSize]  __attribute__ ((aligned (16)));
}  // namespace

// The name of this function is important for Arduino compatibility.
int micro_model_setup(const void* model_data) {
  // Set up logging. Google style is to avoid globals or statics because of
  // lifetime uncertainty, but since this has a trivial destructor it's okay.
  static tflite::MicroErrorReporter micro_error_reporter;  // NOLINT
  error_reporter = &micro_error_reporter;

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    TF_LITE_REPORT_ERROR(error_reporter,
                         "Model provided is schema version %d not equal "
                         "to supported version %d.",
                         model->version(), TFLITE_SCHEMA_VERSION);
    return 1;
  }

//FILL_MICRO_MUTABLE_OPS_RESOLVER

  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();

  if (allocate_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "Allocate failed for tensor size %d", kTensorArenaSize);
    return 2;
  }

  TF_LITE_REPORT_ERROR(error_reporter, "FOUND TENSOR SIZE: %d", interpreter->arena_used_bytes());

  // Obtain pointer to the model's input tensor.
  model_input = interpreter->input(0);
  model_output = interpreter->output(0);

  return 0;


}

int micro_model_invoke(float* input_data, int num_inputs, float* results, int num_outputs) {

  for (int i =0; i< num_inputs; i++)
  {
    model_input->data.f[i] = input_data[i];
  }

  // Run inference, and report any error.
  TfLiteStatus invoke_status = interpreter->Invoke();

  if (invoke_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed on index");
    return 1;
  }

  // Read the predicted y value from the model's output tensor
  for (int i=1; i< num_outputs; i++ )
  {
      results[i] = model_output->data.f[i];
  }

  return 0;

}


void * get_micro_api_error_reporter()
{
  return error_reporter;
}


