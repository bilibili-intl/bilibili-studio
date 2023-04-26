#pragma once

#define MAX_GPU_NUM     4L      // ��ص�GPU������NVIDIA��������GPU������64���������ֻ���4����
#define MY_PROCESS_ERROR(Condition) do { if (!(Condition)) goto Exit0; } while (false)

#define MAX_DISPLAY_CARDS               4       // �����4���Կ�����ʱӦ�ù��˰ɣ�
// ��������
#define MAX_PHYSICAL_GPUS               64
#define SHORT_STRING_MAX                64
#define MAX_THERMAL_SENSORS_PER_GPU     3
#define MAX_CLOCKS_PER_GPU              0x120
#define MAX_PSTATES_PER_GPU             8
#define MAX_USAGES_PER_GPU              33
#define MAX_COOLER_PER_GPU              20
#define MAX_MEMORY_VALUES_PER_GPU       5

// �ӿ�IDֵ
#define ID_NvAPI_Initialize                     0x0150E828
#define ID_NvAPI_GPU_GetFullName                0xCEEE8E9F
#define ID_NvAPI_GPU_GetThermalSettings         0xE3640A56
#define ID_NvAPI_EnumNvidiaDisplayHandle        0x9ABDD40D
#define ID_NvAPI_GetPhysicalGPUsFromDisplay     0x34EF9506
#define ID_NvAPI_EnumPhysicalGPUs               0xE5AC921F
#define ID_NvAPI_GPU_GetTachReading             0x5F608315
#define ID_NvAPI_GPU_GetAllClocks               0x1BD69F49
#define ID_NvAPI_GPU_GetPStates                 0x60DED2ED
#define ID_NvAPI_GPU_GetUsages                  0x189A1FDF
#define ID_NvAPI_GPU_GetCoolerSettings          0xDA141340
#define ID_NvAPI_GPU_SetCoolerLevels            0x891FA0AE
#define ID_NvAPI_GPU_GetMemoryInfo              0x774AA982
#define ID_NvAPI_GetDisplayDriverVersion        0xF951A4D1
#define ID_NvAPI_GetInterfaceVersionString      0x01053FA5
#define ID_NvAPI_GPU_GetPCIIdentifiers          0x2DDFB66E

// �汾�Ų�������
#define GPU_THERMAL_SETTINGS_VER                (sizeof(NvGPUThermalSettings) | 0x10000)
#define GPU_CLOCKS_VER                          (sizeof(NvClocks) | 0x20000)
#define GPU_PSTATES_VER                         (sizeof(NvPStates) | 0x10000)
#define GPU_USAGES_VER                          (sizeof(NvUsages) | 0x10000)
#define GPU_COOLER_SETTINGS_VER                 (sizeof(NvGPUCoolerSettings) | 0x20000)
#define GPU_MEMORY_INFO_VER                     (sizeof(NvMemoryInfo) | 0x20000)
#define DISPLAY_DRIVER_VERSION_VER              (sizeof(NvDisplayDriverVersion) | 0x10000)
#define GPU_COOLER_LEVELS_VER                   (sizeof(NvGPUCoolerLevels) | 0x10000)

