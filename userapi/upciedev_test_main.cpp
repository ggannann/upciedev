#include "upciedev_local.h"
#include <vector>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <malloc.h>

#define _GET_DATA_BY_MACROS_

//#define _MODE_FROM_SIZE_(_a_size_) ((_a_size_)==1 ? RW_D08 : ((_a_size_)==2 ? RW_D16 : RW_D32))
#define _ACCESS_MODE_(_a_mode_) ( (  (_a_mode_)==MTCA_SIMPLE_READ||(_a_mode_)==MTCA_SIMPLE_READ || (_a_mode_)==MTCA_SET_BITS|| (_a_mode_)==MTCA_SWAP_BITS  )  ?\
				(_a_mode_) : MTCA_SIMPLE_READ)
//#define _MODE_SNGL_(_a_size_vect_) ((_a_size_vect_).size() ? (_MODE_FROM_SIZE_((_a_size_vect_)[0])) : RW_D32)
#define _REG_SIZE_MODE_FIRST_(_a_size_vect_) ((_a_size_vect_).size() ? ((_a_size_vect_)[0]) : RW_D32)
#define _BAR_SNGL_(_a_bar_vect_) ((_a_bar_vect_).size() ? (_a_bar_vect_)[0] : 0)
#define _COUNT_SNGL_(_a_count_vect_) ((_a_count_vect_).size() ? \
    ((_a_count_vect_)[0]>0 ? (_a_count_vect_)[0] : 1) : 1)
#define _CREATE_VECTOR_(_a_mode_,_a_sz_,_d_v_) \
    ((((_a_mode_)&0x7)==RW_D08) ? ((MyVectorBase*)(new MyVector<u_int8_t>((_a_sz_),(u_int8_t)(_d_v_)))) : \
    ((((_a_mode_)&0x7)==RW_D16) ? ((MyVectorBase*)(new MyVector<u_int16_t>((_a_sz_),(u_int16_t)(_d_v_)))) : \
                                                    ((MyVectorBase*)(new MyVector<u_int32_t>((_a_sz_),(u_int32_t)(_d_v_))))  ) )
#define _FILL_VECT_PRVT_(_a_vect_out_ptr_,_a_vect_in_ptr_,_a_size_) \
{\
    /*(_a_vect_out_ptr_)->resize((_a_size_));*/ \
    for(size_t i(0); i<(_a_size_);++i) \
    {\
        (_a_vect_out_ptr_)->operator[](i) = (*(_a_vect_in_ptr_))[i]; \
    }\
}
#define _FILL_VECTOR_(_a_mode_out0_,_a_mode_in0_,_a_vect_out_ptr_,_a_vect_in_ptr_,_a_size_) \
{\
    u_int32_t _a_mode_out_(_a_mode_out0_&0x7);\
    u_int32_t _a_mode_in_(_a_mode_in0_&0x7);\
    if((_a_mode_out_)==RW_D08 && (_a_mode_in_)==RW_D32)\
    {\
        MyVector<u_int8_t>* pVectorOut = dynamic_cast<MyVector<u_int8_t>*>(_a_vect_out_ptr_);\
        MyVector<u_int32_t>* pVectorIn = dynamic_cast<MyVector<u_int32_t>*>(_a_vect_in_ptr_);\
        _FILL_VECT_PRVT_(pVectorOut,pVectorIn,(_a_size_));\
    } \
    else if((_a_mode_out_)==RW_D16 && (_a_mode_in_)==RW_D32)\
    {\
        MyVector<u_int16_t>* pVectorOut = dynamic_cast<MyVector<u_int16_t>*>(_a_vect_out_ptr_);\
        MyVector<u_int32_t>* pVectorIn = dynamic_cast<MyVector<u_int32_t>*>(_a_vect_in_ptr_);\
        _FILL_VECT_PRVT_(pVectorOut,pVectorIn,(_a_size_));\
    } \
    else if((_a_mode_out_)==RW_D32 && (_a_mode_in_)==RW_D08)\
    {\
        MyVector<u_int32_t>* pVectorOut = dynamic_cast<MyVector<u_int32_t>*>(_a_vect_out_ptr_);\
        MyVector<u_int8_t>* pVectorIn = dynamic_cast<MyVector<u_int8_t>*>(_a_vect_in_ptr_);\
        _FILL_VECT_PRVT_(pVectorOut,pVectorIn,(_a_size_));\
    } \
    else if((_a_mode_out_)==RW_D32 && (_a_mode_in_)==RW_D16)\
    {\
        MyVector<u_int32_t>* pVectorOut = dynamic_cast<MyVector<u_int32_t>*>(_a_vect_out_ptr_);\
        MyVector<u_int16_t>* pVectorIn = dynamic_cast<MyVector<u_int16_t>*>(_a_vect_in_ptr_);\
        _FILL_VECT_PRVT_(pVectorOut,pVectorIn,(_a_size_));\
    } \
    else if((_a_mode_out_)==RW_D32 && (_a_mode_in_)==RW_D32)\
    {\
        MyVector<u_int32_t>* pVectorOut = dynamic_cast<MyVector<u_int32_t>*>(_a_vect_out_ptr_);\
        MyVector<u_int32_t>* pVectorIn = dynamic_cast<MyVector<u_int32_t>*>(_a_vect_in_ptr_);\
        _FILL_VECT_PRVT_(pVectorOut,pVectorIn,(_a_size_));\
    } \
}
#ifdef _GET_DATA_BY_MACROS_
#define _GET_DATA_(_a_mode_,_a_vect_ptr_) \
    ((((_a_mode_)&0x7)==RW_D08) ? \
                (void*)((dynamic_cast<MyVector<u_int8_t>*>(_a_vect_ptr_))->data()) : \
    ((((_a_mode_)&0x7)==RW_D16) ? \
                (void*)((dynamic_cast<MyVector<u_int16_t>*>(_a_vect_ptr_))->data()) : \
                (void*)((dynamic_cast<MyVector<u_int32_t>*>(_a_vect_ptr_))->data()) ) )
