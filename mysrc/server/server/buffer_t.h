#ifndef PSL_BUFFER_T_H_
#define PSL_BUFFER_T_H_

#include <inttypes.h>
class buffer_t
{
public:
    buffer_t(size_t buffer_size);
    buffer_t(const buffer_t & other);
    buffer_t(buffer_t && other);
    buffer_t & operator = (const buffer_t & other);
    buffer_t & operator = (buffer_t && other);
    ~buffer_t();
public:
    uint8_t * get_buffer();
    size_t get_data_size();
    size_t get_buffer_size();
    void copy_data_to_buffer(const void * pdata, size_t data_size);
    void set_data_size(size_t size);
private:
    uint8_t * pnew_buffer_;
    uint8_t pbuffer_[128];
    size_t data_size_;
    size_t buffer_size_;
};

#endif
