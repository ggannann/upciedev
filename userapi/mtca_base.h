#ifndef MTCA_BASE_H
#define MTCA_BASE_H

#include <stddef.h>
#ifdef WIN32
#else
#include <sys/types.h>
#endif
#include "pciedev_io.h"
#include <fcntl.h>

#define _TO_CLOSE_BIT_              0
#define _IS_OPEN_BIT_               1
#define _IS_CONNECTED_BIT_          2
#define _IS_IRQ_REQUEST_DONE_BIT_   3

#define _SET_BIT_FLAG_(_a_flag_,_a_bit_,_a_value_) \
{ \
    (_a_flag_) &= (~(1<<(_a_bit_))); \
    (_a_flag_) |= (((_a_value_)?1:0)<<(_a_bit_));\
}

#define _GET_BIT_FLAG_(_a_flag_,_a_bit_) ((1<<(_a_bit_)) & (_a_flag_))

namespace MTCA_CLASSES
{
class mtca_base
{
public:
    mtca_base():m_unFlags(0),m_nRegSizeMode(-1){}

    virtual ~mtca_base(){/*close();*/}

    virtual int open(const char* devName,int flag=O_RDWR)=0;

    virtual void close()=0;

    virtual int ioctl(unsigned long request,void*arg=NULL)=0;

    virtual int DeviceIoControl(unsigned long request, void* in, int inLen, void* out, int* outLen)=0;

    virtual int read(int bar, int offset, int numberOfRegs, void* buffer,int reg_size=0xff)const=0;

    virtual int write(int bar, int offset, int numberOfRegs, const void* buffer,int reg_size=0xff)=0;

    virtual int pread(int bar, int offset, int numberOfRegs, void* buffer,int reg_size=0xff)const=0;

    virtual int pwrite(int bar, int offset, int numberOfRegs, const void* buffer,int reg_size=0xff)=0;

    virtual int set_bits(int bar, int offset, int numberOfRegs, const void* buf, const void* mask,int reg_size=0xff)=0;

    virtual int swap_bits(int bar, int offset, int numberOfRegs, const void* mask,int reg_size=0xff)=0;

    virtual int single_ioc_access(int bar, int offset, int numberOfRegs, const void* data,const void* mask,int access_mode,int reg_size=0xff)=0;

    virtual int vector_rw(int rwNumber, device_ioc_rw* iocRW)=0;

    ///
    virtual int RegisterForInterrupts()=0;

    virtual void UnRegisterFromInterrupts()=0;

    virtual const char* GetInterupt(int timeout)=0;

    virtual int GetSwap()const=0;

    virtual void SetSwap(int swap)=0;

    virtual int  GetRegisterSizeMode()const=0;

protected:
    mutable unsigned int m_unFlags;
    mutable int          m_nRegSizeMode;
};
}

#endif // MTCA_BASE_H