#else
static void* _GET_DATA_(u_int32_t _a_mode_,class MyVectorBase* _a_vect_ptr_);
#endif
#define _SET_DATA_(_a_mode0_,_a_vect_base_ptr_,_a_index_,_a_data_) \
{\
    u_int32_t _a_mode_(_a_mode0_&0x7);\
    MyVector<u_int8_t>* pVec08;MyVector<u_int16_t>* pVec16;MyVector<u_int32_t>* pVec32;\
    switch(_a_mode_)\
    {\
    case RW_D08:\
        pVec08 = dynamic_cast<MyVector<u_int8_t>*>(_a_vect_base_ptr_);\
        pVec08->operator[](_a_index_) = (u_int8_t)(_a_data_); \
        break;\
    case RW_D16:\
        pVec16 = dynamic_cast<MyVector<u_int16_t>*>(_a_vect_base_ptr_);\
        pVec16->operator[](_a_index_) = (u_int16_t)(_a_data_); \
        break;\
    case RW_D32:\
        pVec32 = dynamic_cast<MyVector<u_int32_t>*>(_a_vect_base_ptr_);\
        pVec32->operator[](_a_index_) = (u_int32_t)(_a_data_); \
        break;\
    default: break;\
    }\
}

class MyVectorBase
{
public:
    virtual ~MyVectorBase(){}
};

static void printHelp();
template <typename INT_TYPE>
static inline INT_TYPE StrTolFormula(const char* a_str,char** a_final_ptr);

template <typename ContType>
class MyVector : public std::vector<ContType>, public MyVectorBase
{
public:
    MyVector(size_t a_size,const ContType& a_value) : std::vector<ContType>(a_size,a_value){}
};

typedef const char* cont_char_ptr;
static int ParseCommandLine(int argc, char* argv[],
                             cont_char_ptr* a_devName,
                             std::vector<u_int32_t>*a_pvRegSizes,
			     std::vector<u_int32_t>*a_pvAccessModes,
                             std::vector<u_int32_t>* a_pvBars,
                             std::vector<u_int32_t>* a_pvOffsets,
                             std::vector<u_int32_t>* a_pvCounts,
                             std::vector<MyVector<u_int32_t>*>* a_pvvData,
                             std::vector<MyVector<u_int32_t>*>* a_pvvMasks,
                             int* pnIoctlNum);

