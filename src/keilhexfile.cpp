/* Used to eliminate warnings from windows. Will make no harm in Linux. */
#define _CRT_SECURE_NO_WARNINGS

#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <cstring>

#include "keilhexfile.h"
#include "flashmemorysegment.h"

// This defines how much data to put on each line.
#define NOMINAL_FILE_LINE_LEN           (16UL)

#if (NOMINAL_FILE_LEN > 255UL)
#error "A KEIL hex file can not have more than 255 data bytes per line."
#endif

#define COLON_INDEX                     (0)
#define RECORD_LEN_INDEX                (1)
#define ADDR_INDEX                      (3)
#define RECORD_TYPE_INDEX               (7)
#define DATA_INDEX                      (9)

#define EOF_RECORD_DATA_LEN             (0)
#define EXT_SEG_ADDR_RECORD_DATA_LEN    (2)
#define EXT_LIN_ADDR_RECORD_DATA_LEN    (2)

#define UINT8_CHAR_LEN                  (2)
#define COLON_CHAR_LEN                  (1)
#define RECORD_LEN_CHAR_LEN             (UINT8_CHAR_LEN)
#define ADDR_CHAR_LEN                   (2 * UINT8_CHAR_LEN)
#define RECORD_TYPE_CHAR_LEN            (UINT8_CHAR_LEN)
#define CHECKSUM_CHAR_LEN               (UINT8_CHAR_LEN)
#define MIN_LINE_LEN                    (COLON_CHAR_LEN +\
                                            RECORD_LEN_CHAR_LEN +\
                                            ADDR_CHAR_LEN +\
                                            RECORD_TYPE_CHAR_LEN +\
                                            CHECKSUM_CHAR_LEN)

static const char FILE_LINE_PREFIX_FMT[]= ":%02X%04X%02X";
static const char FILE_BYTE_FMT[]       = "%02X";
static const char FILE_LINE_SEP[]       = "\r\n";

static const char HEX_CHAR_STR[]        = "0123456789abcdefABCDEF";
static const char WHITESPACE_STR[]      = " \t\r\n";

static uint8_t calc_checksum(uint8_t record_len,
                                uint16_t address,
                                KeilHexFile::RecordType type,
                                uint8_t * data);


static KeilHexFile::Status read_line(std::vector<FlashMemorySegment> &segments,
                                        std::string &line,
                                        uint32_t &current_address,
                                        bool &finished);


static KeilHexFile::Status parse_line(std::string &line,
                                        uint32_t &addr,
                                        KeilHexFile::RecordType &record_type,
                                        uint8_t *& buf,
                                        uint32_t &buf_len);


static bool is_valid_hex(std::string &line, size_t offset);


static void trim(std::string &line);


static KeilHexFile::Status write_line(FILE * out_file,
                                        uint8_t record_len,
                                        uint16_t address,
                                        KeilHexFile::RecordType type,
                                        uint8_t * data);


static bool verify_segment_addresses(std::vector<FlashMemorySegment> &segs);


KeilHexFile::KeilHexFile(void)
{
}


KeilHexFile::KeilHexFile(const KeilHexFile &rhs)
{
    segments = rhs.segments;
}


KeilHexFile& KeilHexFile::operator=(const KeilHexFile &rhs)
{
    segments = rhs.segments;

    return *this;
}


KeilHexFile::~KeilHexFile(void)
{
}


KeilHexFile::Status KeilHexFile::open(const char * path)
{
    uint32_t        current_addr        = 0;
    bool            finished            = false;
    bool            has_error           = false;
    Status          result              = INVALID_FILE_FORMAT;
    std::ifstream   in_file(path);

    if (!in_file.is_open())
    {
        return FILE_ACCESS_ERROR;
    }

    segments.clear();

    while (!finished && in_file.good())
    {
        std::string line;

        if (in_file.bad())
        {
            has_error = true;
            break;
        }

        std::getline(in_file, line);

        trim(line);

        if (0 == line.length())
        {
            continue;
        }

        if (':' != line[COLON_INDEX])
        {
            continue;
        }

        if (!is_valid_hex(line, COLON_CHAR_LEN))
        {
            has_error = true;
            break;
        }

        if (SUCCESS != (result = read_line(segments, line, current_addr, finished)))
        {
            has_error = true;
            break;
        }
    }

    in_file.close();

    if (has_error || !finished)
    {
        segments.clear();

        if (!finished)
        {
            return INVALID_FILE_FORMAT;
        }
    }
    else if (0 != segments.size())
    {
        std::sort(segments.begin(), segments.end());

        if (!verify_segment_addresses(segments))
        {
            return INVALID_FILE_FORMAT;
        }
    }

    return result;
}