enum NvStatus
{
    enumNvStatus_OK = 0,
    enumNvStatus_ERROR = -1,
    enumNvStatus_LIBRARY_NOT_FOUND = -2,
    enumNvStatus_NO_IMPLEMENTATION = -3,
    enumNvStatus_API_NOT_INTIALIZED = -4,
    enumNvStatus_INVALID_ARGUMENT = -5,
    enumNvStatus_NVIDIA_DEVICE_NOT_FOUND = -6,
    enumNvStatus_END_ENUMERATION = -7,
    enumNvStatus_INVALID_HANDLE = -8,
    enumNvStatus_INCOMPATIBLE_STRUCT_VERSION = -9,
    enumNvStatus_HANDLE_INVALIDATED = -10,
    enumNvStatus_OPENGL_CONTEXT_NOT_CURRENT = -11,
    enumNvStatus_NO_GL_EXPERT = -12,
    enumNvStatus_INSTRUMENTATION_DISABLED = -13,
    enumNvStatus_EXPECTED_LOGICAL_GPU_HANDLE = -100,
    enumNvStatus_EXPECTED_PHYSICAL_GPU_HANDLE = -101,
    enumNvStatus_EXPECTED_DISPLAY_HANDLE = -102,
    enumNvStatus_INVALID_COMBINATION = -103,
    enumNvStatus_NOT_SUPPORTED = -104,
    enumNvStatus_PORTID_NOT_FOUND = -105,
    enumNvStatus_EXPECTED_UNATTACHED_DISPLAY_HANDLE = -106,
    enumNvStatus_INVALID_PERF_LEVEL = -107,
    enumNvStatus_DEVICE_BUSY = -108,
    enumNvStatus_NV_PERSIST_FILE_NOT_FOUND = -109,
    enumNvStatus_PERSIST_DATA_NOT_FOUND = -110,
    enumNvStatus_EXPECTED_TV_DISPLAY = -111,
    enumNvStatus_EXPECTED_TV_DISPLAY_ON_DCONNECTOR = -112,
    enumNvStatus_NO_ACTIVE_SLI_TOPOLOGY = -113,
    enumNvStatus_SLI_RENDERING_MODE_NOTALLOWED = -114,
    enumNvStatus_EXPECTED_DIGITAL_FLAT_PANEL = -115,
    enumNvStatus_ARGUMENT_EXCEED_MAX_SIZE = -116,
    enumNvStatus_DEVICE_SWITCHING_NOT_ALLOWED = -117,
    enumNvStatus_TESTING_CLOCKS_NOT_SUPPORTED = -118,
    enumNvStatus_UNKNOWN_UNDERSCAN_CONFIG = -119,
    enumNvStatus_TIMEOUT_RECONFIGURING_GPU_TOPO = -120,
    enumNvStatus_DATA_NOT_FOUND = -121,
    enumNvStatus_EXPECTED_ANALOG_DISPLAY = -122,
    enumNvStatus_NO_VIDLINK = -123,
    enumNvStatus_REQUIRES_REBOOT = -124,
    enumNvStatus_INVALID_HYBRID_MODE = -125,
    enumNvStatus_MIXED_TARGET_TYPES = -126,
    enumNvStatus_SYSWOW64_NOT_SUPPORTED = -127,
    enumNvStatus_IMPLICIT_SET_GPU_TOPOLOGY_CHANGE_NOT_ALLOWED = -128,
    enumNvStatus_REQUEST_USER_TO_CLOSE_NON_MIGRATABLE_APPS = -129,
    enumNvStatus_OUT_OF_MEMORY = -130,
    enumNvStatus_WAS_STILL_DRAWING = -131,
    enumNvStatus_FILE_NOT_FOUND = -132,
    enumNvStatus_TOO_MANY_UNIQUE_STATE_OBJECTS = -133,
    enumNvStatus_INVALID_CALL = -134,
    enumNvStatus_D3D10_1_LIBRARY_NOT_FOUND = -135,
    enumNvStatus_FUNCTION_NOT_FOUND = -136
};

enum NvThermalController
{
    enumNvThermalController_NONE = 0,
    enumNvThermalController_GPU_INTERNAL,
    enumNvThermalController_ADM1032,
    enumNvThermalController_MAX6649,
    enumNvThermalController_MAX1617,
    enumNvThermalController_LM99,
    enumNvThermalController_LM89,
    enumNvThermalController_LM64,
    enumNvThermalController_ADT7473,
    enumNvThermalController_SBMAX6649,
    enumNvThermalController_VBIOSEVT,
    enumNvThermalController_OS,
    enumNvThermalController_UNKNOWN = -1,
};

enum NvThermalTarget
{
    enumNvThermalTarget_NONE = 0,
    enumNvThermalTarget_GPU = 1,
    enumNvThermalTarget_MEMORY = 2,
    enumNvThermalTarget_POWER_SUPPLY = 4,
    enumNvThermalTarget_BOARD = 8,
    enumNvThermalTarget_ALL = 15,
    enumNvThermalTarget_UNKNOWN = -1
};

typedef struct _NvSensor
{
    NvThermalController Controller;
    unsigned int DefaultMinTemp;
    unsigned int DefaultMaxTemp;
    unsigned int CurrentTemp;
    NvThermalTarget Target;
}NvSensor;

typedef struct _NvGPUThermalSettings
{
    unsigned int Version;
    unsigned int Count;
    NvSensor Sensor[MAX_THERMAL_SENSORS_PER_GPU];
}NvGPUThermalSettings;

typedef struct _NvClocks
{
    unsigned int Version;
    unsigned int Clock[MAX_CLOCKS_PER_GPU];
}NvClocks;