int main(int argc, char* argv[])
{
    if(argc<1)
    {
        fprintf(stderr,"Too low arguments!\n");
        printHelp();
        return 1;
    }

    const char* cpcErrorString = NULL;
    int nReturn = 0, i, i2, nSize, nSizeInner;
    int nSysCallReturn = 0;
    int nVerbose = 1;
    int nIoctlNum = -1;
    const char* cpcDeviceName = NULL;
    std::vector<u_int32_t> vRegisterSizes2;
    std::vector<u_int32_t> vAccessModes;
    std::vector<u_int32_t> vBars;
    std::vector<u_int32_t> vOffsets;
    std::vector<u_int32_t> vCounts;
    std::vector<MyVector<u_int32_t>*> vvDataIn0 ;
    std::vector<MyVector<u_int32_t>*> vvMasks0;
    std::vector<MyVectorBase*> vvDataInF ;
    std::vector<MyVectorBase*> vvMasksF;
    std::vector<MyVectorBase*> vvDataOut0 ;
    std::vector<MyVector<u_int32_t>*> vvDataOutF ;
    MyVector<u_int32_t>* pTemporal;

    if(ParseCommandLine(argc-1,argv+1,&cpcDeviceName,&vRegisterSizes2,&vAccessModes,&vBars,
                        &vOffsets,&vCounts,&vvDataIn0,&vvMasks0,&nIoctlNum))return 0;

    if(cpcDeviceName == NULL)
    {
        fprintf(stderr,"device entry name is not specified!\n");
        printHelp();
        return 1;
    }

    MTCA_CLASSES::upciedev_local aUpcieLocal;

    if(!aUpcieLocal.open(cpcDeviceName))
    {
        fprintf(stderr,"unable to open file \"%s\"\n",cpcDeviceName);
        perror("");
        return 2;
    }

    if(strcmp(argv[1],"read")==0)
    {
        if(vOffsets.size()==0){cpcErrorString = "Offset is not specified!";goto cleanBuffersPoint;}
        u_int32_t unRegSize = _REG_SIZE_MODE_FIRST_(vRegisterSizes2);
        u_int32_t unBar = _BAR_SNGL_(vBars);
        u_int32_t unOffset = vOffsets[0];
        u_int32_t unCount = _COUNT_SNGL_(vCounts);
        MyVectorBase* pvDataOut0 = _CREATE_VECTOR_(unRegSize,unCount,0);
        vvDataOut0.push_back(pvDataOut0);
        nSysCallReturn=aUpcieLocal.read(unBar,unOffset,unCount,_GET_DATA_(unRegSize,pvDataOut0),unRegSize);
        MyVector<u_int32_t>* pvDataOutF = new MyVector<u_int32_t>(unCount,0);
        vvDataOutF.push_back(pvDataOutF);
        _FILL_VECTOR_(RW_D32,unRegSize,pvDataOutF,pvDataOut0,unCount);
        goto printOutAndExit;
    }

    if(strcmp(argv[1],"write")==0)
    {
        if(vOffsets.size()==0){cpcErrorString = "Offset is not specified!";goto cleanBuffersPoint;}
        if(vvDataIn0.size()==0){cpcErrorString = "Data to write is not provided!";goto cleanBuffersPoint;}
        u_int32_t unRegSize = _REG_SIZE_MODE_FIRST_(vRegisterSizes2);
        u_int32_t unBar = _BAR_SNGL_(vBars);
        u_int32_t unOffset = vOffsets[0];
        u_int32_t unCount = _COUNT_SNGL_(vCounts);
        MyVector<u_int32_t>* pvDataIn0 = vvDataIn0[0];
        //unCount = unCount<=pvDataIn0->size() ? unCount : pvDataIn0->size();
        unCount=pvDataIn0->size();
        MyVectorBase* pvDataInF = _CREATE_VECTOR_(unRegSize,unCount,0);
        vvDataInF.push_back(pvDataInF);
        _FILL_VECTOR_(unRegSize,RW_D32,pvDataInF,pvDataIn0,unCount);
        nSysCallReturn=aUpcieLocal.write(unBar,unOffset,unCount,_GET_DATA_(unRegSize,pvDataInF),unRegSize);
        goto printOutAndExit;
    }

    if(strcmp(argv[1],"set-bits")==0)
    {
        if(vOffsets.size()==0){cpcErrorString = "Offset is not specified!";goto cleanBuffersPoint;}
        if(vvDataIn0.size()==0){cpcErrorString = "Data to write is not provided!";goto cleanBuffersPoint;}
        u_int32_t unRegSize = _REG_SIZE_MODE_FIRST_(vRegisterSizes2);
        u_int32_t unBar = _BAR_SNGL_(vBars);
        u_int32_t unOffset = vOffsets[0];
        u_int32_t unCount = _COUNT_SNGL_(vCounts);
        MyVector<u_int32_t>* pvDataIn0 = vvDataIn0[0];
        MyVector<u_int32_t>* pvMaskIn0 = vvMasks0.size() ? vvMasks0[0] : NULL;
        u_int32_t unCountMask = pvMaskIn0 ? pvMaskIn0->size() : 0;
        unCount = unCount<=pvDataIn0->size() ? unCount : pvDataIn0->size();
        MyVectorBase* pvDataInF = _CREATE_VECTOR_(unRegSize,unCount,0);
        vvDataInF.push_back(pvDataInF);
        _FILL_VECTOR_(unRegSize,RW_D32,pvDataInF,pvDataIn0,unCount);
        MyVectorBase* pvMaskInF = _CREATE_VECTOR_(unRegSize,unCount,0);
        vvMasksF.push_back(pvMaskInF);
        _FILL_VECTOR_(unRegSize,RW_D32,pvMaskInF,pvMaskIn0,unCountMask<unCount ? unCountMask : unCount);
        for(u_int32_t i3(unCountMask); i3<unCount;++i3)
        {
            _SET_DATA_(unRegSize,pvMaskInF,i3,0xffffffff);
        }
        nSysCallReturn=aUpcieLocal.set_bits(unBar,unOffset,unCount,
                                            _GET_DATA_(unRegSize,pvDataInF),_GET_DATA_(unRegSize,pvMaskInF),unRegSize);
        goto printOutAndExit;
    }

    if(strcmp(argv[1],"swap-bits")==0)
    {
        if(vOffsets.size()==0){cpcErrorString = "Offset is not specified!";goto cleanBuffersPoint;}
        if(vvMasks0.size()==0){cpcErrorString = "Mask for bit swap is not provided!";goto cleanBuffersPoint;}
        u_int32_t unRegSize = _REG_SIZE_MODE_FIRST_(vRegisterSizes2);
        u_int32_t unBar = _BAR_SNGL_(vBars);
        u_int32_t unOffset = vOffsets[0];
        u_int32_t unCount = _COUNT_SNGL_(vCounts);
        MyVector<u_int32_t>* pvMaskIn0 = vvMasks0[0];
        //unCount = unCount<=pvDataIn0->size() ? unCount : pvDataIn0->size();
        unCount=pvMaskIn0->size();
        MyVectorBase* pvMaskInF = _CREATE_VECTOR_(unRegSize,unCount,0);
        vvMasksF.push_back(pvMaskInF);
        _FILL_VECTOR_(unRegSize,RW_D32,pvMaskInF,pvMaskIn0,unCount);
        nSysCallReturn=aUpcieLocal.swap_bits(unBar,unOffset,unCount,_GET_DATA_(unRegSize,pvMaskInF),unRegSize);
        goto printOutAndExit;
    }

    if(strcmp(argv[1],"ioctl")==0)
    {
        if(nIoctlNum==-1){if(argc>3){nIoctlNum=StrTolFormula<int>(argv[2],NULL);}}
        void* pIocData = vvDataIn0.size() ? vvDataIn0[0]->data() : NULL;
        nSysCallReturn=aUpcieLocal.ioctl(nIoctlNum,pIocData);
        goto printOutAndExit;
    }

    if(strncmp(argv[1],"vector",6)==0)
    {
#if 1
        if(vOffsets.size()==0){cpcErrorString = "Offset is not specified!";goto cleanBuffersPoint;}
        const int cnRegSizeLen(vRegisterSizes2.size());
        std::vector<u_int32_t> vModes;
        u_int32_t unAccessMode;
        //u_int32_t unDataInpDefault = (vvDataIn0.size()&& vvDataIn0[0]->size()) ? (vvDataIn0[0])[0] : 0;
        u_int32_t unDataInpDefault = 0;
        int nRWnumber=0;
        int nWrCount = 0;
        int i4(6);
        for(; argv[1]+i4 != '\0';++i4)
        {
            unAccessMode = _ACCESS_MODE_(vAccessModes[nRWnumber]);
            if((argv[1])[i4]=='w' )
            {
               /*unMode += W_INITIAL;*/unAccessMode=MTCA_SIMPLE_WRITE;
	       ++nWrCount;
               vAccessModes.push_back(unAccessMode);
               ++nRWnumber;
            }//if((argv[1])[i4]=='r')
	    else if((argv[1])[i4]=='r')
	    {
               unAccessMode=MTCA_SIMPLE_READ;
               vAccessModes.push_back(unAccessMode);
               ++nRWnumber;
            }//if((argv[1])[i4]=='r')
        }//for(int i4(6); argv[1]+i4 != '\0';++i4)

        if(nWrCount && (vvDataIn0.size()==0)){cpcErrorString = "No data is provided for writing!";goto cleanBuffersPoint;}
        vBars.resize(nRWnumber,_BAR_SNGL_(vBars));
        vOffsets.resize(nRWnumber,vOffsets[0]);
        vCounts.resize(nRWnumber,_COUNT_SNGL_(vCounts));
        vvDataIn0.resize(nWrCount,NULL);
        vvMasks0.resize(nWrCount,NULL);

        int nWriteIndex = 0;
        device_ioc_rw* pIocRWdata = (device_ioc_rw*)calloc(nRWnumber,sizeof(device_ioc_rw));
        for(i4=0;i4<nRWnumber;++i4)
        {
            pIocRWdata[i4].register_size = vRegisterSizes2[i4];/* mode of rw (RW_D8, RW_D16, RW_D32)      */
	    pIocRWdata[i4].rw_access_mode = vAccessModes[i4];/* mode of rw (RW_D8, RW_D16, RW_D32)      */
            pIocRWdata[i4].barx_rw = vBars[i4];	/* BARx (0, 1, 2, 3, 4, 5)                 */
            pIocRWdata[i4].offset_rw = vOffsets[i4];	/* offset in address                       */
            pIocRWdata[i4].count_rw = vCounts[i4];	/* number of register to read or write     */
            //pIocRWdata[i4].dataPtr;	// In the case of more than one register access
            //u_int64_t		maskPtr;

            if(pIocRWdata[i4].rw_access_mode ==MTCA_SIMPLE_READ)
            {
                MyVectorBase* pvDataOut0 = _CREATE_VECTOR_(pIocRWdata[i4].register_size,pIocRWdata[i4].count_rw,0);
                vvDataOut0.push_back(pvDataOut0);
                pIocRWdata[i4].dataPtr = (u_int64_t)_GET_DATA_(pIocRWdata[i4].register_size,pvDataOut0);
            }
            else
            {
                MyVector<u_int32_t>* pvDataIn0 = vvDataIn0[nWriteIndex];
                MyVectorBase* pvDataInF = _CREATE_VECTOR_(pIocRWdata[i4].register_size,pIocRWdata[i4].count_rw,unDataInpDefault);
                vvDataInF.push_back(pvDataInF);
                if(pvDataIn0)
                {
                    pvDataIn0->resize(pIocRWdata[i4].count_rw,unDataInpDefault);
                    _FILL_VECTOR_(pIocRWdata[i4].register_size,RW_D32,pvDataInF,pvDataIn0,pIocRWdata[i4].count_rw);
                }
                pIocRWdata[i4].dataPtr = (u_int64_t)_GET_DATA_(pIocRWdata[i4].register_size,pvDataInF);

                MyVector<u_int32_t>* pvMaskIn0 = vvMasks0[nWriteIndex];
                MyVectorBase* pvMaskInF = _CREATE_VECTOR_(pIocRWdata[i4].register_size,pIocRWdata[i4].count_rw,0xffffffff);
                vvMasksF.push_back(pvMaskInF);
                if(pvMaskIn0)
                {
                    pvMaskIn0->resize(pIocRWdata[i4].count_rw,0xffffffff);
                    _FILL_VECTOR_(pIocRWdata[i4].register_size,RW_D32,pvMaskInF,pvMaskIn0,pIocRWdata[i4].count_rw);
                }
                pIocRWdata[i4].maskPtr = (u_int64_t)_GET_DATA_(pIocRWdata[i4].register_size,pvMaskInF);
                ++nWriteIndex;
            }
        }//for(i4=0;i4<nRWnumber;++i4)

        nSysCallReturn=aUpcieLocal.vector_rw(nRWnumber,pIocRWdata);

        int nRedIndex = 0;
        for(i4=0;i4<nRWnumber;++i4)
        {
            if(pIocRWdata[i4].rw_access_mode ==MTCA_SIMPLE_READ)
            {
                MyVectorBase* pvDataOut0 = vvDataOut0[nRedIndex];
                MyVector<u_int32_t>* pvDataOutF = new MyVector<u_int32_t>(pIocRWdata[i4].count_rw,0);
                vvDataOutF.push_back(pvDataOutF);
                _FILL_VECTOR_(RW_D32,pIocRWdata[i4].register_size,pvDataOutF,pvDataOut0,pIocRWdata[i4].count_rw);
            }
        }//for(i4=0;i4<nRWnumber;++i4)

#endif
    }//if(strncmp(argv[1],"vector",6)==0)

    if(strcmp(argv[1],"mmap")==0)
    {
        int nFd = (int)aUpcieLocal;
        char* pcBuffer = (char*)::mmap(NULL,1024,PROT_READ,MAP_SHARED,nFd,0);
        printf("mapped pointer: 0x%p\n",pcBuffer);
        if(pcBuffer)printf("mmap={0x%.2x,0x%.2x,...}\n",pcBuffer[0],pcBuffer[1]);
        goto printOutAndExit;
    }

printOutAndExit:
    aUpcieLocal.close();

    if(cpcErrorString)
    {
        fprintf(stderr,"Error: \"%s\", code=%d",cpcErrorString,nReturn);
        perror("\n");
        printHelp();
    }

    if(nVerbose)printf("action: %s, return: %d\n",argv[1],nSysCallReturn);

    nSize = vvDataOutF.size();
    for(i=0;i<nSize;++i)
    {
        pTemporal = vvDataOutF[i];
        printf("[");
        nSizeInner = pTemporal->size()-1;
        for(i2=0;i2<nSizeInner;++i2)
        {
            printf("0x%x,",pTemporal->operator [](i2));
        }printf("0x%x]\n",pTemporal->operator [](nSizeInner));
    }

cleanBuffersPoint:
    nSize = vvDataIn0.size();
    for(i=0;i<nSize;++i){delete vvDataIn0[i];}

    nSize = vvMasks0.size();
    for(i=0;i<nSize;++i){delete vvMasks0[i];}

    nSize = vvDataInF.size();
    for(i=0;i<nSize;++i){delete vvDataInF[i];}

    nSize = vvMasksF.size();
    for(i=0;i<nSize;++i){delete vvMasksF[i];}

    nSize = vvDataOut0.size();
    for(i=0;i<nSize;++i){delete vvDataOut0[i];}

    nSize = vvDataOutF.size();
    for(i=0;i<nSize;++i){delete vvDataOutF[i];}

    return nReturn;
}


