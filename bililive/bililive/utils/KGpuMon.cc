#include "KGpuMon.h"

#include <algorithm>

KGpuMon::KGpuMon(void)
{
    m_hNvApiDll = NULL;
    m_nDisplayCardCount = 0;
    m_pDisplayCards = NULL;
}

KGpuMon::~KGpuMon(void)
{
    Unit();
}

BOOL KGpuMon::Init()
{
    BOOL bResult = FALSE;
    BOOL bRetCode = FALSE;

    int nIndex = 0;
    int nResult = 0;

    m_hNvApiDll = LoadLibraryW(L"nvapi.dll");
    if (m_hNvApiDll)
    {
        m_pfnNvapi_QueryInterface = (nvapi_QueryInterfaceType)GetProcAddress(m_hNvApiDll, "nvapi_QueryInterface");
        if (m_pfnNvapi_QueryInterface)
        {
            m_pfnNvAPI_Initialize = (NvAPI_InitializeType)m_pfnNvapi_QueryInterface(ID_NvAPI_Initialize);
            m_pfnNvAPI_GPU_GetFullName = (NvAPI_GPU_GetFullNameType)m_pfnNvapi_QueryInterface(ID_NvAPI_GPU_GetFullName);
            m_pfnNvAPI_GPU_GetThermalSettings = (NvAPI_GPU_GetThermalSettingsType)m_pfnNvapi_QueryInterface(ID_NvAPI_GPU_GetThermalSettings);
            m_pfnNvAPI_EnumNvidiaDisplayHandle = (NvAPI_EnumNvidiaDisplayHandleType)m_pfnNvapi_QueryInterface(ID_NvAPI_EnumNvidiaDisplayHandle);
            m_pfnNvAPI_GetPhysicalGPUsFromDisplay = (NvAPI_GetPhysicalGPUsFromDisplayType)m_pfnNvapi_QueryInterface(ID_NvAPI_GetPhysicalGPUsFromDisplay);
            m_pfnNvAPI_EnumPhysicalGPUs = (NvAPI_EnumPhysicalGPUsType)m_pfnNvapi_QueryInterface(ID_NvAPI_EnumPhysicalGPUs);
            m_pfnNvAPI_GPU_GetTachReading = (NvAPI_GPU_GetTachReadingType)m_pfnNvapi_QueryInterface(ID_NvAPI_GPU_GetTachReading);
            m_pfnNvAPI_GPU_GetAllClocks = (NvAPI_GPU_GetAllClocksType)m_pfnNvapi_QueryInterface(ID_NvAPI_GPU_GetAllClocks);
            m_pfnNvAPI_GPU_GetPStates = (NvAPI_GPU_GetPStatesType)m_pfnNvapi_QueryInterface(ID_NvAPI_GPU_GetPStates);
            m_pfnNvAPI_GPU_GetUsages = (NvAPI_GPU_GetUsagesType)m_pfnNvapi_QueryInterface(ID_NvAPI_GPU_GetUsages);
            m_pfnNvAPI_GPU_GetCoolerSettings = (NvAPI_GPU_GetCoolerSettingsType)m_pfnNvapi_QueryInterface(ID_NvAPI_GPU_GetCoolerSettings);
            m_pfnNvAPI_GPU_SetCoolerLevels = (NvAPI_GPU_SetCoolerLevelsType)m_pfnNvapi_QueryInterface(ID_NvAPI_GPU_SetCoolerLevels);
            m_pfnNvAPI_GPU_GetMemoryInfo = (NvAPI_GPU_GetMemoryInfoType)m_pfnNvapi_QueryInterface(ID_NvAPI_GPU_GetMemoryInfo);
            m_pfnNvAPI_GetDisplayDriverVersion = (NvAPI_GetDisplayDriverVersionType)m_pfnNvapi_QueryInterface(ID_NvAPI_GetDisplayDriverVersion);
            m_pfnNvAPI_GetInterfaceVersionString = (NvAPI_GetInterfaceVersionStringType)m_pfnNvapi_QueryInterface(ID_NvAPI_GetInterfaceVersionString);
            m_pfnNvAPI_GPU_GetPCIIdentifiers = (NvAPI_GPU_GetPCIIdentifiersType)m_pfnNvapi_QueryInterface(ID_NvAPI_GPU_GetPCIIdentifiers);

            if (m_pfnNvAPI_Initialize)
            {
                nResult = m_pfnNvAPI_Initialize();
                if (enumNvStatus_OK == nResult)
                {
                    m_pDisplayCards = new DISPLAY_CARD_INFO[MAX_DISPLAY_CARDS];
                    ZeroMemory(m_pDisplayCards, MAX_DISPLAY_CARDS * sizeof(DISPLAY_CARD_INFO));

                    // 获取显卡个数
                    nResult = EnumDisplayCards();
                    MY_PROCESS_ERROR(nResult > 0);

                    // 获取每块显卡的GPU个数
                    for (nIndex = 0; nIndex < m_nDisplayCardCount; ++nIndex)
                    {
                        bRetCode = GetGpuHandles(m_pDisplayCards[nIndex].nvDisplayHandle, &m_pDisplayCards[nIndex]);
                        MY_PROCESS_ERROR(bRetCode);
                    }

                    bResult = TRUE;
                }
            }
        }
    }
Exit0:
    return bResult;
}

BOOL KGpuMon::Unit()
{
    m_nDisplayCardCount = 0;

    if (m_pDisplayCards)
    {
        delete[]m_pDisplayCards;
        m_pDisplayCards = NULL;
    }

    if (m_hNvApiDll)
    {
        FreeLibrary(m_hNvApiDll);
        m_hNvApiDll = NULL;
    }

    return TRUE;
}

