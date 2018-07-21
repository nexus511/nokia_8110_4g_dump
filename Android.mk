LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_CFLAGS += \
	-pedantic

LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_MODULE := dump8110
LOCAL_MODULE_TAGS := optional
LOCAL_CPP_EXTENSION := .cpp
LOCAL_LDFLAGS := -static
LOCAL_STATIC_LIBRARIES += libcutils liblog libstdc++ libc
LOCAL_SRC_FILES := \
        src/main.cpp

include $(BUILD_EXECUTABLE)
