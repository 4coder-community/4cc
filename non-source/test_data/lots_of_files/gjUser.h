///////////////////////////////////////////////////////////////////
//*-------------------------------------------------------------*//
//| Part of the Game Jolt API C++ Library (http://gamejolt.com) |//
//*-------------------------------------------------------------*//
//| Released under the zlib License                             |//
//| More information available in the readme file               |//
//*-------------------------------------------------------------*//
///////////////////////////////////////////////////////////////////
#pragma once
#ifndef _GJ_GUARD_USER_H_
#define _GJ_GUARD_USER_H_


// ****************************************************************
/*! User object class.\n
 *  http://gamejolt.com/api/doc/game/users/
 *  \brief User Object */
class gjUser final
{
private:
    int m_iID;                             //!< ID of the user (-1 = guest)
    std::string m_sName;                   //!< display name of the user

    std::string m_sType;                   //!< type of the user ("Guest", "User", "Developer", "Moderator", "Administrator")

    std::string m_sAvatarURL;              //!< URL of the avatar

    std::string m_sSignedUp;               //!< time string when the user signed up (e.g. 4 weeks ago)
    std::string m_sLastLoggedIn;           //!< time string of the last login (e.g. 4 weeks ago)
    std::string m_sStatus;                 //!< status of the user ("Active", "Banned")

    std::string m_sDeveloperName;          //!< name of a developer user
    std::string m_sDeveloperWebsite;       //!< website of a developer user
    std::string m_sDeveloperDescription;   //!< description of a developer user

    gjAPI* m_pAPI;                         //!< main interface access pointer


public:
    gjUser(const gjData& aUserData, gjAPI* pAPI)noexcept;

    /*! \name Update Data Request */
    //! @{
    /*! Update data of this user through an API request.
     *  \note   \b -Now blocks, \b -Call uses non-blocking callbacks
     *  \return **GJ_OK** on success\n
     *          **GJ_REQUEST_FAILED** if request was unsuccessful\n
     *          **GJ_INVALID_CALL** if this is a guest users\n
     *          (see #GJ_ERROR) */
                          inline int UpdateDataNow()                              {return this->__UpdateData(true, GJ_NETWORK_NULL_API(gjUserPtr));}
                          inline int UpdateDataCall()                             {return this->__UpdateData(false, GJ_NETWORK_NULL_API(gjUserPtr));}
    template <typename T> inline int UpdateDataCall(GJ_NETWORK_OUTPUT(gjUserPtr)) {return this->__UpdateData(false, GJ_NETWORK_OUTPUT_FW);}
    //! @}

    /*! \name Download Avatar */
    //! @{
    /*! Download the avatar of this user.\n
     *  Receive the cached local path of the file.
     *  \note   \b -Now blocks, \b -Call uses non-blocking callbacks
     *  \param  sToFolder Relative path of the download target folder
     *  \return **GJ_OK** on success\n
     *          **GJ_REQUEST_FAILED** if request was unsuccessful\n
     *          **GJ_INVALID_INPUT** if target folder string is empty\n
     *          (see #GJ_ERROR) */
                          inline int DownloadAvatarNow(const std::string& sToFolder, std::string* psOutput)           {return m_pAPI->InterFile()->DownloadFileNow(this->GetAvatarURL(), sToFolder, m_sName + GJ_API_AVATAR_FORMAT, psOutput);}
    template <typename T> inline int DownloadAvatarCall(const std::string& sToFolder, GJ_NETWORK_OUTPUT(std::string)) {return m_pAPI->InterFile()->DownloadFileCall(this->GetAvatarURL(), sToFolder, m_sName + GJ_API_AVATAR_FORMAT, GJ_NETWORK_OUTPUT_FW);}
    //! @}

    /*! \name Get Attributes */
    //! @{
    inline const int&         GetID()const                   {return m_iID;}                     //!< \copybrief m_iID
    inline const std::string& GetName()const                 {return m_sName;}                   //!< \copybrief m_sName
    inline const std::string& GetType()const                 {return m_sType;}                   //!< \copybrief m_sType
    inline const std::string& GetAvatarURL()const            {return m_sAvatarURL;}              //!< \copybrief m_sAvatarURL
    inline const std::string& GetSignedUp()const             {return m_sSignedUp;}               //!< \copybrief m_sSignedUp
    inline const std::string& GetLastLoggedIn()const         {return m_sLastLoggedIn;}           //!< \copybrief m_sLastLoggedIn
    inline const std::string& GetStatus()const               {return m_sStatus;}                 //!< \copybrief m_sStatus
    inline const std::string& GetDeveloperName()const        {return m_sDeveloperName;}          //!< \copybrief m_sDeveloperName
    inline const std::string& GetDeveloperWebsite()const     {return m_sDeveloperWebsite;}       //!< \copybrief m_sDeveloperWebsite
    inline const std::string& GetDeveloperDescription()const {return m_sDeveloperDescription;}   //!< \copybrief m_sDeveloperDescription
    /*! */ //! @}


private:
    /*! \name Hide Copy */
    //! @{
    gjUser(const gjUser& that)noexcept;
    gjUser& operator = (const gjUser& that)noexcept;
    friend class gjAPI;
    //! @}

    /*! \name Superior Request Functions */
    //! @{
    template <typename T> int __UpdateData(const bool& bNow, GJ_NETWORK_OUTPUT(gjUserPtr));
    //! @}

    /*! \name Callback Functions */
    //! @{
    int __UpdateDataCallback(const std::string& sData, void* pAdd, gjUserPtr* pOutput);
    //! @}
};


// ****************************************************************
/* update data of this user */
template <typename T> int gjUser::__UpdateData(const bool& bNow, GJ_NETWORK_OUTPUT(gjUserPtr))
{
    if(m_iID <= 0) return GJ_INVALID_CALL;

    // send get user request
    std::string sResponse;
    if(m_pAPI->SendRequest("/users/"
                           "?game_id=" + m_pAPI->GetProcGameID() +
                           "&user_id=" + gjAPI::UtilIntToString(m_iID),
                           bNow ? &sResponse : NULL, this, &gjUser::__UpdateDataCallback, NULL, GJ_NETWORK_OUTPUT_FW)) return GJ_REQUEST_FAILED;

    if(bNow) return this->__UpdateDataCallback(sResponse, NULL, NULL);
    return GJ_OK;
}


#endif /* _GJ_GUARD_USER_H_ */