#ifndef UPCIEDEV_CLASS_H
#define UPCIEDEV_CLASS_H


#ifdef WIN32
#include <io.h>
#include <linux/ioctl.h>
//#include <sys/utime.h>
#else
#include <unistd.h>
#include <sys/ioctl.h>
#endif

#include <signal.h>
#include "mtca_base.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "pciedev_io.h"

namespace MTCA_CLASSES
{
class upciedev_local : public mtca_base
{
public:
    upciedev_local(int a_fd=-1);

    upciedev_local(const char* devName,int flag=O_RDWR);

    virtual ~upciedev_local();

    int open(const char* devName,int flag=O_RDWR);

    void close();

    int ioctl(unsigned long request,void*arg=NULL);

    int DeviceIoControl(unsigned long request, void* in, int inLen, void* out, int* outLen);

    int read(int bar, int offset, int numberOfRegs, void* buffer, int reg_size=0xff)const;

    int write(int bar, int offset, int numberOfRegs, const void* buffer, int reg_size=0xff);

    int pread(int bar, int offset, int numberOfRegs, void* buffer,int reg_size=0xff)const;

    int pwrite(int bar, int offset, int numberOfRegs, const void* buffer,int reg_size=0xff);

    int set_bits(int bar, int offset, int numberOfRegs, const void* buf, const void* mask,int reg_size=0xff);

    int swap_bits(int bar, int offset, int numberOfRegs, const void* mask,int reg_size=0xff);

    int single_ioc_access(int bar, int offset, int numberOfRegs, const void* data,const void* mask,int access_mode,int reg_size=0xff);

    int vector_rw(int rwNumber, device_ioc_rw* iocRW);

    int lock_device();

    int unlock_device();

    operator const int&()const;

    upciedev_local& operator=(const int& fd);

    int GetSwap()const;

    void SetSwap(int swap);

    int  GetRegisterSizeMode()const;

    virtual int RegisterForInterrupts();

    virtual void UnRegisterFromInterrupts();

    virtual const char* GetInterupt(int timeout);

    ///////////////////////////////
    int GetRegAccesSizeInBytes(int register_size_mode)const;

    int GetDevNo()const;

protected:
    int     m_fd;
    union
    {
        char*   m_pcShared;
        siginfo_t   m_SigInfo;
    };
    int     m_nIRQtype;
};
}

#endif // UPCIEDEV_CLASS_H
