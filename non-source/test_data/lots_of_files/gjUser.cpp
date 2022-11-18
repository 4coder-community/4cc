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
gjUser::gjUser(const gjData& aUserData, gjAPI* pAPI)noexcept
: m_iID                   (atoi(SAFE_MAP_GET(aUserData, "id").c_str()))
, m_sName                 (SAFE_MAP_GET(aUserData, "username"))
, m_sType                 (SAFE_MAP_GET(aUserData, "type"))
, m_sAvatarURL            (SAFE_MAP_GET(aUserData, "avatar_url"))
, m_sSignedUp             (SAFE_MAP_GET(aUserData, "signed_up"))
, m_sLastLoggedIn         (SAFE_MAP_GET(aUserData, "last_logged_in"))
, m_sStatus               (SAFE_MAP_GET(aUserData, "status"))
, m_sDeveloperName        (SAFE_MAP_GET(aUserData, "developer_name"))
, m_sDeveloperWebsite     (SAFE_MAP_GET(aUserData, "developer_website"))
, m_sDeveloperDescription (SAFE_MAP_GET(aUserData, "developer_description"))
, m_pAPI                  (pAPI)
{
}

gjUser::gjUser(const gjUser& that)noexcept
: m_iID                   (that.m_iID)
, m_sName                 (that.m_sName)
, m_sType                 (that.m_sType)
, m_sAvatarURL            (that.m_sAvatarURL)
, m_sSignedUp             (that.m_sSignedUp)
, m_sLastLoggedIn         (that.m_sLastLoggedIn)
, m_sStatus               (that.m_sStatus)
, m_sDeveloperName        (that.m_sDeveloperName)
, m_sDeveloperWebsite     (that.m_sDeveloperWebsite)
, m_sDeveloperDescription (that.m_sDeveloperDescription)
, m_pAPI                  (that.m_pAPI)
{
}


// ****************************************************************
/* assignment operator */
gjUser& gjUser::operator = (const gjUser& that)noexcept
{
    m_iID                   = that.m_iID;
    m_sName                 = that.m_sName;
    m_sType                 = that.m_sType;
    m_sAvatarURL            = that.m_sAvatarURL;
    m_sSignedUp             = that.m_sSignedUp;
    m_sLastLoggedIn         = that.m_sLastLoggedIn;
    m_sStatus               = that.m_sStatus;
    m_sDeveloperName        = that.m_sDeveloperName;
    m_sDeveloperWebsite     = that.m_sDeveloperWebsite;
    m_sDeveloperDescription = that.m_sDeveloperDescription;
    m_pAPI                  = that.m_pAPI;
    return *this;
}


// ****************************************************************
/* callback for updating the data */
int gjUser::__UpdateDataCallback(const std::string& sData, void* pAdd, gjUserPtr* pOutput)
{
    // parse output
    gjDataList aaReturn;
    if(m_pAPI->ParseRequestKeypair(sData, &aaReturn) != GJ_OK)
    {
        gjAPI::ErrorLogAdd("API Error: could not parse user");
        return GJ_REQUEST_FAILED;
    }

    // copy new user over old user
    gjUser NewUser(aaReturn[0], m_pAPI);
    *this = NewUser;

    if(pOutput) (*pOutput) = this;
    return GJ_OK;
}