KeilHexFile::Status KeilHexFile::open(std::string hex_file_contents)
{
    uint32_t        current_addr        = 0;
    bool            finished            = false;
    bool            has_error           = false;
    Status          result              = INVALID_FILE_FORMAT;

    std::istringstream in_stream(hex_file_contents);

    segments.clear();

    while (!finished && in_stream.good())
    {
        std::string line;

        if (in_stream.bad())
        {
            has_error = true;
            break;
        }

        std::getline(in_stream, line);

        trim(line);

        if (0 == line.length())
        {
            continue;
        }

        if (':' != line[COLON_INDEX])
        {
            continue;
        }

        if (!is_valid_hex(line, COLON_CHAR_LEN))
        {
            has_error = true;
            break;
        }

        if (SUCCESS != (result = read_line(segments, line, current_addr, finished)))
        {
            has_error = true;
            break;
        }
    }

    if (has_error || !finished)
    {
        segments.clear();

        if (!finished)
        {
            return INVALID_FILE_FORMAT;
        }
    }
    else if (0 != segments.size())
    {
        std::sort(segments.begin(), segments.end());

        if (!verify_segment_addresses(segments))
        {
            return INVALID_FILE_FORMAT;
        }
    }

    return result;
}


KeilHexFile::Status KeilHexFile::read(uint32_t address,
                                            uint8_t data[],
                                            uint32_t data_len,
                                            uint32_t &bytes_read) const
{
    std::vector<FlashMemorySegment>::const_iterator it;

    if (0 == data_len)
    {
        bytes_read = 0;
        return SUCCESS;
    }

    it = std::lower_bound(segments.begin(), segments.end(), address);

    if (segments.end() == it)
    {
        bytes_read = 0;
        return SUCCESS;
    }

    if (FlashMemorySegment::SUCCESS != it->read(address, data, data_len, bytes_read))
    {
        return MEMORY_SEGMENT_ERROR;
    }

    if (0 == bytes_read)
    {
        // The requested address was not valid.
        return MEMORY_SEGMENT_ERROR;
    }

    for (it++; ((data_len > bytes_read) && (segments.end() != it)); it++)
    {
        uint32_t next_address = (address + bytes_read);
        uint32_t next_bytes_read;

        if (it->get_address() == next_address)
        {
            if (FlashMemorySegment::SUCCESS != it->read(next_address,
                                                             &data[bytes_read],
                                                             (data_len - bytes_read),
                                                             next_bytes_read))
            {
                return MEMORY_SEGMENT_ERROR;
            }

            bytes_read += next_bytes_read;
        }
        else
        {
            // The next data read would not be contiguous.
            break;
        }
    }

    return SUCCESS;
}


KeilHexFile::Status KeilHexFile::nand_read(uint32_t address, uint8_t data[], uint32_t data_len) const
{
    uint32_t result_addr;
    uint32_t result_len;
    uint32_t end_addr = (address + data_len);

    memset(data, 0xFF, data_len);

    find(address, result_addr, result_len);

    while ((0 != result_len) && (result_addr < end_addr))
    {
        KeilHexFile::Status status;
        uint32_t bytes_read;

        result_len = (result_len <= (end_addr - result_addr)) ? (result_len) : (end_addr - result_addr);

        status = read(result_addr, &data[(result_addr - address)], result_len, bytes_read);

        if (KeilHexFile::SUCCESS != status)
        {
            return status;
        }

        find((result_addr + result_len), result_addr, result_len);
    }

    return KeilHexFile::SUCCESS;
}


