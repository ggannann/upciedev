/*****************************************************************************
 * File		  sis8300.h
 * created on 2015-07-31
 *****************************************************************************
 * Author:	D.Kalantaryan, Tel:033762/77552 kalantar
 * Email:	davit.kalantaryan@desy.de
 * Mail:	DESY, Platanenallee 6, 15738 Zeuthen
 *****************************************************************************
 * Description
 *   file for ...
 ****************************************************************************/
//#include "stdafx.h"
#include "upciedev_local.h"

#include <memory.h>
#include <mtcagen_io.h>

#ifdef WIN32
#include <WINDOWS.H>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
//#define open_cm    _open
#define close_cm   _close
#define read_cm    _read
#define write_cm   _write
static inline int open_cm(const char* a_filename,int a_flags,int a_mode=_S_IREAD|_S_IWRITE ){int nFD;_sopen_s(&nFD,a_filename,a_flags,_SH_DENYNO,a_mode);return nFD;}
/// Piti nayvi
#define __off_t long
static int pread(int a_fd,void* a_buf,size_t a_count, __off_t a_offset){_lseek(a_fd,a_offset,SEEK_SET);return read_cm(a_fd,a_buf,a_count);}
static int pwrite(int a_fd,const void* a_buf,size_t a_count, __off_t a_offset){_lseek(a_fd,a_offset,SEEK_SET);return write_cm(a_fd,a_buf,a_count);}
#else
#include <sys/mman.h>
#define open_cm    open
#define close_cm   close
#define read_cm    read
#define write_cm   write
#endif

using namespace MTCA_CLASSES;


upciedev_local::upciedev_local(int a_fd)
    :   m_fd(a_fd), m_pcShared(NULL), m_nIRQtype(0)
{
}



upciedev_local::upciedev_local(const char* a_cpcDeviceName,int a_nFlags)
    :   m_fd(-1), m_pcShared(NULL)
{
    if(a_cpcDeviceName){this->open(a_cpcDeviceName,a_nFlags);}
}



upciedev_local::~upciedev_local()
{
    this->close();
}


//_SET_BIT_FLAG_(_a_flag_,_a_bit_,_a_value_)
int upciedev_local::open(const char* a_file,int a_flags)
{
    this->close();
    m_fd = ::open_cm(a_file,a_flags);
    if(m_fd>=0){_SET_BIT_FLAG_(m_unFlags,_TO_CLOSE_BIT_,1);}
    return m_fd;
}


//#define _GET_BIT_FLAG_(_a_flag_,_a_bit_) ((1<<(_a_bit_)) & (_a_flag_))
void upciedev_local::close()
{    
    if((_GET_BIT_FLAG_(m_unFlags,_TO_CLOSE_BIT_) && (::close_cm(m_fd)==0)) || !_GET_BIT_FLAG_(m_unFlags,_TO_CLOSE_BIT_))
    {
        m_fd=-1;
        _SET_BIT_FLAG_(m_unFlags,_TO_CLOSE_BIT_,0);
    }
}


int upciedev_local::ioctl(unsigned long a_request,void* a_arg)
{
#ifdef WIN32
    //unsigned long unCommand = _IOWR(PCIE_GEN_IOC, a_ulnCommandPrvt, int);
    DWORD dwReturned;
    BOOL bRet = ::DeviceIoControl( (HANDLE)_get_osfhandle(m_fd),a_request,a_arg,0,a_arg,0,&dwReturned,NULL);
    return bRet>=0 ? bRet : (GetLastError()>0 ? -((int)GetLastError()) : ((int)GetLastError()));
#else
    return ::ioctl(m_fd,a_request,a_arg);
#endif
}



