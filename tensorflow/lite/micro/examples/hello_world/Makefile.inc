HELLO_WORLD_TEST_SRCS := \
tensorflow/lite/micro/examples/hello_world/hello_world_test.cc \
tensorflow/lite/micro/examples/hello_world/sine_model_data.cc

HELLO_WORLD_TEST_HDRS := \
tensorflow/lite/micro/examples/hello_world/sine_model_data.h

OUTPUT_HANDLER_TEST_SRCS := \
tensorflow/lite/micro/examples/hello_world/output_handler_test.cc \
tensorflow/lite/micro/examples/hello_world/output_handler.cc

OUTPUT_HANDLER_TEST_HDRS := \
tensorflow/lite/micro/examples/hello_world/output_handler.h \
tensorflow/lite/micro/examples/hello_world/constants.h

HELLO_WORLD_SRCS := \
tensorflow/lite/micro/examples/hello_world/main.cc \
tensorflow/lite/micro/examples/hello_world/main_functions.cc \
tensorflow/lite/micro/examples/hello_world/sine_model_data.cc \
tensorflow/lite/micro/examples/hello_world/output_handler.cc \
tensorflow/lite/micro/examples/hello_world/constants.cc

HELLO_WORLD_HDRS := \
tensorflow/lite/micro/examples/hello_world/sine_model_data.h \
tensorflow/lite/micro/examples/hello_world/output_handler.h \
tensorflow/lite/micro/examples/hello_world/constants.h \
tensorflow/lite/micro/examples/hello_world/main_functions.h

#Find any platform - specific rules for this example.
include $(wildcard tensorflow/lite/micro/examples/hello_world/*/Makefile.inc)

# Tests loading and running the sine model.
$(eval $(call microlite_test,hello_world_test,\
$(HELLO_WORLD_TEST_SRCS),$(HELLO_WORLD_TEST_HDRS)))

# Tests producing an output.
$(eval $(call microlite_test,output_handler_test,\
$(OUTPUT_HANDLER_TEST_SRCS),$(OUTPUT_HANDLER_TEST_HDRS)))



THIRD_PARTY_CC_SRCS += $(filter $(MAKEFILE_DIR)/downloads/%, $(MICROLITE_CC_SRCS))
MICROLITE_CC_SRCS := $(filter-out $(THIRD_PARTY_CC_SRCS), $(MICROLITE_CC_SRCS))
THIRD_PARTY_CC_SRCS := $(patsubst  $(MAKEFILE_DIR)/downloads/%, third_party/%,  $(THIRD_PARTY_CC_SRCS))

GENERATED_PROJECT_INCLUDES += $(filter -I$(MAKEFILE_DIR)/downloads/%, $(INCLUDES))
GENERATED_PROJECT_INCLUDES += $(patsubst  -I$(MAKEFILE_DIR)/downloads/%, -Ithird_party/%,  $(GENERATED_PROJECT_INCLUDES))
GENERATED_PROJECT_INCLUDES += $(filter -isystem$(MAKEFILE_DIR)/downloads/%, $(INCLUDES))
GENERATED_PROJECT_INCLUDES += $(patsubst  -isystem$(MAKEFILE_DIR)/downloads/%, -isystemthird_party/%,  $(GENERATED_PROJECT_INCLUDES))

MICROLITE_LIBS := $(patsubst  $(MAKEFILE_DIR)/downloads/%, $(TENSORFLOW_ROOT)$(MAKEFILE_DIR)/downloads/%, $(MICROLITE_LIBS))
LDFLAGS :=  $(patsubst  $(MAKEFILE_DIR)/downloads/%, $(TENSORFLOW_ROOT)$(MAKEFILE_DIR)/downloads/%, $(LDFLAGS))

THIRD_PARTY_CC_HDRS += $(filter $(MAKEFILE_DIR)/downloads/%, $(MICROLITE_CC_HDRS))
MICROLITE_CC_SRCS := $(filter-out $(THIRD_PARTY_CC_HDRS), $(MICROLITE_CC_SRCS))
THIRD_PARTY_CC_HDRS := $(patsubst  $(MAKEFILE_DIR)/downloads/%, third_party/%,  $(THIRD_PARTY_CC_HDRS))

#copy all third party headers that are mentioned in includes
INCLUDE_HDRS := $(wildcard $(addsuffix /*.h,$(filter $(MAKEFILE_DIR)/downloads/%, $(patsubst -I%,%,$(INCLUDES)))))
INCLUDE_HDRS += $(wildcard $(addsuffix /*.h,$(filter $(MAKEFILE_DIR)/downloads/%,  $(patsubst -isystem%,%,$(INCLUDES)))))
THIRD_PARTY_CC_HDRS += $(patsubst  $(MAKEFILE_DIR)/downloads/%, third_party/%,  $(INCLUDE_HDRS))


# Builds a standalone binary.
$(eval $(call microlite_test,hello_world,\
$(HELLO_WORLD_SRCS),$(HELLO_WORLD_HDRS)))