KeilHexFile::Status KeilHexFile::nor_read(uint32_t address, uint8_t data[], uint32_t data_len) const
{
    uint32_t result_addr;
    uint32_t result_len;
    uint32_t end_addr = (address + data_len);

    memset(data, 0x00, data_len);

    find(address, result_addr, result_len);

    while ((0 != result_len) && (result_addr < end_addr))
    {
        KeilHexFile::Status status;
        uint32_t bytes_read;

        result_len = (result_len <= (end_addr - result_addr)) ? (result_len) : (end_addr - result_addr);

        status = read(result_addr, &data[(result_addr - address)], result_len, bytes_read);

        if (KeilHexFile::SUCCESS != status)
        {
            return status;
        }

        find((result_addr + result_len), result_addr, result_len);
    }

    return KeilHexFile::SUCCESS;
}


KeilHexFile::Status KeilHexFile::write(uint32_t address, const uint8_t data[], uint32_t data_len)
{
    std::vector<FlashMemorySegment>::iterator   it;
    uint32_t                                    overlap_len = 0;

    if (0 == data_len)
    {
        return SUCCESS;
    }

    it = std::lower_bound(segments.begin(), segments.end(), address);

    if (segments.end() == it)
    {
        // Check to see if the data would be accepted by the last element in the vector.
        if ((segments.size() == 0) || !segments.back().will_accept_address(address))
        {
            try
            {
                segments.push_back(FlashMemorySegment(address));
            }
            catch (const std::exception)
            {
                return KeilHexFile::OUT_OF_MEMORY;
            }
        }

        if (FlashMemorySegment::OUT_OF_MEMORY == segments.back().write(address, data, data_len))
        {
            return OUT_OF_MEMORY;
        }

        return SUCCESS;
    }

    if (it->will_accept_address(address))
    {
        // Extend this memory segment.
        if (segments.end() != (it + 1))
        {
            uint32_t end_address = ((address - it->get_address()) + data_len);

            if (end_address > (it + 1)->get_address())
            {
                overlap_len = (end_address - (it + 1)->get_address());
            }
        }
    }
    else
    {
        // Insert a new seg before this iterator.
        if ((address + data_len) > it->get_address())
        {
            overlap_len = ((address + data_len) - it->get_address());
        }

        it = segments.insert(it, FlashMemorySegment(address));
    }

    if (overlap_len)
    {
        data_len -= overlap_len;
    }

    if (FlashMemorySegment::OUT_OF_MEMORY == it->write(address, data, data_len))
    {
        return OUT_OF_MEMORY;
    }

    if (overlap_len)
    {
        return write((address + data_len), (data + data_len), overlap_len);
    }

    return SUCCESS;
}


void KeilHexFile::remove(uint32_t address, uint32_t len)
{
    std::vector<FlashMemorySegment>::iterator it;

    if (0 == len)
    {
        return;
    }

    it = std::lower_bound(segments.begin(), segments.end(), address);

    if ((segments.end() != it) && !it->can_remove(address, len))
    {
        // The lower_bound function will select the segment before
        // the proper segment if the two segments are immediately
        // adjacent.
        if ((it->get_address() + it->get_length()) == address)
        {
            it++;
        }
    }

    for (; ((0 < len) && (segments.end() != it));)
    {
        uint32_t count = 0;

        if (!it->can_remove(address, len))
        {
            if (it->get_address() > address)
            {
                return;
            }
            else
            {
                // Need to split the segment into two pieces.
                uint32_t first_addr     = it->get_address();
                uint32_t second_addr    = (address + len);

                it = segments.insert(it, FlashMemorySegment(second_addr));
                it = segments.insert(it, FlashMemorySegment(first_addr));

                // Now the iterator points to first_addr.
                (it + 2)->split((address - first_addr), (second_addr - first_addr), *it, *(it + 1));

                segments.erase(it + 2);

                return;
            }
        }

        if (address < it->get_address())
        {
            count += (it->get_address() - address);
        }

        count += it->remove(address, len);

        if (0 == it->get_length())
        {
            // The iterator will be set to the index of the next item in the vector.
            // It will then be incremented AGAIN at the end of the loop. Decrement
            // it now to prevent it from skipping past the end of the vector.
            it = segments.erase(it);
        }
        else
        {
            it++;
        }

        address += count;
        len -= count;
    }

    return;
}


