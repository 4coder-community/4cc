///////////////////////////////////////////////////////////////////
//*-------------------------------------------------------------*//
//| Part of the Game Jolt API C++ Library (http://gamejolt.com) |//
//*-------------------------------------------------------------*//
//| Released under the zlib License                             |//
//| More information available in the readme file               |//
//*-------------------------------------------------------------*//
///////////////////////////////////////////////////////////////////
#include "gjAPI.h"


// ****************************************************************
/* constructor */
gjDataItem::gjDataItem(const gjData& aDataItemData, const int& iType, gjAPI* pAPI)noexcept
: m_sKey     (SAFE_MAP_GET(aDataItemData, "key"))
, m_sData    ("")
, m_iType    (iType)
, m_sVerify  ("")
, m_pTarget  (NULL)
, m_pAPI     (pAPI)
{
}


// ****************************************************************
/* callback for successfully setting data store item (Base64) data */
int gjDataItem::__SetDataCallback(const std::string& sData, void* pAdd, gjDataItemPtr* ppOutput)
{
    // parse output (only check status)
    gjDataList aaReturn;
    if(m_pAPI->ParseRequestKeypair(sData, &aaReturn) != GJ_OK)
    {
        gjAPI::ErrorLogAdd("API Error: could not parse data store item");
        return GJ_REQUEST_FAILED;
    }

    if(pAdd)
    {
        // use verification data
        std::string* psAdd = (std::string*)pAdd;
        m_sData            = (*psAdd);
        (*psAdd)           = "";
    }

    if(ppOutput) (*ppOutput) = this;
    return GJ_OK;
}


// ****************************************************************
/* callback for successfully getting data store item data */
int gjDataItem::__GetDataCallback(const std::string& sData, void* pAdd, std::string* psOutput)
{
    // parse output
    std::string sReturn;
    if(m_pAPI->ParseRequestDump(sData, &sReturn) != GJ_OK)
    {
        gjAPI::ErrorLogAdd("API Error: could not parse data store item");
        return GJ_REQUEST_FAILED;
    }

    // save data
    m_sData = sReturn;

    if(psOutput) (*psOutput) = m_sData;
    return GJ_OK;
}


// ****************************************************************
/* callback for successfully getting data store item Base64 data */
int gjDataItem::__GetDataBase64Callback(const std::string& sData, void* pAdd, gjVoidPtr* ppOutput)
{
    // parse output and save data
    std::string sReturn;
    const int iError = this->__GetDataCallback(sData, NULL, &sReturn);
    if(iError) return iError;

    // convert Base64 string to binary data
    base64_decode(sReturn.c_str(), (unsigned char*)m_pTarget, P_TO_I(pAdd));

    if(ppOutput) (*ppOutput) = (void*)m_pTarget;
    return GJ_OK;
}


// ****************************************************************
/* callback for successfully clearing/removing the data store item  */
int gjDataItem::__RemoveCallback(const std::string& sData, void* pAdd, gjDataItemPtr* ppOutput)
{
    // parse output (only check status)
    gjDataList aaReturn;
    if(m_pAPI->ParseRequestKeypair(sData, &aaReturn) != GJ_OK)
    {
        gjAPI::ErrorLogAdd("API Error: could not parse data store item");
        return GJ_REQUEST_FAILED;
    }

    if(ppOutput) (*ppOutput) = this;
    return GJ_OK;
}