int upciedev_local::DeviceIoControl(unsigned long a_request, void* a_in, int a_inLen, void* a_out, int* a_outLen)
{
    int nOutBufLen = a_outLen ? *a_outLen : 0;
    int code;
#ifdef WIN32
    DWORD dwReturned;
    BOOL bRet = ::DeviceIoControl( (HANDLE)_get_osfhandle(m_fd),a_request,a_in,a_inLen,
                                 a_out,nOutBufLen,&dwReturned,NULL);
    if(a_outLen)*a_outLen = dwReturned;

    code = bRet>=0 ? bRet : (GetLastError()>0 ? -((int)GetLastError()) : ((int)GetLastError()));
#else
    void* pInput;
    int nLen;

    if(nOutBufLen>a_inLen)
    {
        pInput = a_out;
        if(a_inLen){memcpy(pInput,a_in,a_inLen);}
        nLen = nOutBufLen;
    }
    else
    {
        pInput = a_in;
        nLen = a_inLen;
    }

    if(nLen>0 && pInput) code = ::ioctl (m_fd, a_request, pInput);
    else code = ::ioctl (m_fd, a_request);

    if(nOutBufLen && a_inLen>=nOutBufLen){memcpy(a_out,a_in,nOutBufLen);}
#endif
    return code;
}



int upciedev_local::GetRegAccesSizeInBytes(int a_register_size_mode)const
{
    if(m_nRegSizeMode<0)
    {
#ifdef WIN32
        DWORD dwReturned;
        m_nRegSizeMode = ::DeviceIoControl( (HANDLE)_get_osfhandle(m_fd),PCIEDEV_GET_REGISTER_SIZE,
                                            &a_register_size_mode,0,NULL,0,&dwReturned,NULL);
#else
        m_nRegSizeMode = ::ioctl(m_fd,PCIEDEV_GET_REGISTER_SIZE);
#endif
    }
    return GetRegisterSizeInBytes(a_register_size_mode,m_nRegSizeMode);
}


int upciedev_local::GetDevNo()const
{

    if(m_fd<0) return 0;

    struct stat aStat;
    aStat.st_rdev = 0;
    fstat(m_fd,&aStat);

    return (int)aStat.st_rdev;
}


int upciedev_local::read(int a_bar, int a_offset, int a_numberOfRegs, void* a_buffer,int a_reg_size)const
{
    device_rw aRWstruct;

    aRWstruct.offset_rw = a_offset;
    aRWstruct.register_size = a_reg_size;
    aRWstruct.barx_rw = a_bar;
    aRWstruct.dataPtr = (u_int64_t)a_buffer;

    int nReturn = ::read_cm(m_fd,&aRWstruct,a_numberOfRegs);

    if(a_numberOfRegs<2)
    {
        int nRegSize = GetRegAccesSizeInBytes(a_reg_size);
        memcpy(a_buffer,&aRWstruct.data_rw,nRegSize);
    }

    return nReturn;
}



int upciedev_local::write(int a_bar, int a_offset, int a_numberOfRegs, const void* a_buffer,int a_reg_size)
{
    device_rw aRWstruct;

    aRWstruct.offset_rw = a_offset;
    if(a_numberOfRegs<2)
    {
        int nRegSize = GetRegAccesSizeInBytes(a_reg_size);
        memcpy(&aRWstruct.data_rw,a_buffer,nRegSize);
    }
    aRWstruct.mode_rw = a_reg_size;
    aRWstruct.barx_rw = a_bar;
    aRWstruct.dataPtr = (u_int64_t)a_buffer;

    return ::write_cm(m_fd,&aRWstruct,a_numberOfRegs);
}



int upciedev_local::pread(int a_bar, int a_offset, int a_numberOfRegs, void* a_buffer,int a_reg_size)const
{
    u_int64_t	reg_size = ((u_int64_t)a_reg_size)<<PRW_REG_SIZE_SHIFT;
    u_int64_t	barx_rw = ((u_int64_t)a_bar)<<PRW_BAR_SHIFT;
    u_int64_t	offset_rw = (u_int64_t)a_offset;
    u_int64_t   llnPos = (reg_size&PRW_REG_SIZE_MASK)|(barx_rw&PRW_BAR_MASK)|(offset_rw&PRW_OFFSET_MASK);

    return ::pread(m_fd,a_buffer,a_numberOfRegs,(__off_t)llnPos);
}