uint32_t KeilHexFile::get_num_segments(void) const
{
    return segments.size();
}


KeilHexFile::Status KeilHexFile::get_segment_address(uint32_t segment_index, uint32_t &address) const
{
    if (segment_index >= segments.size())
    {
        return MEMORY_SEGMENT_ERROR;
    }

    address = segments[segment_index].get_address();

    return SUCCESS;
}


KeilHexFile::Status KeilHexFile::get_segment_length(uint32_t segment_index, uint32_t &len) const
{
    if (segment_index >= segments.size())
    {
        return MEMORY_SEGMENT_ERROR;
    }

    len = segments[segment_index].get_length();

    return SUCCESS;
}


KeilHexFile::Status KeilHexFile::read_segment(uint32_t segment_index,
                                                    uint8_t data[],
                                                    uint32_t data_len,
                                                    uint32_t &bytes_read,
                                                    uint32_t offset) const
{
    uint32_t seg_addr;
    uint32_t seg_len;

    if (segment_index >= segments.size())
    {
        return MEMORY_SEGMENT_ERROR;
    }

    seg_addr = segments[segment_index].get_address();
    seg_len  = segments[segment_index].get_length();

    if (offset > seg_len)
    {
        return MEMORY_SEGMENT_ERROR;
    }

    if (0 == data_len)
    {
        bytes_read = 0;
        return SUCCESS;
    }

    data_len = (data_len < (seg_len - offset)) ? (data_len) : (seg_len - offset);

    seg_addr += offset;

    if (FlashMemorySegment::SUCCESS != segments[segment_index].read(seg_addr, data, data_len, bytes_read))
    {
        return MEMORY_SEGMENT_ERROR;
    }

    return SUCCESS;
}


void KeilHexFile::find(uint32_t address_offset,
                                            uint32_t &result_address,
                                            uint32_t &bytes_available) const
{
    std::vector<FlashMemorySegment>::const_iterator it;

    it = std::lower_bound(segments.begin(), segments.end(), address_offset);

    if (segments.end() == it)
    {
        bytes_available = 0;
        return;
    }

    result_address = (it->get_address() > address_offset) ? (it->get_address()) : (address_offset);

    bytes_available = (it->get_length() - (result_address - it->get_address()));
}


void KeilHexFile::find_contiguous(uint32_t address_offset,
                                            uint32_t &result_address,
                                            uint32_t &bytes_available) const
{
    uint32_t result_len = 0;
    uint32_t len;
    uint32_t addr;

    find(address_offset, result_address, len);

    if (0 == len)
    {
        bytes_available = 0;
        return;
    }
    
    do
    {
        result_len += len;
        find((result_address + result_len), addr, len);
    }
    while ((0 != len) && (addr == (result_address + result_len)));
    
    bytes_available = result_len;
}


void KeilHexFile::find_contiguous_max(uint32_t address_offset,
                                            uint32_t bytes_max,
                                            uint32_t &result_address,
                                            uint32_t &bytes_available) const
{
    uint32_t result_len = 0;
    uint32_t len;
    uint32_t addr;

    find(address_offset, result_address, len);

    if (0 == len)
    {
        bytes_available = 0;
        return;
    }
    
    do
    {
        result_len += len;
        find((result_address + result_len), addr, len);
    }
    while ((0 != len) && (addr == (result_address + result_len)));
    
    bytes_available = result_len;

    if (bytes_available > bytes_max){
        bytes_available = bytes_max;
    }
}


KeilHexFile::Status KeilHexFile::has_data(uint32_t start_addr, uint32_t end_addr, bool &result) const
{
    uint32_t result_addr;
    uint32_t result_len;

    find(start_addr, result_addr, result_len);

    while ((0 != result_len) && (result_addr < end_addr))
    {
        KeilHexFile::Status status;
        uint8_t buf;
        uint32_t bytes_read;

        result_len = (result_len <= (end_addr - result_addr)) ? (result_len) : (end_addr - result_addr);

        for (uint32_t i = 0; i < result_len; i++)
        {
            status = read((result_addr + i), &buf, sizeof(buf), bytes_read);

            if (KeilHexFile::SUCCESS != status)
            {
                return status;
            }

            if (buf != 0xFF)
            {
                result = true;
                return KeilHexFile::SUCCESS;
            }
        }

        find((result_addr + result_len), result_addr, result_len);
    }

    result = false;
    return KeilHexFile::SUCCESS;
}