typedef struct _NvPState
{
    bool Present;
    int Percentage;
}NvPState;

typedef struct _NvPStates
{
    unsigned int Version;
    unsigned int Flags;
    NvPState PStates[MAX_PSTATES_PER_GPU];
}NvPStates;

typedef struct _NvUsages
{
    unsigned int Version;
    unsigned int Usages[MAX_USAGES_PER_GPU];
}NvUsages;

typedef struct _NvCooler
{
    int Type;
    int Controller;
    int DefaultMin;
    int DefaultMax;
    int CurrentMin;
    int CurrentMax;
    int CurrentLevel;
    int DefaultPolicy;
    int CurrentPolicy;
    int Target;
    int ControlType;
    int Active;
}NvCooler;

typedef struct _NvGPUCoolerSettings
{
    unsigned int Version;
    unsigned int Count;
    NvCooler Coolers[MAX_COOLER_PER_GPU];
}NvGPUCoolerSettings;

typedef struct _NvLevel
{
    int Level;
    int Policy;
}NvLevel;

typedef struct _NvGPUCoolerLevels
{
    unsigned int Version;
    NvLevel Levels[MAX_COOLER_PER_GPU];
}NvGPUCoolerLevels;

typedef struct _NvMemoryInfo
{
    unsigned int Version;
    unsigned int Values[MAX_MEMORY_VALUES_PER_GPU];
}NvMemoryInfo;

typedef struct _NvDisplayDriverVersion
{
    unsigned int Version;
    unsigned int DriverVersion;
    unsigned int BldChangeListNum;
    char szBuildBranch[SHORT_STRING_MAX];
    char szAdapter[SHORT_STRING_MAX];
}NvDisplayDriverVersion;

typedef int NvPhysicalGpuHandle;
typedef int NvDisplayHandle;

// ��������
typedef void* (*nvapi_QueryInterfaceType)(unsigned int uiInterfaceID);
typedef NvStatus(*NvAPI_InitializeType)();
typedef NvStatus(*NvAPI_GPU_GetFullNameType)(const NvPhysicalGpuHandle gpuHandle, char *pszName);
typedef NvStatus(*NvAPI_GPU_GetThermalSettingsType)(const NvPhysicalGpuHandle gpuHandle, int sensorIndex, NvGPUThermalSettings *pnvGPUThermalSettings);
typedef NvStatus(*NvAPI_EnumNvidiaDisplayHandleType)(const int thisEnum, NvDisplayHandle *pDisplayHandle);
typedef NvStatus(*NvAPI_GetPhysicalGPUsFromDisplayType)(const NvDisplayHandle displayHandle, NvPhysicalGpuHandle *pGpuHandles, unsigned int *pGpuCount);
typedef NvStatus(*NvAPI_EnumPhysicalGPUsType)(NvPhysicalGpuHandle *pGpuHandles, int *pGpuCount);
typedef NvStatus(*NvAPI_GPU_GetTachReadingType)(const NvPhysicalGpuHandle gpuHandle, int *pnValue);
typedef NvStatus(*NvAPI_GPU_GetAllClocksType)(const NvPhysicalGpuHandle gpuHandle, NvClocks *pnvClocks);
typedef NvStatus(*NvAPI_GPU_GetPStatesType)(const NvPhysicalGpuHandle gpuHandle, NvPStates *pnvPStates);
typedef NvStatus(*NvAPI_GPU_GetUsagesType)(const NvPhysicalGpuHandle gpuHandle, NvUsages *pnvUsages);
typedef NvStatus(*NvAPI_GPU_GetCoolerSettingsType)(const NvPhysicalGpuHandle gpuHandle, int coolerIndex, NvGPUCoolerSettings *pnvGPUCoolerSettings);
typedef NvStatus(*NvAPI_GPU_SetCoolerLevelsType)(const NvPhysicalGpuHandle gpuHandle, int coolerIndex, NvGPUCoolerLevels *pnvGPUCoolerLevels);
typedef NvStatus(*NvAPI_GPU_GetMemoryInfoType)(const NvDisplayHandle displayHandle, NvMemoryInfo *pnvMemoryInfo);
typedef NvStatus(*NvAPI_GetDisplayDriverVersionType)(const NvDisplayHandle displayHandle, NvDisplayDriverVersion *pnvDisplayDriverVersion);
typedef NvStatus(*NvAPI_GetInterfaceVersionStringType)(char *pszVersion);
typedef NvStatus(*NvAPI_GPU_GetPCIIdentifiersType)(
    const NvPhysicalGpuHandle gpuHandle,
    unsigned int *puiDeviceId,
    unsigned int *puiSubSystemId,
    unsigned int *puiRevisionId,
    unsigned int *puiExtDeviceId
    );