////////////////////////////
///
///
#include <stdio.h>
static void printHelp()
{
    printf("\
program for testing upciedev driver!\n\
\n\
 Examples:\n\
  ./upciedev_test read -d /dev/sis8300s8 --offset 0 --count 4\n\
  ./upciedev_test write -d /dev/sis8300s8 --offset 2*4 --count 2 --data \"31 ; 0xfffffffe\"\n\
  ./upciedev_test set-bits -d /dev/sis8300s8 --offset 2*4 --count 2 --data \"0; 0x0\" --mask \"7;0xffffff00\"\n\
  ./upciedev_test swap-bits -d /dev/sis8300s8 --offset 2*4 --count 2 --mask \"63 ; 0xfffffffb\"\n\
  ./upciedev_test ioctl -d /dev/sis8300s8 --ioctl-number 2*4 --data \"3 ; 0x0a\"\n\
  ./upciedev_test vector_rws -d /dev/sis8300s8 --data \"31;0xfffffffe\" --data \"0;0x0\" --mask \"7;0xffffff00\"\n\
  ./upciedev_test mmap \n\
\n\
 Possible functions to call\n\
  1. read      :  Reads data from specifed registers\n\
  2. write     :  Writes data to the specified registers\n\
  3. set-bits  :  Sets individual bits to one or more registers\n\
  4. swap-bits :  Swaps the specified bits from selected registers\n\
  5. ioctl     :  Makes ioctl call to driver\n\
  6. vector    :  Makes several device acceses at once in atomic manner\n\
  7. mmap      :  Makes mmap system call to driver, and if succesfull shows content of mapped memory\n\
\n\
 Followings are the command line options: \n\
    --device (-d)          :  (No default): Specifies device name \n\
    --register-size (-rs)  :  (default: 4): Specifies the register size to access (1B,2B or 4B).\n\
                               In the case of vector access for each device access independent\n\
                               register size can be specified.\n\
    --bar (-b)             :  (default: 0): Specifies pci bar to acess (0,1,2,3,4 or 5). \n\
                               In the case of vector access for each access device bar\n\
                               can be specified separately.\n\
    --offset (-o)          :  (no default): Specifies the offset of first register to access.\n\
                               In the case of vector access for each access first register offset\n\
                               can be specified.\n\
    --register-offset (-ro):  (no default): Same as --offset.\n\
    --address              :  (no default): Same as --offset.\n\
    --count (-c)           :  (default: 1): Specifies the number of registers to access.\n\
                               In the case of vector access for each access number of registers\n\
                               can be specified.\n\
    --data (-dt)           :  (no default): Data to use in the case of write, set-bits or swap-bits access.\n\
                               In the case vector access for each write, set-bits and swap-bits accesses\n\
                               data should be proided!\n\
    --mask (-ms)           :  (default: 0xffffffff): Mask for set-bits and swap-bits access.\n\
    --ioctl-number (-in)   :  (no default):  Number of ioctl for ioctl call.\n\
    --help (-h)            :  To display this message and exit.\n"
           );
}


