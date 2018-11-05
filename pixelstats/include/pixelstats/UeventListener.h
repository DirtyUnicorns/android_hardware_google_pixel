/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef HARDWARE_GOOGLE_PIXEL_PIXELSTATS_UEVENTLISTENER_H
#define HARDWARE_GOOGLE_PIXEL_PIXELSTATS_UEVENTLISTENER_H

#include <android-base/chrono_utils.h>
#include <hardware/google/pixelstats/1.0/IPixelStats.h>

namespace android {
namespace hardware {
namespace google {
namespace pixel {

/**
 * A class to listen for uevents and report reliability events to
 * the PixelStats HAL.
 * Runs in a background thread if created with ListenForeverInNewThread().
 * Alternatively, process one message at a time with ProcessUevent().
 */
class UeventListener {
  public:
    UeventListener(const std::string audio_uevent);

    bool ProcessUevent();  // Process a single Uevent.
    void ListenForever();  // Process Uevents forever
  private:
    void ReportUsbConnectorUevents(const char *power_supply_typec_mode);
    void ReportUsbAudioUevents(const char *driver, const char *product, const char *action);
    void ReportMicBroken(const char *devpath, const char *mic_break_status);

    const std::string kAudioUevent;

    int uevent_fd_;

    bool is_usb_attached_;                         // Tracks USB port connectivity state.
    android::base::Timer usb_connect_time_;        // Time of last USB port connection.
    android::base::Timer usb_audio_connect_time_;  // Time of last USB audio connection.
    char *attached_product_;  // PRODUCT= string of currently attached USB audio device.
};

}  // namespace pixel
}  // namespace google
}  // namespace hardware
}  // namespace android

#endif  // HARDWARE_GOOGLE_PIXEL_PIXELSTATS_UEVENTLISTENER_H
