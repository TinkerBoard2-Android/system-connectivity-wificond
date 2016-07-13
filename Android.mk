# Copyright (C) 2016 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := $(call my-dir)

wificond_cpp_flags := -std=c++11 -Wall -Werror -Wno-unused-parameter

###
### wificond daemon.
###
include $(CLEAR_VARS)
LOCAL_MODULE := wificond
LOCAL_CPPFLAGS := $(wificond_cpp_flags)
LOCAL_INIT_RC := wificond.rc
LOCAL_SRC_FILES := \
    main.cpp
LOCAL_SHARED_LIBRARIES := \
    libbinder \
    libbase \
    libcutils \
    libutils \
    libwifi-hal \
    libwifi-system
LOCAL_STATIC_LIBRARIES := \
    libwificond
include $(BUILD_EXECUTABLE)

###
### wificond static library
###
include $(CLEAR_VARS)
LOCAL_MODULE := libwificond
LOCAL_CPPFLAGS := $(wificond_cpp_flags)
LOCAL_SRC_FILES := \
    ap_interface_binder.cpp \
    ap_interface_impl.cpp \
    client_interface.cpp \
    looper_backed_event_loop.cpp \
    server.cpp
LOCAL_SHARED_LIBRARIES := \
    libbase \
    libutils \
    libwifi-hal \
    libwifi-system
LOCAL_STATIC_LIBRARIES := \
    libwificond_nl
LOCAL_WHOLE_STATIC_LIBRARIES := \
    libwificond_ipc
include $(BUILD_STATIC_LIBRARY)

###
### wificond netlink library
###
include $(CLEAR_VARS)
LOCAL_MODULE := libwificond_nl
LOCAL_CPPFLAGS := $(wificond_cpp_flags)
LOCAL_SRC_FILES := \
    net/nl80211_attribute.cpp
LOCAL_SHARED_LIBRARIES := \
    libbase
include $(BUILD_STATIC_LIBRARY)

###
### wificond IPC interface library
###
include $(CLEAR_VARS)
LOCAL_MODULE := libwificond_ipc
LOCAL_AIDL_INCLUDES += $(LOCAL_PATH)/aidl
LOCAL_CPPFLAGS := $(wificond_cpp_flags)
LOCAL_SRC_FILES := \
    ipc_constants.cpp \
    aidl/android/net/wifi/IApInterface.aidl \
    aidl/android/net/wifi/IClientInterface.aidl \
    aidl/android/net/wifi/IWificond.aidl
LOCAL_SHARED_LIBRARIES := \
    libbinder
include $(BUILD_STATIC_LIBRARY)

###
### test util library
###
include $(CLEAR_VARS)
LOCAL_MODULE := libwificond_test_utils
LOCAL_CPPFLAGS := $(wificond_cpp_flags)
LOCAL_SRC_FILES := \
    tests/shell_utils.cpp
LOCAL_SHARED_LIBRARIES := \
    libbase
include $(BUILD_STATIC_LIBRARY)

###
### wificond unit tests.
###
include $(CLEAR_VARS)
LOCAL_MODULE := wificond_unit_test
LOCAL_CPPFLAGS := $(wificond_cpp_flags)
LOCAL_SRC_FILES := \
    tests/main.cpp \
    tests/looper_backed_event_loop_unittest.cpp \
    tests/nl80211_attribute_unittest.cpp \
    tests/server_unittest.cpp \
    tests/shell_unittest.cpp
LOCAL_STATIC_LIBRARIES := \
    libgmock \
    libgtest \
    libwifi-hal-test \
    libwifi-system-test \
    libwificond \
    libwificond_nl \
    libwificond_test_utils
LOCAL_SHARED_LIBRARIES := \
    libbase \
    libbinder \
    liblog \
    libutils \
    libwifi-hal \
    libwifi-system
include $(BUILD_NATIVE_TEST)

###
### wificond device integration tests.
###
include $(CLEAR_VARS)
LOCAL_MODULE := wificond_integration_test
LOCAL_CPPFLAGS := $(wificond_cpp_flags)
LOCAL_SRC_FILES := \
    tests/main.cpp \
    tests/integration/life_cycle_test.cpp
LOCAL_SHARED_LIBRARIES := \
    libbase \
    libbinder \
    libcutils \
    libutils
LOCAL_STATIC_LIBRARIES := \
    libwificond_ipc \
    libwificond_test_utils
include $(BUILD_NATIVE_TEST)