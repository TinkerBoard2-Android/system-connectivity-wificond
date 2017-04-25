/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef WIFICOND_OFFLOAD_SCAN_MANAGER_H_
#define WIFICOND_OFFLOAD_SCAN_MANAGER_H_

#include <android/hardware/wifi/offload/1.0/IOffload.h>
#include "wificond/scanning/offload/offload_callback.h"
#include "wificond/scanning/offload/offload_service_utils.h"
#include "wificond/scanning/offload/offload_callback_handlers.h"

#include <vector>

using ::android::hardware::hidl_vec;
using android::hardware::wifi::offload::V1_0::ScanResult;
using android::hardware::wifi::offload::V1_0::IOffload;
using android::hardware::wifi::offload::V1_0::OffloadStatus;

namespace com {
namespace android {
namespace server {
namespace wifi {
namespace wificond {

class NativeScanResult;

}  // namespace wificond
}  // namespace wifi
}  // namespace server
}  // namespace android
}  // namespace com

namespace android {
namespace wificond {

class OffloadScanManager;

typedef std::function<void(
    const std::vector<::com::android::server::wifi::wificond::NativeScanResult>
        scanResult)> OnNativeScanResultsReadyHandler;

// Provides callback interface implementation from Offload HAL
class OffloadCallbackHandlersImpl : public OffloadCallbackHandlers {
 public:
  OffloadCallbackHandlersImpl(OffloadScanManager* parent);
  ~OffloadCallbackHandlersImpl() override;

  void OnScanResultHandler(const std::vector<ScanResult> &scanResult) override;
  void OnErrorHandler(OffloadStatus status) override;

 private:
  OffloadScanManager* offload_scan_manager_;
};

// Provides methods to interact with Offload HAL
class OffloadScanManager {
 public:
  enum StatusCode {
      /* Corresponds to OffloadStatus::OFFLOAD_STATUS_OK */
      kNoError,
      /* Offload HAL service not avaialble */
      kNoService,
      /* Corresponds to OffloadStatus::OFFLOAD_STATUS_NO_CONNECTION */
      kNotConnected,
      /* Corresponds to OffloadStatus::OFFLOAD_STATUS_TIMEOUT */
      kTimeOut,
      /* Corresponds to OffloadStatus::OFFLOAD_STATUS_ERROR */
      kError
  };

  explicit OffloadScanManager(OffloadServiceUtils* utils,
      OnNativeScanResultsReadyHandler handler);
  virtual ~OffloadScanManager();
  /* Caller can use this method to obtain status of the Offload HAL service
   * before invoking methods to perform disconnected PNO scans */
  StatusCode getOffloadStatus() const;

 private:
  void ReportScanResults(const std::vector<ScanResult> scanResult);
  void ReportError(OffloadStatus status);

  android::sp<IOffload> wifi_offload_hal_;
  android::sp<OffloadCallback> wifi_offload_callback_;
  OnNativeScanResultsReadyHandler scan_result_handler_;
  StatusCode offload_status_;
  const std::unique_ptr<OffloadCallbackHandlersImpl> offload_callback_handlers_;

  friend class OffloadCallbackHandlersImpl;
};

}  // namespace wificond
}  // namespace android

#endif // WIFICOND_OFFLOAD_SCAN_MANAGER_H_