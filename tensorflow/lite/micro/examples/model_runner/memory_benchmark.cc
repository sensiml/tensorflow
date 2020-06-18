#include "tensorflow/lite/micro/examples/model_runner/memory_benchmark.h"
#include "tensorflow/lite/micro/examples/model_runner/output_handler.h"

#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"

#include "tensorflow/lite/version.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"

// Globals, used for compatibility with Arduino-style sketches.
namespace {
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
TfLiteTensor* model_input = nullptr;
TfLiteTensor* model_output = nullptr;
}  // namespace

// The name of this function is important for Arduino compatibility.
int model_verify_allocate(const void* model_data, uint8_t* tensor_arena, int kTensorArenaSize) {
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

  // Only Pull in functions that are needed by the model
  static tflite::MicroMutableOpResolver<8> resolver;
    resolver.AddBuiltin(tflite::BuiltinOperator_CONV_2D, tflite::ops::micro::Register_CONV_2D());
    resolver.AddBuiltin(tflite::BuiltinOperator_DEQUANTIZE,
             tflite::ops::micro::Register_DEQUANTIZE());
    resolver.AddBuiltin(tflite::BuiltinOperator_FULLY_CONNECTED,
             tflite::ops::micro::Register_FULLY_CONNECTED());
    resolver.AddBuiltin(tflite::BuiltinOperator_MAX_POOL_2D,
             tflite::ops::micro::Register_MAX_POOL_2D());
    resolver.AddBuiltin(tflite::BuiltinOperator_QUANTIZE, tflite::ops::micro::Register_QUANTIZE());
    resolver.AddBuiltin(tflite::BuiltinOperator_RELU, tflite::ops::micro::Register_RELU());
    resolver.AddBuiltin(tflite::BuiltinOperator_RESHAPE, tflite::ops::micro::Register_RESHAPE());
    resolver.AddBuiltin(tflite::BuiltinOperator_SOFTMAX, tflite::ops::micro::Register_SOFTMAX());

  tflite::MicroInterpreter interpreter(
      model, resolver, tensor_arena, kTensorArenaSize, error_reporter);

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter.AllocateTensors();

  if (allocate_status != kTfLiteOk) {
    TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed for tensor size %d", kTensorArenaSize);
    return 1;
  }


  return 0;  
}


int find_arena_size(const unsigned char* tflite_buffer, uint8_t * tensor_arena, int* arena_size_p, size_t a_low,
                           size_t a_high, size_t delta) {
                            

    int low = a_low;
    int high = a_high;
    int curr = low + ((high - low) / 2);
    int last_success = high;
    int steps = 0;
    int ret;

  while (steps < 100) {
    ++steps;
    ret = model_verify_allocate(tflite_buffer, tensor_arena, curr);
    TF_LITE_REPORT_ERROR(error_reporter, "%d %d %d", ret, last_success, curr);

    if (ret == 0) {
      last_success = curr;
    }

    if (ret == 1) {  // too low
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
      return 1;
    }

  }

  *arena_size_p = last_success;



    TF_LITE_REPORT_ERROR(error_reporter, "FOUND TENSOR SIZE: %d", last_success);

  return 0;
}