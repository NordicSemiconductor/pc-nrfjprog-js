#include "flashmemorysegment.h"
#include <iterator>

#ifdef __linux__
    namespace stdext { template<typename T> inline T checked_array_iterator(T arr, size_t numVals) { return arr; } }
#elif __APPLE__
    namespace stdext { template<typename T> inline T checked_array_iterator(T arr, size_t numVals) { return arr; } }
#endif

FlashMemorySegment::FlashMemorySegment(void)
{
    addr = 0;
}


FlashMemorySegment::FlashMemorySegment(uint32_t address)
{
    addr = address;
}


FlashMemorySegment::FlashMemorySegment(const FlashMemorySegment &rhs)
{
    addr = rhs.addr;
    data = rhs.data;
}


FlashMemorySegment& FlashMemorySegment::operator=(const FlashMemorySegment &rhs)
{
    addr = rhs.addr;
    data = rhs.data;

    return *this;
}


FlashMemorySegment::~FlashMemorySegment(void)
{
}


FlashMemorySegment::Status FlashMemorySegment::write(uint32_t address,
                                                        const uint8_t buf[],
                                                        uint32_t buf_len)
{
    if (0 == buf_len)
    {
        return SUCCESS;
    }

    if (!will_accept_address(address))
    {
        return DISCONTIGUOUS_ERROR;
    }

    uint32_t offset         = (address - addr);
    uint32_t required_len   = (offset + buf_len);

    if (data.size() < required_len)
    {
        try
        {
            data.resize(required_len);
        }
        catch (const std::exception)
        {
            return OUT_OF_MEMORY;
        }
    }

    std::copy(buf, (buf + buf_len), (data.begin() + offset));

    return SUCCESS;
}


FlashMemorySegment::Status FlashMemorySegment::read(uint32_t address,
                                                        uint8_t buf[],
                                                        uint32_t buf_len,
                                                        uint32_t &bytes_written) const
{
    if (0 == buf_len)
    {
        bytes_written = 0;
        return SUCCESS;
    }

    if (!has_address(address))
    {
        bytes_written = 0;
        return SUCCESS;
    }

    uint32_t offset = (address - addr);

    if ((offset + buf_len) <= data.size())
    {
        bytes_written = buf_len;
    }
    else
    {
        bytes_written = (data.size() - offset);
    }

    std::copy((data.begin() + offset),
                (data.begin() + offset + bytes_written),
                stdext::checked_array_iterator<uint8_t*>(buf, buf_len));

    return SUCCESS;
}


uint32_t FlashMemorySegment::get_address(void) const
{
    return addr;
}


uint32_t FlashMemorySegment::get_length(void) const
{
    return data.size();
}


bool FlashMemorySegment::has_address(uint32_t address) const
{
    if ((address >= addr) && (address < (addr + data.size())))
    {
        return true;
    }

    return false;
}


bool FlashMemorySegment::will_accept_address(uint32_t address) const
{
    // Return true if the address is contained by the segment or follows it immediately.
    return ((address >= addr) && (address <= (addr + data.size())));
}


bool FlashMemorySegment::can_remove(uint32_t address, uint32_t len) const
{
    if((0 == len) || (addr == address))
    {
        return true;
    }

    if (address < addr)
    {
        return ((address + len) > addr);
    }

    return ((addr + data.size() - len) == address);
}


uint32_t FlashMemorySegment::remove(uint32_t address, uint32_t len)
{
    std::vector<uint8_t>::iterator it;

    if (0 == len)
    {
        return 0;
    }

    if (address < addr)
    {
        len -= (addr - address);
        address = addr;
    }

    if (len > data.size())
    {
        len = data.size();
    }

    if (addr == address)
    {
        it = data.begin();
        data.erase(it, (it + len));
        addr += len;
    }
    else if ((addr + data.size() - len) == address)
    {
        // Remove from end.
        it = (data.end() - len);
        data.erase(it, data.end());
    }
    else
    {
        return 0;
    }

    return len;
}


FlashMemorySegment::Status FlashMemorySegment::split(uint32_t end_of_first_region,
                                                        uint32_t start_of_second_region,
                                                        FlashMemorySegment &first,
                                                        FlashMemorySegment &second)
{
    if ((end_of_first_region > data.size()) || (start_of_second_region > data.size()))
    {
        return DISCONTIGUOUS_ERROR;
    }

    if (end_of_first_region > start_of_second_region)
    {
        return DISCONTIGUOUS_ERROR;
    }

    if (first.data.size() < end_of_first_region)
    {
        try
        {
            first.data.resize(end_of_first_region);
        }
        catch (const std::exception)
        {
            return FlashMemorySegment::OUT_OF_MEMORY;
        }
    }

    if (second.data.size() < (data.size() - start_of_second_region))
    {
        try
        {
            second.data.resize(data.size() - start_of_second_region);
        }
        catch (const std::exception)
        {
            return FlashMemorySegment::OUT_OF_MEMORY;
        }
    }

    std::copy(data.begin(), (data.begin() + end_of_first_region), first.data.begin());
    std::copy((data.begin() + start_of_second_region), data.end(), second.data.begin());

    return SUCCESS;
}


bool FlashMemorySegment::operator< (const FlashMemorySegment& rhs) const
{
    return ((addr + data.size()) <= rhs.addr);
}


bool FlashMemorySegment::operator< (uint32_t address) const
{
    return ((addr + data.size()) <= address);
}