#include <stdlib.h>

template <typename INT_TYPE>
static inline void __MY_STRTOL__(const char* _a_str_,char** _a_final_ptr_,INT_TYPE* _a_data_ptr_,bool* _a_bOk_)
{
    char *_pcTemp1_, *_pcTemp2_;
    long _dataW1_, _dataW2_;
    bool bSuccedLoc;
    bool& bSucced = (_a_bOk_) ? *(_a_bOk_) : bSuccedLoc;

    bSucced = true;
    _dataW1_ = strtol((_a_str_),&_pcTemp1_,10);
    _dataW2_ = strtol((_a_str_),&_pcTemp2_,16);
    if((unsigned long)(_pcTemp2_ - (_a_str_)) == (unsigned long)(_pcTemp1_ - (_a_str_)))
    {
        if((_a_str_) == _pcTemp2_){bSucced = false;return;}
        *(_a_data_ptr_) = _dataW1_;
        if((_a_final_ptr_)){*(_a_final_ptr_) = _pcTemp1_;}
    }
    else
    {
        *(_a_data_ptr_) = _dataW2_;
        if((_a_final_ptr_)){*(_a_final_ptr_) = _pcTemp2_;}
    }
}


template <typename INT_TYPE>
static inline void __STRTOL_FORMULA__(const char* _a_str_,char** _a_final_ptr_,INT_TYPE* _a_data_ptr_,bool* _a_bOk_)
{
    char *pcFinal1, *pcFinal2;
    INT_TYPE tData2;

    __MY_STRTOL__<INT_TYPE>(_a_str_,&pcFinal1,_a_data_ptr_,_a_bOk_);
    if(!(*_a_bOk_))return;
    pcFinal2 = pcFinal1;
    for(;*pcFinal2!='*' && *pcFinal2!='\0';++pcFinal2);
    if(*pcFinal2=='\0'){if(_a_final_ptr_){*_a_final_ptr_ = pcFinal1;}return;}

    __MY_STRTOL__<INT_TYPE>(pcFinal2 + 1,_a_final_ptr_,&tData2,_a_bOk_);
    if(*_a_bOk_){(*_a_data_ptr_) *= tData2;}
}


