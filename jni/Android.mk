#APP_STL:= stlport_static
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE    := smanative
LOCAL_C_INCLUDES := ./include
LOCAL_CFLAGS := -std=gnu++11
LOCAL_LDLIBS += -latomic

FILE_LIST := $(wildcard $(LOCAL_PATH)/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/**/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/**/**/*.cpp)
LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)

include $(BUILD_STATIC_LIBRARY)