int upciedev_local::pwrite(int a_bar, int a_offset, int a_numberOfRegs, const void* a_buffer,int a_reg_size)
{
    u_int64_t	reg_size = ((u_int64_t)a_reg_size)<<PRW_REG_SIZE_SHIFT;
    u_int64_t	barx_rw = ((u_int64_t)a_bar)<<PRW_BAR_SHIFT;
    u_int64_t	offset_rw = (u_int64_t)a_offset;
    u_int64_t   llnPos = (reg_size&PRW_REG_SIZE_MASK)|(barx_rw&PRW_BAR_MASK)|(offset_rw&PRW_OFFSET_MASK);

    return ::pwrite(m_fd,a_buffer,a_numberOfRegs,(__off_t)llnPos);
}



int upciedev_local::set_bits(int a_bar, int a_offset, int a_numberOfRegs, const void* a_buf, const void* a_mask,int a_reg_size)
{
    device_ioc_rw aRWioc;
    //aRWioc.mode_rw = (a_mode&_STEP_FOR_NEXT_MODE2_)+W_BITS_INITIAL;
    aRWioc.register_size = a_reg_size;
    aRWioc.barx_rw = a_bar;
    aRWioc.offset_rw = a_offset;
    aRWioc.count_rw = a_numberOfRegs;
    aRWioc.dataPtr = (u_int64_t)a_buf;
    aRWioc.maskPtr = (u_int64_t)a_mask;
    return this->ioctl(PCIEDEV_SET_BITS,&aRWioc);
}


int upciedev_local::swap_bits(int a_bar, int a_offset, int a_numberOfRegs, const void* a_mask,int a_reg_size)
{
    device_ioc_rw aRWioc;
    aRWioc.register_size = a_reg_size;
    aRWioc.barx_rw = a_bar;
    aRWioc.offset_rw = a_offset;
    aRWioc.count_rw = a_numberOfRegs;
    aRWioc.dataPtr = 0;
    aRWioc.maskPtr = (u_int64_t)a_mask;
    return this->ioctl(PCIEDEV_SWAP_BITS,&aRWioc);
}



int upciedev_local::single_ioc_access(int a_bar, int a_offset, int a_numberOfRegs, const void* a_data,const void* a_mask,int a_access_mode,int a_reg_size)
{
    device_ioc_rw aRWioc;
    aRWioc.rw_access_mode = a_access_mode;
    aRWioc.register_size = a_reg_size;
    aRWioc.barx_rw = a_bar;
    aRWioc.offset_rw = a_offset;
    aRWioc.count_rw = a_numberOfRegs;
    aRWioc.dataPtr = (u_int64_t)a_data;
    aRWioc.maskPtr = (u_int64_t)a_mask;
    return this->ioctl(PCIEDEV_SINGLE_IOC_ACCESS,&aRWioc);
}



int upciedev_local::vector_rw(int a_rwNumber, device_ioc_rw* a_iocRW)
{
    device_vector_rw aVectorRW;
    aVectorRW.number_of_rw = a_rwNumber;
    aVectorRW.device_ioc_rw_ptr = (u_int64_t)a_iocRW;
    return this->ioctl(PCIEDEV_VECTOR_RW,&aVectorRW);
}


int upciedev_local::lock_device()
{
    return this->ioctl(PCIEDEV_LOCK_DEVICE);
}


int upciedev_local::unlock_device()
{
    return this->ioctl(PCIEDEV_UNLOCK_DEVICE);
}



upciedev_local& upciedev_local::operator=(const int& a_fd)
{
    this->close();
    m_fd = a_fd;
    return *this;
}


upciedev_local::operator const int&()const
{
    return m_fd;
}



int upciedev_local::GetSwap()const
{
    if(m_fd<=0)return 0;
#ifdef WIN32
    DWORD dwReturned;
    return ::DeviceIoControl( (HANDLE)_get_osfhandle(m_fd),PCIEDEV_GET_SWAP2,NULL,0,
                              NULL,0,&dwReturned,NULL);
#else
    return ::ioctl(m_fd,PCIEDEV_GET_SWAP2);
#endif
}