template <typename INT_TYPE>
static inline INT_TYPE StrTolFormula(const char* a_str,char** a_final_ptr)
{
    INT_TYPE tReturn;
    __STRTOL_FORMULA__(a_str,a_final_ptr,&tReturn,NULL);
    return tReturn;
}


template <typename INT_TYPE>
static void LoadNumbers(const char* a_str,std::vector<INT_TYPE>* a_pvNumbers)
{
    bool bReturn;
    std::vector<INT_TYPE>& vNumbers = *a_pvNumbers;
    char* pcFinal;
    const char* cpcBegin;
    INT_TYPE tData;

    __STRTOL_FORMULA__<INT_TYPE>(a_str,&pcFinal,&tData,&bReturn);

    if(!bReturn)
    {
        cpcBegin = strchr(a_str,'{');
        if(!cpcBegin)return;

        __STRTOL_FORMULA__<INT_TYPE>(cpcBegin+1,&pcFinal,&tData,&bReturn);
        if(!bReturn)return;
    }

    while(bReturn)
    {
        vNumbers.push_back(tData);
        for(;*pcFinal!=';' && *pcFinal!=',' && *pcFinal!='\0'; ++pcFinal);
        if(*pcFinal == '\0')return;
        cpcBegin = pcFinal+1;
        __STRTOL_FORMULA__<INT_TYPE>(cpcBegin,&pcFinal,&tData,&bReturn);
    }

}


