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
 
#pragma once

#include <stdint.h>
#include <vector>


class FlashMemorySegment
{
public:
    static const uint8_t FLASH_ERASE_VALUE = 0xFF;

    enum Status
    {
        SUCCESS             = 0,
        DISCONTIGUOUS_ERROR = 1,
        OUT_OF_MEMORY       = 2
    };

    FlashMemorySegment(void);
    FlashMemorySegment(const FlashMemorySegment &rhs);
    FlashMemorySegment(uint32_t address);
    ~FlashMemorySegment(void);

    /**
     * Reads or writes from the MemorySegment. Will grow the segment if necessary.
     */
    Status write(uint32_t address, const uint8_t buf[], uint32_t buf_len);
    Status read(uint32_t address, uint8_t buf[], uint32_t buf_len, uint32_t &bytes_written) const;

    /**
     * Returns this MemorySegment's starting address.
     */
    uint32_t get_address(void) const;

    /**
     * Returns the length of this segment's data.
     */
    uint32_t get_length(void) const;

    /**
     * Returns true if the given address is part of this segment.
     */
    bool has_address(uint32_t address) const;

    /**
     * Returns true if adding data at the given address will not cause this MemorySegment
     * to become discontiguous.
     */
    bool will_accept_address(uint32_t address) const;

    /**
     * Returns true if the given data can be removed without causing this segment to
     * become discontiguous (i.e. data is located at beginning or end of segment).
     */
    bool can_remove(uint32_t address, uint32_t len) const;

    /**
     * Returns number of bytes removed (can be zero).
     */
    uint32_t remove(uint32_t address, uint32_t len);

    /**
     * Copies the contents of this objets data into two the data of two other
     * objects. Data is copied in the ranges [0, end_of_first_region) and
     * [start_of_second_region, get_length()).
     */
    Status split(uint32_t end_of_first_region,
                            uint32_t start_of_second_region,
                            FlashMemorySegment &first,
                            FlashMemorySegment &second);

    FlashMemorySegment& operator=(const FlashMemorySegment &rhs);

    /**
     * FlashMemorySegments are compared by address.
     */
    bool operator< (const FlashMemorySegment& rhs) const;
    bool operator< (uint32_t address) const;

private:
    uint32_t addr;
    std::vector<uint8_t> data;
};
