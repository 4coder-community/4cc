///////////////////////////////////////////////////////////////////
//*-------------------------------------------------------------*//
//| Part of the Game Jolt API C++ Library (http://gamejolt.com) |//
//*-------------------------------------------------------------*//
//| Released under the zlib License                             |//
//| More information available in the readme file               |//
//*-------------------------------------------------------------*//
///////////////////////////////////////////////////////////////////
#pragma once
#ifndef _GJ_GUARD_DATAITEM_H_
#define _GJ_GUARD_DATAITEM_H_


// ****************************************************************
/*! Data store item class.\n
 *  http://gamejolt.com/api/doc/game/data-store/
 *  \brief Data Store Item Object
 *  \bug Data store items are not UTF-8 compatible (server-side),\n
 *       if you have problems with string data, use Base64 instead */
class gjDataItem final
{
private:
    std::string m_sKey;        //!< unique key of this item
    std::string m_sData;       //!< semi-cached data

    int m_iType;               //!< type of this item (0 = global, 1 = user)

    std::string m_sVerify;     //!< temporary verification data (helper)
    void* m_pTarget;           //!< last target for binary data (helper)

    gjAPI* m_pAPI;             //!< main interface access pointer


public:
    gjDataItem(const gjData& aDataItemData, const int& iType, gjAPI* pAPI)noexcept;

    /*! \name Set Data Request */
    //! @{
    /*! Set data of this data store item through an API request.\n
     *  May does nothing, if this is an user item and the same data was already sent.
     *  \pre    Login maybe required
     *  \note   \b -Now blocks, \b -Call uses non-blocking callbacks
     *  \param  sData Data to send
     *  \return **GJ_OK** on success\n
     *          **GJ_REQUEST_FAILED** if request was unsuccessful\n
     *          **GJ_REQUEST_CANCELED** if this is an user item and the same data was already sent\n
     *          **GJ_NOT_CONNECTED** if connection/login is missing\n
     *          (see #GJ_ERROR) */
                          inline int SetDataNow(const std::string& sData)                                    {return this->__SetData(sData, true, GJ_NETWORK_NULL_API(gjDataItemPtr));}
                          inline int SetDataCall(const std::string& sData)                                   {return this->__SetData(sData, false, GJ_NETWORK_NULL_API(gjDataItemPtr));}
    template <typename T> inline int SetDataCall(const std::string& sData, GJ_NETWORK_OUTPUT(gjDataItemPtr)) {return this->__SetData(sData, false, GJ_NETWORK_OUTPUT_FW);}
    //! @}

    /*! \name Set Data Request Base64 */
    //! @{
    /*! Like \link SetDataNow SetData\endlink\n
     *  Allows to send data in binary form.
     *  \pre    Login maybe required
     *  \note   \b -Now blocks, \b -Call uses non-blocking callbacks
     *  \param  pData Data in binary form to send
     *  \param  iSize Size of the data
     *  \return **GJ_OK** on success\n
     *          **GJ_REQUEST_FAILED** if request was unsuccessful\n
     *          **GJ_REQUEST_CANCELED** if this is an user item and the same data was already sent\n
     *          **GJ_NOT_CONNECTED** if connection/login is missing\n
     *          (see #GJ_ERROR) */
                          inline int SetDataBase64Now(void* pData, const size_t& iSize)                                    {return this->__SetDataBase64(pData, iSize, true, GJ_NETWORK_NULL_API(gjDataItemPtr));}
                          inline int SetDataBase64Call(void* pData, const size_t& iSize)                                   {return this->__SetDataBase64(pData, iSize, false, GJ_NETWORK_NULL_API(gjDataItemPtr));}
    template <typename T> inline int SetDataBase64Call(void* pData, const size_t& iSize, GJ_NETWORK_OUTPUT(gjDataItemPtr)) {return this->__SetDataBase64(pData, iSize, false, GJ_NETWORK_OUTPUT_FW);}
    //! @}

    /*! \name Get Data Request */
    //! @{
    /*! Get data of this data store item through an API request.
     *  \pre    Login maybe required
     *  \note   \b -Now blocks, \b -Call uses non-blocking callbacks
     *  \return **GJ_OK** on success\n
     *          **GJ_REQUEST_FAILED** if request was unsuccessful\n
     *          **GJ_NOT_CONNECTED** if connection/login is missing\n
     *          (see #GJ_ERROR) */
                          inline int GetDataNow(std::string* psOutput)           {if(!psOutput) return GJ_INVALID_INPUT; return this->__GetData(psOutput, GJ_NETWORK_NULL_API(std::string));}
    template <typename T> inline int GetDataCall(GJ_NETWORK_OUTPUT(std::string)) {return this->__GetData(NULL, GJ_NETWORK_OUTPUT_FW);}
    //! @}