//typedef const char* cont_char_ptr;
static int ParseCommandLine(int argc, char* argv[],
                             cont_char_ptr* a_devName,
                             std::vector<u_int32_t>*a_pvRegSizes,
			     std::vector<u_int32_t>*a_pvAccessModes,
                             std::vector<u_int32_t>* a_pvBars,
                             std::vector<u_int32_t>* a_pvOffsets,
                             std::vector<u_int32_t>* a_pvCounts,
                             std::vector<MyVector<u_int32_t>*>* a_pvvData,
                             std::vector<MyVector<u_int32_t>*>* a_pvvMasks,
                             int* a_pnIoctlNum)
{
    const char*& cpcDeviceName = *a_devName;
    std::vector<u_int32_t>& vRegSizes = *a_pvRegSizes;
    std::vector<u_int32_t>& vAccessModes = *a_pvAccessModes;
    std::vector<u_int32_t>& vBars = *a_pvBars;
    std::vector<u_int32_t>& vOffsets = *a_pvOffsets;
    std::vector<u_int32_t>& vCounts = *a_pvCounts;
    std::vector<MyVector<u_int32_t>*>& vvData = *a_pvvData;
    std::vector<MyVector<u_int32_t>*>& vvMasks = *a_pvvMasks;

    for( int i(0); i < argc; ++i )
    {
        // Device and host name
        if( strcmp(argv[i],"--device") == 0 || strcmp(argv[i],"-d") == 0)
        {
            if(i<argc-1){cpcDeviceName = argv[++i];}
            continue;
        }

        // register sizes, default known by driver
        if( strcmp(argv[i],"--register-size") == 0 || strcmp(argv[i],"-rs") == 0)
        {
            if(i<argc-1){LoadNumbers<u_int32_t>(argv[++i],&vRegSizes);}
            continue;
        } 
	
	// mode
        if( strcmp(argv[i],"--access-mode") == 0 || strcmp(argv[i],"-am") == 0)
        {
            if(i<argc-1){LoadNumbers<u_int32_t>(argv[++i],&vAccessModes);}
            continue;
        }

        // bar
        if( strcmp(argv[i],"--bar") == 0 || strcmp(argv[i],"-b") == 0)
        {
            if(i<argc-1){LoadNumbers<u_int32_t>(argv[++i],&vBars);}
            continue;
        }

        // offset
        if( strcmp(argv[i],"--offset") == 0 || strcmp(argv[i],"-o") == 0 || strcmp(argv[i],"-ro") == 0 ||
                strcmp(argv[i],"--register-offset") == 0 || strcmp(argv[i],"-arrdess") == 0)
        {
            if(i<argc-1){LoadNumbers<u_int32_t>(argv[++i],&vOffsets);}
            continue;
        }

        // count
        if( strcmp(argv[i],"--count") == 0 || strcmp(argv[i],"-c") == 0)
        {
            if(i<argc-1){LoadNumbers<u_int32_t>(argv[++i],&vCounts);}
            continue;
        }

        // data
        if( strcmp(argv[i],"--data") == 0 || strcmp(argv[i],"-dt") == 0)
        {
            MyVector<u_int32_t>*   pData = new MyVector<u_int32_t>(0,0);
            if(i<argc-1){LoadNumbers<u_int32_t>(argv[++i],pData);}
            vvData.push_back(pData);
            continue;
        }

        // mask
        if( strcmp(argv[i],"--mask") == 0 || strcmp(argv[i],"-ms") == 0)
        {
            MyVector<u_int32_t>*   pMask = new MyVector<u_int32_t>(0,0);
            if(i<argc-1){LoadNumbers<u_int32_t>(argv[++i],pMask);}
            vvMasks.push_back(pMask);
            continue;
        }

        // ioctl-number
        if( strcmp(argv[i],"--ioctl-number") == 0 || strcmp(argv[i],"-in") == 0)
        {
            if(i<argc-1){__STRTOL_FORMULA__(argv[++i],NULL,a_pnIoctlNum,NULL);}
            continue;
        }

        // help
        if( strcmp(argv[i],"--help") == 0 || strcmp(argv[i],"-h") == 0)
        {
            printHelp();
            return 1;
        }

    } // for( int i(1); i < argc; ++i )

    return 0;
}


#ifndef _GET_DATA_BY_MACROS_
static void* _GET_DATA_(u_int32_t _a_mode_,class MyVectorBase* _a_vect_ptr_)
{
    if()
    return (((_a_mode_)==RW_D08) ? (void*)((dynamic_cast<MyVector<u_int8_t>*>(_a_vect_ptr_))->data()) : \
    (((_a_mode_)==RW_D16) ? (void*)((dynamic_cast<MyVector<u_int16_t>*>(_a_vect_ptr_))->data()) : \
                            (void*)((dynamic_cast<MyVector<u_int16_t>*>(_a_vect_ptr_))->data()) ) );
}
#endif