//*****************************���������ڲ�ʹ�õĽṹ��************************************//
/*
* @brief GPU��Ϣ
*/
typedef struct _GPU_INFO
{
    NvPhysicalGpuHandle     nvGpuHandle;        // GPU���
    int                     nUsage;             // GPUռ����
}GPU_INFO;

/*
* @brief �Կ���Ϣ
*/
typedef struct _DISPLAY_CARD_INFO
{
    NvDisplayHandle nvDisplayHandle;            // �Կ����
    int             nGpuCount;                  // Gpu����
    DWORD           dwTotalMemory;              // ���Դ��С��KB��
    DWORD           dwFreeMemory;               // �����Դ��С��KB��

    GPU_INFO        sGpuInfo[MAX_GPU_NUM];      // GPU��Ϣ
}DISPLAY_CARD_INFO;
//*****************************************************************************************//

/*
* @brief �Կ������Ϣ�������ڵ����Ľṹ�壩
*/
typedef struct _DISPLAY_INFO
{
    int         nGpuCount;                  // Gpu����
    int         nGpuUsages[MAX_GPU_NUM];    // Gpuռ����
    DWORD       dwTotalMemory;              // ���Դ��С��KB��
    DWORD       dwFreeMemory;               // �����Դ��С��KB��
}DISPLAY_INFO;

class KGpuMon
{
public:
    KGpuMon(void);
    ~KGpuMon(void);

    BOOL Init();
    BOOL Unit();

    int GetDisplayCardCount();
    BOOL GetDisplayInfo(const int nCardIndex, DISPLAY_INFO *pDisplayInfo);
private:
    int     EnumDisplayCards();
    BOOL    GetGpuHandles(const NvDisplayHandle nvDisplayHandle, DISPLAY_CARD_INFO *pCardInfo);

    BOOL    GetDisplayCardGpuUsages(const NvDisplayHandle nvDisplayHandle, DISPLAY_CARD_INFO *pCardInfo);
    BOOL    GetDisplayCardMemoryInfo(const NvDisplayHandle nvDisplayHandle, DISPLAY_CARD_INFO *pCardInfo);
private:
    nvapi_QueryInterfaceType m_pfnNvapi_QueryInterface;
    NvAPI_InitializeType m_pfnNvAPI_Initialize;
    NvAPI_GPU_GetFullNameType m_pfnNvAPI_GPU_GetFullName;
    NvAPI_GPU_GetThermalSettingsType m_pfnNvAPI_GPU_GetThermalSettings;
    NvAPI_EnumNvidiaDisplayHandleType m_pfnNvAPI_EnumNvidiaDisplayHandle;
    NvAPI_GetPhysicalGPUsFromDisplayType m_pfnNvAPI_GetPhysicalGPUsFromDisplay;
    NvAPI_EnumPhysicalGPUsType m_pfnNvAPI_EnumPhysicalGPUs;
    NvAPI_GPU_GetTachReadingType m_pfnNvAPI_GPU_GetTachReading;
    NvAPI_GPU_GetAllClocksType m_pfnNvAPI_GPU_GetAllClocks;
    NvAPI_GPU_GetPStatesType m_pfnNvAPI_GPU_GetPStates;
    NvAPI_GPU_GetUsagesType m_pfnNvAPI_GPU_GetUsages;
    NvAPI_GPU_GetCoolerSettingsType m_pfnNvAPI_GPU_GetCoolerSettings;
    NvAPI_GPU_SetCoolerLevelsType m_pfnNvAPI_GPU_SetCoolerLevels;
    NvAPI_GPU_GetMemoryInfoType m_pfnNvAPI_GPU_GetMemoryInfo;
    NvAPI_GetDisplayDriverVersionType m_pfnNvAPI_GetDisplayDriverVersion;
    NvAPI_GetInterfaceVersionStringType m_pfnNvAPI_GetInterfaceVersionString;
    NvAPI_GPU_GetPCIIdentifiersType m_pfnNvAPI_GPU_GetPCIIdentifiers;


    int                 m_nDisplayCardCount;
    DISPLAY_CARD_INFO*  m_pDisplayCards;
    HMODULE             m_hNvApiDll;
};