void upciedev_local::SetSwap(int a_nSwap)
{
    if(m_fd<=0)return;
#ifdef WIN32
    DWORD nReturned;
    ::DeviceIoControl( (HANDLE)_get_osfhandle(m_fd),PCIEDEV_SET_SWAP2,&a_nSwap,4,
                              NULL,0,&nReturned,NULL);
#else
    ::ioctl(m_fd,PCIEDEV_SET_SWAP2,&a_nSwap);
#endif
}



int upciedev_local::GetRegisterSizeMode()const
{
    if(m_fd<=0)return RW_D32;
#ifdef WIN32
    DWORD dwReturned;
    m_nRegSizeMode = ::DeviceIoControl( (HANDLE)_get_osfhandle(m_fd),PCIEDEV_GET_REGISTER_SIZE,NULL,0,
                                        NULL,0,&dwReturned,NULL);
#else
    m_nRegSizeMode = ::ioctl(m_fd,PCIEDEV_GET_REGISTER_SIZE);
#endif
    return m_nRegSizeMode;
}



int upciedev_local::RegisterForInterrupts()
{
    if(m_nIRQtype<=0)m_nIRQtype = this->ioctl(MTCA_GET_IRQ_TYPE);

    struct
    {
        int64_t llnSigNum;
        u_int64_t ullnCallback;
    }aType1;

    switch(m_nIRQtype)
    {
    case _IRQ_TYPE1_:
        aType1.llnSigNum = SIGUSR1;
        aType1.ullnCallback = (u_int64_t)this;
        return this->ioctl(GEN_REGISTER_USER_FOR_IRQ,&aType1);
    case _IRQ_TYPE2_:
#ifdef WIN32
#else
        if(m_fd>=0)m_pcShared = (char*)mmap(NULL,1024,PROT_READ,MAP_SHARED,m_fd,0);
#endif
        return m_pcShared ? 0 : -1;
    default: break;
    }
    return -2;
}


void upciedev_local::UnRegisterFromInterrupts()
{
    char* pcShared;
    //if(m_nIRQtype<=0)m_nIRQtype = this->ioctl(MTCA_GET_IRQ_TYPE);

    switch(m_nIRQtype)
    {
    case _IRQ_TYPE1_:
        this->ioctl(GEN_UNREGISTER_USER_FROM_IRQ);
    case _IRQ_TYPE2_:
#ifdef WIN32
        /// Piti arvi
        m_pcShared = NULL;
#else
        pcShared = m_pcShared;
        m_pcShared = NULL;
        if(pcShared)munmap(pcShared,1024);
#endif
    default: break;
    }
}



const char* upciedev_local::GetInterupt(int a_timeout)
{
#ifdef WIN32
    /// Piti arvi
    Sleep(a_timeout);
    return "Not implemenetde";
#else
#define __DEBUG__
    int nReturn;
    struct timespec aTimeout;
    sigset_t aSigSet;
#ifdef __DEBUG__
    const char* cpcReturn;
#endif
    if(m_nIRQtype<=0)m_nIRQtype = this->ioctl(MTCA_GET_IRQ_TYPE);

    switch(m_nIRQtype)
    {
    case _IRQ_TYPE1_:
        sigemptyset(&aSigSet);
        sigaddset(&aSigSet, SIGUSR1);
        aTimeout.tv_sec = a_timeout / 1000;
        aTimeout.tv_nsec = (a_timeout % 1000) * 1000000;
        nReturn = sigtimedwait(&aSigSet,&m_SigInfo,&aTimeout);
#ifdef __DEBUG__
        cpcReturn = (const char*)((m_SigInfo._sifields._pad));
        return nReturn == SIGUSR1 ? cpcReturn : NULL;
#else
        return nReturn==SIGUSR1 ? (const char*)&m_SigInfo : NULL;
#endif
    case _IRQ_TYPE2_:
        /*if(m_pcShared)*/ nReturn = this->ioctl(MTCA_WAIT_FOR_IRQ2_TIMEOUT,&a_timeout);
        return nReturn>0 ? m_pcShared : NULL;
        break;
    default: break;
    }

    return NULL;
#endif
}