    /*! \name Get Data Request Base64 */
    //! @{
    /*! Like \link GetDataNow GetData\endlink\n
     *  Allows to get data in binary form.
     *  \pre    Login maybe required
     *  \note   \b -Now blocks, \b -Call uses non-blocking callbacks
     *  \param   pTarget Pointer to the target
     *  \param   iSize   Size of the target
     *  \return **GJ_OK** on success\n
     *          **GJ_REQUEST_FAILED** if request was unsuccessful\n
     *          **GJ_NOT_CONNECTED** if connection/login is missing\n
     *          (see #GJ_ERROR) */
                          inline int GetDataBase64Now(void* pTarget, const size_t& iSize)                                {if(!pTarget || iSize <= 0) return GJ_INVALID_INPUT; return this->__GetDataBase64(pTarget, iSize, true,  GJ_NETWORK_NULL_API(gjVoidPtr));}
                          inline int GetDataBase64Call(void* pTarget, const size_t& iSize)                               {if(!pTarget || iSize <= 0) return GJ_INVALID_INPUT; return this->__GetDataBase64(pTarget, iSize, false, GJ_NETWORK_NULL_API(gjVoidPtr));}
    template <typename T> inline int GetDataBase64Call(void* pTarget, const size_t& iSize, GJ_NETWORK_OUTPUT(gjVoidPtr)) {if(!pTarget || iSize <= 0) return GJ_INVALID_INPUT; return this->__GetDataBase64(pTarget, iSize, false, GJ_NETWORK_OUTPUT_FW);}
    //! @}

    /*! \name Remove Request */
    //! @{
    /*! Clears/Removes the data store item through an API request.\n
     *  \pre    Login maybe required
     *  \note   \b -Now blocks, \b -Call uses non-blocking callbacks
     *  \return **GJ_OK** on success\n
     *          **GJ_REQUEST_FAILED** if request was unsuccessful\n
     *          **GJ_NOT_CONNECTED** if connection/login is missing\n
     *          (see #GJ_ERROR) */
                          inline int RemoveNow()                                  {return this->__Remove(true,  GJ_NETWORK_NULL_API(gjDataItemPtr));}
                          inline int RemoveCall()                                 {return this->__Remove(false, GJ_NETWORK_NULL_API(gjDataItemPtr));}
    template <typename T> inline int RemoveCall(GJ_NETWORK_OUTPUT(gjDataItemPtr)) {return this->__Remove(false, GJ_NETWORK_OUTPUT_FW);}
    //! @}

    /*! \name Get Attributes */
    //! @{
    inline const std::string& GetKey()const  {return m_sKey;}    //!< \copybrief m_sKey
    inline const int&         GetType()const {return m_iType;}   //!< \copybrief m_iType
    /*! */ //! @}


private:
    DISABLE_COPY(gjDataItem)

    /*! \name Superior Request Functions */
    //! @{
    template <typename T> int __SetData(const std::string& sData, const bool& bNow, GJ_NETWORK_OUTPUT(gjDataItemPtr));
    template <typename T> int __SetDataBase64(void* pData, const size_t& iSize, const bool& bNow, GJ_NETWORK_OUTPUT(gjDataItemPtr));
    template <typename T> int __GetData(std::string* psOutput, GJ_NETWORK_OUTPUT(std::string));
    template <typename T> int __GetDataBase64(void* pTarget, const size_t& iSize, const bool& bNow, GJ_NETWORK_OUTPUT(gjVoidPtr));
    template <typename T> int __Remove(const bool& bNow, GJ_NETWORK_OUTPUT(gjDataItemPtr));
    //! @}

    /*! \name Callback Functions */
    //! @{
    int __SetDataCallback(const std::string& sData, void* pAdd, gjDataItemPtr* ppOutput);
    int __GetDataCallback(const std::string& sData, void* pAdd, std::string* psOutput);
    int __GetDataBase64Callback(const std::string& sData, void* pAdd, gjVoidPtr* ppOutput);
    int __RemoveCallback(const std::string& sData, void* pAdd, gjDataItemPtr* ppOutput);
    //! @}
};


// ****************************************************************
/* set data of this data store item */
template <typename T> int gjDataItem::__SetData(const std::string& sData, const bool& bNow, GJ_NETWORK_OUTPUT(gjDataItemPtr))
{
    if(!m_pAPI->IsUserConnected() && m_iType) return GJ_NOT_CONNECTED;

    // cancel request if data was already sent
    if(sData == m_sData && m_iType) return GJ_REQUEST_CANCELED;

    // set verification data
    m_sVerify = sData;

    // access user or global data store item
    const std::string sUserData = m_iType ?
                                  "&username="   + m_pAPI->GetProcUserName()  +
                                  "&user_token=" + m_pAPI->GetProcUserToken() :
                                  "";

    // send set data store request
    std::string sResponse;
    if(m_pAPI->SendRequest("/data-store/set/"
                           "?game_id=" + m_pAPI->GetProcGameID()         +
                           "&key="     + gjAPI::UtilEscapeString(m_sKey) +
                           sUserData + "&POST" + sData, bNow ? &sResponse : NULL, this, &gjDataItem::__SetDataCallback, &m_sVerify, GJ_NETWORK_OUTPUT_FW)) return GJ_REQUEST_FAILED;

    if(bNow) return this->__SetDataCallback(sResponse, &m_sVerify, NULL);
    return GJ_OK;
}