KeilHexFile::Status KeilHexFile::save(const char * path) const
{
    FILE *      out_file;
    uint16_t    temp_addr;
    uint32_t    result_len;
    uint8_t     buf[NOMINAL_FILE_LINE_LEN];
    uint32_t    high_address = 0;
    uint32_t    result_addr = 0;

    out_file = fopen(path, "wb");
    if (out_file == NULL)
    {
        return FILE_ERROR;
    }

    // Find the first address so the initial EXT_LINEAR_ADDR_RECORD can be written.
    find(result_addr, result_addr, result_len);

    if (0 != result_len)
    {
        high_address = (result_addr & 0xFFFF0000);

        // Addresses are written in big-endian form. This is normally taken care of
        // automatically when coverting a multi-byte hex number to ASCII (i.e. %04X).
        // However, in this case the address is part of the data field of the record.
        // Therefore, it is necessary to reverse the order of the bytes before printing them.
        temp_addr = (result_addr >> 24);
        temp_addr |= ((result_addr >> 8) & 0xFF00);

        if (SUCCESS != write_line(out_file, sizeof(temp_addr), 0x0000, EXT_LINEAR_ADDR_RECORD, (uint8_t*) &temp_addr))
        {
            fclose(out_file);
            return FILE_ERROR;
        }
    }

    while (0 != result_len)
    {
        read(result_addr, buf, NOMINAL_FILE_LINE_LEN, result_len);

        if (0 == result_len)
        {
            // Find next available address.
            find(result_addr, result_addr, result_len);

            continue;
        }

        if ((result_addr & 0xFFFF0000) < ((result_addr + result_len - 1) & 0xFFFF0000))
        {
            // This line must be split in two with an EXT_LINEAR_ADDR_RECORD between the two parts.
            result_len = (0x10000 - (result_addr & 0xFFFF));
        }

        if ((result_addr & 0xFFFF0000) != high_address)
        {
            high_address = (result_addr & 0xFFFF0000);

            temp_addr = (result_addr >> 24);
            temp_addr |= ((result_addr >> 8) & 0xFF00);

            if (SUCCESS != write_line(out_file, sizeof(temp_addr), 0x0000, EXT_LINEAR_ADDR_RECORD, (uint8_t*) &temp_addr))
            {
                fclose(out_file);
                return FILE_ERROR;
            }
        }

        temp_addr = (uint16_t) result_addr;
        if (SUCCESS != write_line(out_file, (uint8_t) result_len, temp_addr, DATA_RECORD, buf))
        {
            fclose(out_file);
            return FILE_ERROR;
        }

        result_addr += result_len;
    }

    if (SUCCESS != write_line(out_file, 0, 0x0000, EOF_RECORD, NULL))
    {
        fclose(out_file);
        return FILE_ERROR;
    }

    fclose(out_file);

    return SUCCESS;
}


KeilHexFile::Status KeilHexFile::merge(const KeilHexFile &rhs)
{
    uint32_t old_len = segments.size();

    if (this == &rhs)
    {
        return INVALID_PARAM_ERROR;
    }

    if (0 == rhs.segments.size())
    {
        return SUCCESS;
    }

    try
    {
        segments.resize(old_len + rhs.segments.size());
    }
    catch (const std::exception)
    {
        return OUT_OF_MEMORY;
    }

    std::copy(rhs.segments.begin(), rhs.segments.end(), (segments.begin() + old_len));

    std::sort(segments.begin(), segments.end());

    if (!verify_segment_addresses(segments))
    {
        return MEMORY_SEGMENT_ERROR;
    }

    return SUCCESS;
}


