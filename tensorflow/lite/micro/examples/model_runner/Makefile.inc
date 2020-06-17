

OUTPUT_HANDLER_TEST_SRCS := \
tensorflow/lite/micro/examples/model_runner/output_handler.cc \
tensorflow/lite/micro/examples/model_runner/output_handler_test.cc

OUTPUT_HANDLER_TEST_HDRS := \
tensorflow/lite/micro/examples/model_runner/output_handler.h


model_runner_SRCS := \
tensorflow/lite/micro/examples/model_runner/main.cc \
tensorflow/lite/micro/examples/model_runner/main_functions.cc \
tensorflow/lite/micro/examples/model_runner/micro_api.cc \
tensorflow/lite/micro/examples/model_runner/model.cc \
tensorflow/lite/micro/examples/model_runner/output_handler.cc

model_runner_HDRS := \
tensorflow/lite/micro/examples/model_runner/main_functions.h \
tensorflow/lite/micro/examples/model_runner/model.h \
tensorflow/lite/micro/examples/model_runner/output_handler.h \
tensorflow/lite/micro/examples/model_runner/micro_api.h \

#Find any platform - specific rules for this example.
include $(wildcard tensorflow/lite/micro/examples/model_runner/*/Makefile.inc)

# Tests the output handler
$(eval $(call microlite_test,gesture_output_handler_test,\
$(OUTPUT_HANDLER_TEST_SRCS),$(OUTPUT_HANDLER_TEST_HDRS)))

# Builds a standalone binary
$(eval $(call microlite_test,model_runner,\
$(model_runner_SRCS),$(model_runner_HDRS)))
