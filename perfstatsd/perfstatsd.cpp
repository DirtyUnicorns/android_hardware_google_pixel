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

#define LOG_TAG "perfstatsd"

#include <perfstatsd.h>

using namespace android::pixel::perfstatsd;

perfstatsd_t::perfstatsd_t(void) {
    mRefreshPeriod = DEFAULT_DATA_COLLECT_PERIOD;

    std::unique_ptr<statstype> cpuUsage(new cpu_usage);
    cpuUsage->setBufferSize(CPU_USAGE_BUFFER_SIZE);
    mStats.emplace_back(std::move(cpuUsage));

    std::unique_ptr<statstype> ioUsage(new io_usage);
    ioUsage->setBufferSize(IO_USAGE_BUFFER_SIZE);
    mStats.emplace_back(std::move(ioUsage));
}

void perfstatsd_t::refresh(void) {
    for (auto const &stats : mStats) {
        stats->refresh();
    }
    return;
}

void perfstatsd_t::get_history(std::string *ret) {
    std::priority_queue<statsdata, std::vector<statsdata>, StatsdataCompare> mergedQueue;
    for (auto const &stats : mStats) {
        stats->dump(&mergedQueue);
    }

    while (!mergedQueue.empty()) {
        statsdata data = mergedQueue.top();
        auto raw_time = data.getTime();
        auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(raw_time);
        auto d = raw_time - seconds;
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(d);
        std::string content = data.getData();

        time_t t = std::chrono::system_clock::to_time_t(raw_time);
        char buff[20];
        strftime(buff, sizeof(buff), "%m-%d %H:%M:%S", localtime(&t));

        ret->append(std::string(buff) + ".");
        ret->append(std::to_string(milliseconds.count()) + "\n");
        ret->append(content + "\n");
        mergedQueue.pop();
    }

    if (ret->size() > 400_KiB)
        LOG_TO(SYSTEM, WARNING) << "Data might be too large. size: " << ret->size() << " bytes\n"
                                << *ret;
}

void perfstatsd_t::setOptions(const std::string &key, const std::string &value) {
    if (key == PERFSTATSD_PERIOD) {
        uint32_t val = 0;
        if (!base::ParseUint(value, &val) || val < 1) {
            LOG_TO(SYSTEM, ERROR) << "Invalid value. Minimum refresh period is 1 second";
        } else {
            mRefreshPeriod = val;
            LOG_TO(SYSTEM, INFO) << "set period to " << value << " seconds";
        }
        return;
    }

    for (auto const &stats : mStats) {
        stats->setOptions(std::forward<const std::string>(key),
                          std::forward<const std::string>(value));
    }
    return;
}
