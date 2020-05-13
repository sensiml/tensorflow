TENSORFLOW_LIB_SRCS := tensorflow/lite/micro/examples/cwrapper/micro_api.cc \
					   tensorflow/lite/micro/examples/cwrapper/model.cc \
					   tensorflow/lite/micro/examples/cwrapper/main.cc


TENSORFLOW_LIB_HDRS := tensorflow/lite/micro/examples/cwrapper/micro_api.h \
					   tensorflow/lite/micro/examples/cwrapper/micro_api_benchmark.h \
					   tensorflow/lite/micro/examples/cwrapper/model.h \
					   tensorflow/lite/micro/examples/cwrapper/test_data.h

# Builds a standalone library.
$(eval $(call microlite_test,cwrapper,\
$(TENSORFLOW_LIB_SRCS),$(TENSORFLOW_LIB_HDRS)))