KeilHexFile::Status KeilHexFile::merge(const KeilHexFile &lhs, const KeilHexFile &rhs, KeilHexFile &merged)
{
    if ((&lhs == &merged) || (&rhs == &merged))
    {
        return INVALID_PARAM_ERROR;
    }

    try
    {
        merged.segments.resize(lhs.segments.size() + rhs.segments.size());
    }
    catch (const std::exception)
    {
        return OUT_OF_MEMORY;
    }

    std::copy(lhs.segments.begin(), lhs.segments.end(), merged.segments.begin());
    std::copy(rhs.segments.begin(), rhs.segments.end(), (merged.segments.begin() + lhs.segments.size()));

    std::sort(merged.segments.begin(), merged.segments.end());

    if (!verify_segment_addresses(merged.segments))
    {
        return MEMORY_SEGMENT_ERROR;
    }

    return SUCCESS;
}


static KeilHexFile::Status read_line(std::vector<FlashMemorySegment> &segments,
                                                std::string &line,
                                                uint32_t &current_address,
                                                bool &finished)
{
    KeilHexFile::Status     result;
    uint32_t                addr;
    KeilHexFile::RecordType type;
    uint8_t *               buf = NULL;
    uint32_t                buf_len;

    result = parse_line(line, addr, type, buf, buf_len);

    if (KeilHexFile::SUCCESS != result)
    {
        return result;
    }

    switch (type)
    {
        case KeilHexFile::DATA_RECORD:
            {
                uint32_t working_addr = (addr + current_address);

                try
                {
                    segments.push_back(FlashMemorySegment(working_addr));
                }
                catch (const std::exception)
                {
                    return KeilHexFile::OUT_OF_MEMORY;
                }

                if (FlashMemorySegment::OUT_OF_MEMORY == segments.back().write(working_addr,
                                                                                    buf,
                                                                                    buf_len))
                {
                    result = KeilHexFile::OUT_OF_MEMORY;
                }
            }
            break;
        case KeilHexFile::EOF_RECORD:
            finished = true;
            break;
        case KeilHexFile::EXT_SEG_ADDR_RECORD:
            current_address = buf[0];
            current_address <<= 8;
            current_address |= buf[1];
            current_address <<= 4;
            break;
        case KeilHexFile::EXT_LINEAR_ADDR_RECORD:
            current_address = buf[0];
            current_address <<= 8;
            current_address |= buf[1];
            current_address <<= 16;
            break;
        default:
            // Ignored record type.
            break;
    }

    if (buf_len > 0)
    {
        delete[] buf;
    }

    return result;
}


static KeilHexFile::Status parse_line(std::string &line,
                                                uint32_t &addr,
                                                KeilHexFile::RecordType &record_type,
                                                uint8_t *& buf,
                                                uint32_t &buf_len)
{
    const char *    c_line = line.c_str();
    uint32_t        expected_len;
    uint32_t        checksum;
    uint32_t        temp;

    c_line += COLON_CHAR_LEN;

    if (1 != sscanf(c_line, "%2x", &buf_len))
    {
        buf_len = 0;
        buf = NULL;
        return KeilHexFile::INVALID_FILE_FORMAT;
    }

    expected_len = (MIN_LINE_LEN + (UINT8_CHAR_LEN * buf_len));

    if (line.length() != expected_len)
    {
        buf_len = 0;
        buf = NULL;
        return KeilHexFile::INVALID_FILE_FORMAT;
    }

    c_line += RECORD_LEN_CHAR_LEN;

    if (2 != sscanf(c_line, "%4x%2x", &addr, &temp))
    {
        buf_len = 0;
        buf = NULL;
        return KeilHexFile::INVALID_FILE_FORMAT;
    }

    if (temp > KeilHexFile::START_LINEAR_ADDR_RECORD)
    {
        buf_len = 0;
        buf = NULL;
        return KeilHexFile::INVALID_FILE_FORMAT;
    }

    record_type = (KeilHexFile::RecordType) temp;

    switch (record_type)
    {
        case KeilHexFile::EOF_RECORD:
            if (EOF_RECORD_DATA_LEN != buf_len)
            {
                buf_len = 0;
                buf = NULL;
                return KeilHexFile::INVALID_FILE_FORMAT;
            }
            break;
        case KeilHexFile::EXT_SEG_ADDR_RECORD:
            if (EXT_SEG_ADDR_RECORD_DATA_LEN != buf_len)
            {
                buf_len = 0;
                buf = NULL;
                return KeilHexFile::INVALID_FILE_FORMAT;
            }
            break;
        case KeilHexFile::EXT_LINEAR_ADDR_RECORD:
            if (EXT_LIN_ADDR_RECORD_DATA_LEN != buf_len)
            {
                buf_len = 0;
                buf = NULL;
                return KeilHexFile::INVALID_FILE_FORMAT;
            }
            break;
        default:
            break;
    }

    buf = new uint8_t[buf_len];

    if (NULL == buf)
    {
        buf_len = 0;
        return KeilHexFile::OUT_OF_MEMORY;
    }

    c_line += (ADDR_CHAR_LEN + RECORD_LEN_CHAR_LEN);

    for (uint32_t i = 0; i < buf_len; i++)
    {
        if (1 != sscanf(c_line, "%2x", &temp))
        {
            buf_len = 0;
            delete[] buf;
            buf = NULL;
            return KeilHexFile::INVALID_FILE_FORMAT;
        }

        *(buf + i) = (uint8_t) temp;

        c_line += UINT8_CHAR_LEN;
    }

    if (1 != sscanf(c_line, "%2x", &checksum))
    {
        buf_len = 0;
        delete[] buf;
        buf = NULL;
        return KeilHexFile::INVALID_FILE_FORMAT;
    }

    temp = calc_checksum((uint8_t) buf_len, (uint16_t) addr, record_type, buf);

    if (checksum != temp)
    {
        buf_len = 0;
        delete[] buf;
        buf = NULL;
        return KeilHexFile::INVALID_FILE_FORMAT;
    }

    return KeilHexFile::SUCCESS;
}


