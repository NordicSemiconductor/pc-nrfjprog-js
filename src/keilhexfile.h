#pragma once

#include <stdint.h>
#include <vector>
#include <string>


class FlashMemorySegment;


class KeilHexFile
{
public:

    enum Status
    {
        SUCCESS                 = 0,
        FILE_ACCESS_ERROR       = 1,
        INVALID_FILE_FORMAT     = 2,
        OUT_OF_MEMORY           = 3,
        MEMORY_SEGMENT_ERROR    = 4,
        FILE_ERROR              = 5,
        INVALID_PARAM_ERROR     = 6
    };

    enum RecordType
    {
        DATA_RECORD                 = 0,
        EOF_RECORD                  = 1,
        EXT_SEG_ADDR_RECORD         = 2,
        UNSUPPORTED_RECORD          = 3,
        EXT_LINEAR_ADDR_RECORD      = 4,
        START_LINEAR_ADDR_RECORD    = 5
    };

    KeilHexFile(void);
    KeilHexFile(const KeilHexFile &rhs);
    ~KeilHexFile(void);

    /**
     * Opens and parses the given file.
     */
    virtual Status open(const char * path);

    /**
     * Creates a new object from the hex file contained in the given string.
     */
    virtual Status open(std::string hex_file_contents);

    /**
     * Reads up to data_len bytes from the segment that contains the given address. If the
     * segment contains fewer than data_len bytes then the read will continue to the next
     * contiguous segment if it exists. The bytes_read parameter will be updated to show
     * the number of bytes that were actually read.
     */
    Status read(uint32_t address, uint8_t data[], uint32_t data_len, uint32_t &bytes_read) const;

    /**
     * Similar to the read function except if data for a given byte does not
     * exist then the byte will be treated as 0xFF. For example, reading a
     * uint32_t starting at address 0 will return 0xFFABFFFF if the only data
     * found was 0xAB at address 2.
     */
    Status nand_read(uint32_t address, uint8_t data[], uint32_t data_len) const;

    /**
     * Similar to the read function except if data for a given byte does not
     * exist then the byte will be treated as 0x00. For example, reading a
     * uint32_t starting at address 0 will return 0x00AB0000 if the only data
     * found was 0xAB at address 2.
     */
    Status nor_read(uint32_t address, uint8_t data[], uint32_t data_len) const;

    /**
     * Writes the specified bytes. Will overwrite any existing data and extend
     * any data segments as necessary.
     */
    Status write(uint32_t address, const uint8_t data[], uint32_t data_len);

    /**
     * Removes len bytes starting at addr.
     */
    void remove(uint32_t address, uint32_t len);

    /**
     * Returns the number of FlashMemorySegments that this class contains.
     */
    uint32_t get_num_segments(void) const;

    /**
     * Returns the flash address of the given segment.
     */
    Status get_segment_address(uint32_t segment_index, uint32_t &address) const;

    /**
     * Returns the length of the given segment. Returns MEMORY_SEGMENT_ERROR if the
     * segment_index is >= the number of segments.
     */
    Status get_segment_length(uint32_t segment_index, uint32_t &len) const;

    /**
     * Reads the starting address and length of the segment at the given index.
     * The offset parameter is added to the starting address of the segment
     * and then the read method is called. Will not read past the end of the given segment.
     * Returns MEMORY_SEGMENT_ERROR if the segment_index is >= the number of segments or
     * if the offset is greater than the length of the subset.
     */
    Status read_segment(uint32_t segment_index,
                            uint8_t data[],
                            uint32_t data_len,
                            uint32_t &bytes_read,
                            uint32_t offset=0) const;

    /**
     * Finds the first addess that is >= the address_offset. The bytes_available parameter
     * indicates the number of contiguous bytes that can be read starting from the
     * result_address in the same segment. If no address is found then bytes_available will be set to zero.
     */
    void find(uint32_t address_offset, uint32_t &result_address, uint32_t &bytes_available) const;

    /**
     * Finds the first addess that is >= the address_offset. The bytes_available parameter
     * indicates the number of contiguous bytes that can be read starting from the
     * result_address in any segment. If no address is found then bytes_available will be set to zero.
     */
    void find_contiguous(uint32_t address_offset, uint32_t &result_address, uint32_t &bytes_available) const;

    /**
     * Finds the first addess that is >= the address_offset. The bytes_available parameter
     * indicates the number of contiguous bytes that can be read starting from the
     * result_address in any segment, to a maximum of max_bytes. If no address is found then 
     * bytes_available will be set to zero.
     */
    void find_contiguous_max(uint32_t address_offset, uint32_t max_bytes, uint32_t &result_address, uint32_t &bytes_available) const;

    /**
     * Sets the result parameter to true if at least one byte in the given range
     * has a value that does not equal the erase value (0xFF). Range is [start_addr, end_addr).
     */
    Status has_data(uint32_t start_addr, uint32_t end_addr, bool &result) const;

    /**
     * Saves the file to given path.
     */
    Status save(const char * path) const;

    /**
     * Attempts to merge another hex file into this one. If the merge fails then
     * MEMORY_SEGMENT_ERROR will be returned and this object's data will be left in
     * an invalid state.
     */
    Status merge(const KeilHexFile &rhs);

    /**
     * Merges the two hex file objects and stores the merged data into the merged parameter.
     * If the merged parameter has any data then that data will be deleted before the
     * merged data is written.
     */
    static Status merge(const KeilHexFile &lhs, const KeilHexFile &rhs, KeilHexFile &merged);

    KeilHexFile& operator=(const KeilHexFile &rhs);

protected:
    std::vector<FlashMemorySegment> segments;
};