BOOL KGpuMon::GetDisplayInfo(const int nCardIndex, DISPLAY_INFO *pDisplayInfo)
{
    BOOL bResult = FALSE;

    int nIndex = 0;

    if (nCardIndex < m_nDisplayCardCount)
    {
        bResult = GetDisplayCardGpuUsages(m_pDisplayCards[nCardIndex].nvDisplayHandle, &m_pDisplayCards[nCardIndex]);
        MY_PROCESS_ERROR(bResult);

        pDisplayInfo->nGpuCount = m_pDisplayCards[nCardIndex].nGpuCount;
        for (nIndex = 0; nIndex < pDisplayInfo->nGpuCount; ++nIndex)
        {
            pDisplayInfo->nGpuUsages[nIndex] = m_pDisplayCards[nCardIndex].sGpuInfo[nIndex].nUsage;
        }

        bResult = GetDisplayCardMemoryInfo(m_pDisplayCards[nCardIndex].nvDisplayHandle, &m_pDisplayCards[nCardIndex]);
        MY_PROCESS_ERROR(bResult);

        pDisplayInfo->dwTotalMemory = m_pDisplayCards[nCardIndex].dwTotalMemory;
        pDisplayInfo->dwFreeMemory = m_pDisplayCards[nCardIndex].dwFreeMemory;
    }
Exit0:
    return bResult;
}

int KGpuMon::GetDisplayCardCount()
{
    return m_nDisplayCardCount;
}

int KGpuMon::EnumDisplayCards()
{
    NvStatus nvResult;
    NvDisplayHandle nvDisplayCardHandle;

    int nIndex = 0;

    m_nDisplayCardCount = 0;
    if (m_pfnNvAPI_EnumNvidiaDisplayHandle)
    {
        for (nIndex = 0; nIndex < MAX_DISPLAY_CARDS; ++nIndex)
        {
            nvResult = m_pfnNvAPI_EnumNvidiaDisplayHandle(nIndex, &nvDisplayCardHandle);
            if (enumNvStatus_OK == nvResult)
            {
                m_pDisplayCards[m_nDisplayCardCount].nvDisplayHandle = nvDisplayCardHandle;
                ++m_nDisplayCardCount;
            }
        }
    }

    return m_nDisplayCardCount;
}

BOOL KGpuMon::GetGpuHandles(const NvDisplayHandle nvDisplayHandle, DISPLAY_CARD_INFO *pCardInfo)
{
    BOOL bResult = FALSE;

    NvStatus nvStatus;
    NvPhysicalGpuHandle *pnvHandles = NULL;

    int nIndex = 0;
    unsigned int uiGpuCount = 0;

    if (m_pfnNvAPI_GetPhysicalGPUsFromDisplay)
    {
        pnvHandles = new NvPhysicalGpuHandle[MAX_PHYSICAL_GPUS];
        nvStatus = m_pfnNvAPI_GetPhysicalGPUsFromDisplay(nvDisplayHandle, pnvHandles, &uiGpuCount);
        if (enumNvStatus_OK == nvStatus)
        {
            pCardInfo->nGpuCount = std::min<int>(uiGpuCount, MAX_GPU_NUM);
            for (nIndex = 0; nIndex < pCardInfo->nGpuCount; ++nIndex)
            {
                pCardInfo->sGpuInfo[nIndex].nvGpuHandle = pnvHandles[nIndex];
            }

            bResult = TRUE;
        }

        delete[]pnvHandles;
        pnvHandles = NULL;
    }

    return bResult;
}

BOOL KGpuMon::GetDisplayCardGpuUsages(const NvDisplayHandle nvDisplayHandle, DISPLAY_CARD_INFO *pCardInfo)
{
    BOOL bResult = FALSE;

    int nIndex = 0;

    NvStatus nvStatus = enumNvStatus_ERROR;
    NvUsages *pnvUsages = NULL;

    if (m_pfnNvAPI_GPU_GetUsages)
    {
        pnvUsages = new NvUsages;
        pnvUsages->Version = GPU_USAGES_VER;
        for (nIndex = 0; nIndex < pCardInfo->nGpuCount; ++nIndex)
        {
            nvStatus = m_pfnNvAPI_GPU_GetUsages(pCardInfo->sGpuInfo[nIndex].nvGpuHandle, pnvUsages);
            if (enumNvStatus_OK == nvStatus)
            {
                pCardInfo->sGpuInfo[nIndex].nUsage = pnvUsages->Usages[2];
            }
        }

        delete pnvUsages;
        pnvUsages = NULL;

        bResult = (enumNvStatus_OK == nvStatus) ? TRUE : FALSE;
    }

    return bResult;
}

BOOL KGpuMon::GetDisplayCardMemoryInfo(const NvDisplayHandle nvDisplayHandle, DISPLAY_CARD_INFO *pCardInfo)
{
    BOOL bResult = FALSE;

    int nIndex = 0;

    NvStatus nvStatus = enumNvStatus_ERROR;
    NvUsages *pnvUsages = NULL;
    NvMemoryInfo sMemoryInfo;

    if (m_pfnNvAPI_GPU_GetMemoryInfo)
    {
        sMemoryInfo.Version = GPU_MEMORY_INFO_VER;
        nvStatus = m_pfnNvAPI_GPU_GetMemoryInfo(nvDisplayHandle, &sMemoryInfo);
        if (enumNvStatus_OK == nvStatus)
        {
            pCardInfo->dwTotalMemory = (DWORD)(sMemoryInfo.Values[0]);
            pCardInfo->dwFreeMemory = (DWORD)(sMemoryInfo.Values[4]);

            bResult = TRUE;
        }
    }

    return bResult;
}