static bool is_valid_hex(std::string &line, size_t offset)
{
    size_t pos = line.find_first_not_of(HEX_CHAR_STR, offset);

    return (std::string::npos == pos);
}


static void trim(std::string &line)
{
    size_t start;
    size_t end;

    start = line.find_first_not_of(WHITESPACE_STR);
    if (std::string::npos == start)
    {
        line.clear();
        return;
    }

    end = line.find_last_not_of(WHITESPACE_STR);

    if ((0 != start) || (line.length() != end))
    {
        line = line.substr(start, (end - start + 1));
    }
}


static uint8_t calc_checksum(uint8_t record_len, uint16_t address, KeilHexFile::RecordType type, uint8_t * data)
{
    uint8_t checksum = record_len;

    checksum += ((address >> 8) + (address & 0xFF));
    checksum += ((uint8_t) type);

    for (uint32_t i = 0; i < record_len; i++)
    {
        checksum += *(data + i);
    }

    return (1 + (~checksum));
}


static KeilHexFile::Status write_line(FILE * out_file,
                                        uint8_t record_len,
                                        uint16_t address,
                                        KeilHexFile::RecordType type,
                                        uint8_t * data)
{
    uint8_t checksum = calc_checksum(record_len, address, type, data);

    if (fprintf(out_file,
                            FILE_LINE_PREFIX_FMT,
                            record_len,
                            address,
                            type) < 0)
    {
        return KeilHexFile::FILE_ERROR;
    }

    for (int i = 0; i < record_len; i++)
    {
        if (ferror(out_file) || (fprintf(out_file, FILE_BYTE_FMT, data[i]) < 0))
        {
            return KeilHexFile::FILE_ERROR;
        }
    }

    if (ferror(out_file) || (fprintf(out_file, FILE_BYTE_FMT, checksum) < 0))
    {
        return KeilHexFile::FILE_ERROR;
    }

    if (ferror(out_file) || (fprintf(out_file, FILE_LINE_SEP) < 0))
    {
        return KeilHexFile::FILE_ERROR;
    }

    return KeilHexFile::SUCCESS;
}


static bool verify_segment_addresses(std::vector<FlashMemorySegment> &segs)
{
    std::vector<FlashMemorySegment>::iterator   it;
    uint32_t                                    current_max_addr;

    it = segs.begin();

    if (segs.end() == it)
    {
        return true;
    }

    current_max_addr = (it->get_address() + it->get_length());

    for (it++; it != segs.end(); it++)
    {
        uint32_t next_start_addr = it->get_address();

        if (current_max_addr <= next_start_addr)
        {
            current_max_addr = (next_start_addr + it->get_length());
        }
        else
        {
            return false;
        }
    }

    return true;
}