// ****************************************************************
/* set Base64 data of this data store item */
template <typename T> int gjDataItem::__SetDataBase64(void* pData, const size_t& iSize, const bool& bNow, GJ_NETWORK_OUTPUT(gjDataItemPtr))
{
    if(!m_pAPI->IsUserConnected() && m_iType) return GJ_NOT_CONNECTED;
    if(!pData || iSize <= 0) return GJ_INVALID_INPUT;

    const size_t iNeed = base64_needed(iSize);

    // convert binary data to Base64 string
    char* pcBase64 = new char[iNeed];
    base64_encode((unsigned char*)pData, iSize, pcBase64, iNeed);

    // execute set data function with Base64 string
    const int iReturn = this->__SetData(pcBase64, bNow, GJ_NETWORK_OUTPUT_FW);
    SAFE_DELETE_ARRAY(pcBase64)

    return iReturn;
}


// ****************************************************************
/* get data of this data store item */
template <typename T> int gjDataItem::__GetData(std::string* psOutput, GJ_NETWORK_OUTPUT(std::string))
{
    if(!m_pAPI->IsUserConnected() && m_iType) return GJ_NOT_CONNECTED;

    const bool bNow = psOutput ? true : false;

    // check for cached data
    if(m_iType)
    {
        if(m_sData.length())
        {
            if(bNow) (*psOutput) = m_sData;
            else (pOutputObj->*OutputCallback)(m_sData, pOutputData);
            return GJ_OK;
        }
    }

    // access user or global data store item
    const std::string sUserData = m_iType ?
                                  "&username="   + m_pAPI->GetProcUserName()  +
                                  "&user_token=" + m_pAPI->GetProcUserToken() :
                                  "";

    // send get data store request
    std::string sResponse;
    if(m_pAPI->SendRequest("/data-store/"
                           "?game_id=" + m_pAPI->GetProcGameID()         +
                           "&key="     + gjAPI::UtilEscapeString(m_sKey) +
                           "&format="  + "dump"                          +
                           sUserData, bNow ? &sResponse : NULL, this, &gjDataItem::__GetDataCallback, NULL, GJ_NETWORK_OUTPUT_FW)) return GJ_REQUEST_FAILED;

    if(bNow) return this->__GetDataCallback(sResponse, NULL, psOutput);
    return GJ_OK;
}


// ****************************************************************
/* get Base64 data of this data store item */
template <typename T> int gjDataItem::__GetDataBase64(void* pTarget, const size_t& iSize, const bool& bNow, GJ_NETWORK_OUTPUT(gjVoidPtr))
{
    if(!m_pAPI->IsUserConnected() && m_iType) return GJ_NOT_CONNECTED;
    if(!pTarget || iSize <= 0) return GJ_INVALID_INPUT;

    // save last target
    m_pTarget = pTarget;

    // check for cached data
    if(m_iType)
    {
        if(m_sData.length())
        {
            // convert Base64 string to binary data
            base64_decode(m_sData.c_str(), (unsigned char*)m_pTarget, iSize);
            if(!bNow) (pOutputObj->*OutputCallback)(m_pTarget, pOutputData);
            return GJ_OK;
        }
    }

    // access user or global data store item
    const std::string sUserData = m_iType ?
                                  "&username="   + m_pAPI->GetProcUserName()  +
                                  "&user_token=" + m_pAPI->GetProcUserToken() :
                                  "";

    // send get data store request
    std::string sResponse;
    if(m_pAPI->SendRequest("/data-store/"
                           "?game_id=" + m_pAPI->GetProcGameID()         +
                           "&key="     + gjAPI::UtilEscapeString(m_sKey) +
                           "&format="  + "dump"                          +
                           sUserData, bNow ? &sResponse : NULL, this, &gjDataItem::__GetDataBase64Callback, I_TO_P(iSize), GJ_NETWORK_OUTPUT_FW)) return GJ_REQUEST_FAILED;

    if(bNow) return this->__GetDataBase64Callback(sResponse, I_TO_P(iSize), NULL);
    return GJ_OK;
}


// ****************************************************************
/* clear/remove this data store item */
template <typename T> int gjDataItem::__Remove(const bool& bNow, GJ_NETWORK_OUTPUT(gjDataItemPtr))
{
    if(!m_pAPI->IsUserConnected() && m_iType) return GJ_NOT_CONNECTED;

    // access user or global data store item
    const std::string sUserData = m_iType ?
                                  "&username="   + m_pAPI->GetProcUserName()  +
                                  "&user_token=" + m_pAPI->GetProcUserToken() :
                                  "";

    // send remove data store request
    std::string sResponse;
    if(m_pAPI->SendRequest("/data-store/remove/"
                           "?game_id=" + m_pAPI->GetProcGameID()         +
                           "&key="     + gjAPI::UtilEscapeString(m_sKey) +
                           sUserData, bNow ? &sResponse : NULL, this, &gjDataItem::__RemoveCallback, NULL, GJ_NETWORK_OUTPUT_FW)) return GJ_REQUEST_FAILED;

    if(bNow) return this->__RemoveCallback(sResponse, NULL, NULL);
    return GJ_OK;
}


#endif /* _GJ_GUARD_DATAITEM_H_ */