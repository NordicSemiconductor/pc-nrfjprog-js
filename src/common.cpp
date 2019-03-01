/* Copyright (c) 2015 - 2017, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Use in source and binary forms, redistribution in binary form only, with
 * or without modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 2. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 3. This software, with or without modification, must only be used with a Nordic
 *    Semiconductor ASA integrated circuit.
 *
 * 4. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "common.h"

#include <chrono>

#include "DllCommonDefinitions.h"

const std::string getCurrentTimeInMilliseconds()
{
    auto current_time = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(current_time);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(current_time.time_since_epoch());

    auto ttm = gmtime(&time);

    char date_time_format[] = "%Y-%m-%dT%H:%M:%S";
    char time_str[20] = "";

    strftime(time_str, 20, date_time_format, ttm);

    std::string result(time_str);
    result.append(".");

    char millisecond_str[5];
    sprintf(millisecond_str, "%03d", static_cast<int>(ms.count() % 1000));
    result.append(millisecond_str);
    result.append("Z");

    return result;
}

uint16_t uint16_decode(const uint8_t *p_encoded_data)
{
        return ( (static_cast<uint16_t>(const_cast<uint8_t *>(p_encoded_data)[0])) |
                 (static_cast<uint16_t>(const_cast<uint8_t *>(p_encoded_data)[1]) << 8 ));
}

uint32_t uint32_decode(const uint8_t *p_encoded_data)
{
    return ((static_cast<uint32_t>(const_cast<uint8_t *>(p_encoded_data)[0]) << 0)  |
            (static_cast<uint32_t>(const_cast<uint8_t *>(p_encoded_data)[1]) << 8)  |
            (static_cast<uint32_t>(const_cast<uint8_t *>(p_encoded_data)[2]) << 16) |
            (static_cast<uint32_t>(const_cast<uint8_t *>(p_encoded_data)[3]) << 24));
}

uint16_t fromNameToValue(name_map_t names, const char *name)
{
    name_map_it_t it;
    uint16_t key = -1;

    for (it = names.begin(); it != names.end(); ++it)
    {
        if (strcmp(it->second, name) == 0)
        {
            key = it->first;
            break;
        }
    }

    return key;
}
