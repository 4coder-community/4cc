///////////////////////////////////////////////////////////////////
//*-------------------------------------------------------------*//
//| Part of the Game Jolt API C++ Library (http://gamejolt.com) |//
//*-------------------------------------------------------------*//
//| Released under the zlib License                             |//
//| More information available in the readme file               |//
//*-------------------------------------------------------------*//
///////////////////////////////////////////////////////////////////
#include "gjAPI.h"

#include <sstream>
#include <iostream>
#include <algorithm>

std::vector<std::string> gjAPI::s_asLog;


// ****************************************************************
/* constructor */
gjAPI::gjInterUser::gjInterUser(gjAPI* pAPI, gjNetwork* pNetwork)noexcept
: m_pAPI     (pAPI)
, m_pNetwork (pNetwork)
{
    // create NULL user for secure object handling
    gjData pNullData;
    pNullData["id"]         = "0";
    pNullData["username"]   = "NOT FOUND";
    pNullData["type"]       = "Guest";
    pNullData["avatar_url"] = GJ_API_AVATAR_DEFAULT;
    m_apUser[0] = new gjUser(pNullData, m_pAPI);

    // create guest user for secure object handling
    gjData pGuestData;
    pGuestData["id"]         = "-1";
    pGuestData["username"]   = "Guest";
    pGuestData["type"]       = "Guest";
    pGuestData["avatar_url"] = GJ_API_AVATAR_DEFAULT;
    m_apUser[-1] = new gjUser(pGuestData, m_pAPI);
}


// ****************************************************************
/* destructor */
gjAPI::gjInterUser::~gjInterUser()
{
    // delete all users
    FOR_EACH(it, m_apUser)
        SAFE_DELETE(it->second)

    // clear container
    m_apUser.clear();
}


// ****************************************************************
/* access user objects directly (may block) */
gjUser* gjAPI::gjInterUser::GetUser(const int& iID)
{
    gjUserPtr pOutput;

    if(this->__CheckCache(iID, &pOutput) == GJ_OK) return pOutput;
    if(this->FetchUserNow(iID, &pOutput) == GJ_OK) return pOutput;

    return m_apUser[0];
}

gjUser* gjAPI::gjInterUser::GetUser(const std::string& sName)
{
    gjUserPtr pOutput;

    if(this->__CheckCache(sName, &pOutput) == GJ_OK) return pOutput;
    if(this->FetchUserNow(sName, &pOutput) == GJ_OK) return pOutput;

    return m_apUser[0];
}


// ****************************************************************
/* access main user object directly (may block) */
gjUser* gjAPI::gjInterUser::GetMainUser()
{
    if(!m_pAPI->IsUserConnected()) return m_apUser[0];
    return this->GetUser(m_pAPI->GetUserName());
}


// ****************************************************************
/* delete all cached user objects */
void gjAPI::gjInterUser::ClearCache()
{
    // save NULL user and guest user
    gjUser* pNull  = m_apUser[0];  m_apUser.erase(0);
    gjUser* pGuest = m_apUser[-1]; m_apUser.erase(-1);

    // delete users
    FOR_EACH(it, m_apUser)
        SAFE_DELETE(it->second)

    // clear container
    m_apUser.clear();
    m_apUser[0]  = pNull;
    m_apUser[-1] = pGuest;
}


// ****************************************************************
/* check for cached user objects */
int gjAPI::gjInterUser::__CheckCache(const int& iID, gjUserPtr* ppOutput)
{
    // retrieve cached user
    if(m_apUser.count(iID))
    {
        if(ppOutput) (*ppOutput) = m_apUser[iID];
        return GJ_OK;
    }
    return GJ_NO_DATA_FOUND;
}

int gjAPI::gjInterUser::__CheckCache(const std::string& sName, gjUserPtr* ppOutput)
{
    // retrieve cached user
    FOR_EACH(it, m_apUser)
    {
        if(it->second->GetName() == sName)
        {
            if(ppOutput) (*ppOutput) = it->second;
            return GJ_OK;
        }
    }
    return GJ_NO_DATA_FOUND;
}


// ****************************************************************
/* process user data and cache user objects */
int gjAPI::gjInterUser::__Process(const std::string& sData, void* pAdd, gjUserPtr* ppOutput)
{
    // parse output
    gjDataList aaReturn;
    if(m_pAPI->ParseRequestKeypair(sData, &aaReturn) != GJ_OK)
    {
        gjAPI::ErrorLogAdd("API Error: could not parse user");
        if(ppOutput) (*ppOutput) = m_apUser[0];
        return GJ_REQUEST_FAILED;
    }

    // create and cache user object
    gjUser* pNewUser = new gjUser(aaReturn[0], m_pAPI);
    const int iID = pNewUser->GetID();

    if(m_apUser.count(iID))
    {
        SAFE_DELETE(pNewUser)
        pNewUser = m_apUser[iID];
    }
    else m_apUser[iID] = pNewUser;

    if(ppOutput) (*ppOutput) = pNewUser;
    return pNewUser ? GJ_OK : GJ_NO_DATA_FOUND;
}


// ****************************************************************
/* constructor */
gjAPI::gjInterTrophy::gjInterTrophy(gjAPI* pAPI, gjNetwork* pNetwork)noexcept
: m_iCache   (0)
, m_pAPI     (pAPI)
, m_pNetwork (pNetwork)
{
    // create NULL trophy for secure object handling
    gjData pNullData;
    pNullData["id"]         = "0";
    pNullData["title"]      = "NOT FOUND";
    pNullData["difficulty"] = "Bronze";
    pNullData["image_url"]  = GJ_API_TROPHY_DEFAULT_1;
    m_apTrophy[0] = new gjTrophy(pNullData, m_pAPI);

    // reserve some memory
    m_aiSort.reserve(GJ_API_RESERVE_TROPHY);
    m_aiSecret.reserve(GJ_API_RESERVE_TROPHY);
    m_aiHidden.reserve(GJ_API_RESERVE_TROPHY);

    // retrieve offline-cached trophy data
    this->__LoadOffCache();
}


// ****************************************************************
/* destructor */
gjAPI::gjInterTrophy::~gjInterTrophy()
{
    // delete all trophies
    FOR_EACH(it, m_apTrophy)
        SAFE_DELETE(it->second)

    // clear containers
    m_apTrophy.clear();
    m_aiSort.clear();
    m_aiSecret.clear();
    m_aiHidden.clear();
}


// ****************************************************************
/* access trophy objects directly (may block) */
gjTrophy* gjAPI::gjInterTrophy::GetTrophy(const int& iID)
{
    if(!m_pAPI->IsUserConnected() && m_iCache == 0) return m_apTrophy[0];
    if(m_apTrophy.size() <= 1)
    {
        // wait for prefetching
        if(GJ_API_PREFETCH) m_pNetwork->Wait(2);
        if(m_apTrophy.size() <= 1)
        {
            gjTrophyList apOutput;
            this->FetchTrophiesNow(0, &apOutput);
        }
    }
    return m_apTrophy.count(iID) ? m_apTrophy[iID] : m_apTrophy[0];
}


// ****************************************************************
/* delete all cached trophy objects */
void gjAPI::gjInterTrophy::ClearCache(const bool& bFull)
{
    const bool bRemoveAll = bFull || !GJ_API_OFFCACHE_TROPHY;

    if(bRemoveAll)
    {
        // save NULL trophy
        gjTrophy* pNull = m_apTrophy[0];
        m_apTrophy.erase(0);

        // delete trophies
        FOR_EACH(it, m_apTrophy)
            SAFE_DELETE(it->second)

        // clear container
        m_apTrophy.clear();
        m_apTrophy[0] = pNull;
    }

    // set cache status
    m_iCache = bRemoveAll ? 0 : 1;
}


// ****************************************************************
/* define layout of the returned trophy list */
void gjAPI::gjInterTrophy::SetSort(const int* piIDList, const size_t& iNum)
{
    if(iNum)
    {
        // clear sort list
        m_aiSort.clear();

        // add IDs to sort list
        for(size_t i = 0; i < iNum; ++i)
            m_aiSort.push_back(piIDList[i]);
    }

    // apply sort attribute
    FOR_EACH(it, m_apTrophy)
        it->second->__SetSort(0);
    for(size_t i = 0; i < m_aiSort.size(); ++i)
        if(m_apTrophy.count(m_aiSort[i])) m_apTrophy[m_aiSort[i]]->__SetSort(int(i+1));
}


// ****************************************************************
/* define secret trophy objects */
void gjAPI::gjInterTrophy::SetSecret(const int* piIDList, const size_t& iNum)
{
    if(iNum)
    {
        // clear secret list
        m_aiSecret.clear();

        // add IDs to secret list
        for(size_t i = 0; i < iNum; ++i)
            m_aiSecret.push_back(piIDList[i]);
    }

    // apply secret attribute
    FOR_EACH(it, m_apTrophy)
        it->second->__SetSecret(false);
    FOR_EACH(it, m_aiSecret)
        if(m_apTrophy.count(*it)) m_apTrophy[*it]->__SetSecret(true);
}


// ****************************************************************
/* define hidden trophy objects */
void gjAPI::gjInterTrophy::SetHidden(const int* piIDList, const size_t& iNum)
{
    if(iNum)
    {
        // clear hidden list
        m_aiHidden.clear();

        // add IDs to hidden list
        for(size_t i = 0; i < iNum; ++i)
            m_aiHidden.push_back(piIDList[i]);
    }

    // apply hidden attribute and remove all hidden trophy objects
    FOR_EACH(it, m_aiHidden)
        if(m_apTrophy.count(*it)) m_apTrophy.erase(m_apTrophy.find(*it));
}


// ****************************************************************
/* check for cached trophy objects */
int gjAPI::gjInterTrophy::__CheckCache(const int& iAchieved, gjTrophyList* papOutput)
{
    // retrieve cached trophies
    if(m_apTrophy.size() > 1)
    {
        if(papOutput)
        {
            gjTrophyList apConvert;
            apConvert.reserve(GJ_API_RESERVE_TROPHY);

            // add sorted trophies
            FOR_EACH(it, m_aiSort)
                if(m_apTrophy.count(*it)) apConvert.push_back(m_apTrophy[*it]);

            // add missing unsorted trophies
            FOR_EACH(it, m_apTrophy)
            {
                if(it->first)
                {
                    if(std::find(apConvert.begin(), apConvert.end(), it->second) == apConvert.end())
                        apConvert.push_back(it->second);
                }
            }

            // check for achieved status
            FOR_EACH(it, apConvert)
            {
                gjTrophy* pTrophy = (*it);

                if((iAchieved > 0 &&  pTrophy->IsAchieved()) ||
                   (iAchieved < 0 && !pTrophy->IsAchieved()) || !iAchieved)
                    (*papOutput).push_back(pTrophy);
            }
        }
        return GJ_OK;
    }
    return GJ_NO_DATA_FOUND;
}


// ****************************************************************
/* process trophy data and cache trophy objects */
int gjAPI::gjInterTrophy::__Process(const std::string& sData, void* pAdd, gjTrophyList* papOutput)
{
    // parse output
    gjDataList aaReturn;
    if(m_pAPI->ParseRequestKeypair(sData, &aaReturn) != GJ_OK)
    {
        gjAPI::ErrorLogAdd("API Error: could not parse trophies");
        return GJ_REQUEST_FAILED;
    }

    // offline-cache trophy data
    if(!aaReturn.empty()) this->__SaveOffCache(sData);
    if(m_iCache == 0) m_iCache = 2;

    // create and cache trophy objects
    FOR_EACH(it, aaReturn)
    {
        gjTrophy* pNewTrophy = new gjTrophy(*it, m_pAPI);
        const int iID = pNewTrophy->GetID();

        if(m_apTrophy.count(iID))
        {
            *m_apTrophy[iID] = *pNewTrophy;
            SAFE_DELETE(pNewTrophy)
        }
        else m_apTrophy[iID] = pNewTrophy;
    }

    // apply attributes
    this->SetSort  (NULL, 0);
    this->SetSecret(NULL, 0);
    this->SetHidden(NULL, 0);

    return (this->__CheckCache(P_TO_I(pAdd), papOutput) == GJ_OK) ? GJ_OK : GJ_NO_DATA_FOUND;
}


// ****************************************************************
/* save trophy data to a cache file */
void gjAPI::gjInterTrophy::__SaveOffCache(const std::string& sData)
{
    if(!GJ_API_OFFCACHE_TROPHY) return;
    if(m_iCache != 0) return;

    // open cache file
    std::FILE* pFile = std::fopen(GJ_API_OFFCACHE_NAME, "w");
    if(pFile)
    {
        // write data and close cache file
        std::fputs("[TROPHY]\n", pFile);
        std::fputs(sData.c_str(), pFile);
        std::fclose(pFile);
    }
}


// ****************************************************************
/* load trophy data from a cache file */
void gjAPI::gjInterTrophy::__LoadOffCache()
{
    if(!GJ_API_OFFCACHE_TROPHY) return;
    if(m_iCache != 0) return;

    // open cache file
    std::FILE* pFile = std::fopen(GJ_API_OFFCACHE_NAME, "r");
    if(pFile)
    {
        // read trophy header
        char acHeader[32];
        std::fscanf(pFile, "%31[^\n]%*c", acHeader);

        // read trophy data
        std::string sData;
        while(true)
        {
            char acLine[1024];
            std::fscanf(pFile, "%1023[^\n]%*c", acLine);
            if(std::feof(pFile)) break;

            if(std::strlen(acLine) > 1)
            {
                sData += acLine;
                sData += '\n';
            }
        }

        // close cache file
        std::fclose(pFile);

        if(!sData.empty())
        {
            // flag offline caching and load offline-cached trophies
            m_iCache = 1;
            this->__Process(sData, NULL, NULL);
        }
    }
}


// ****************************************************************
/* constructor */
gjAPI::gjInterScore::gjInterScore(gjAPI* pAPI, gjNetwork* pNetwork)noexcept
: m_pAPI     (pAPI)
, m_pNetwork (pNetwork)
{
    // create NULL score table for secure object handling
    gjData pNullData;
    pNullData["id"]   = "0";
    pNullData["name"] = "NOT FOUND";
    m_apScoreTable[0] = new gjScoreTable(pNullData, m_pAPI);
}


// ****************************************************************
/* destructor */
gjAPI::gjInterScore::~gjInterScore()
{
    // delete all score tables and scores entries
    FOR_EACH(it, m_apScoreTable)
        SAFE_DELETE(it->second)

    // clear container
    m_apScoreTable.clear();
}


// ****************************************************************
/* access score table objects directly (may block) */
gjScoreTable* gjAPI::gjInterScore::GetScoreTable(const int &iID)
{
    if(m_apScoreTable.size() <= 1)
    {
        // wait for prefetching
        if(GJ_API_PREFETCH) m_pNetwork->Wait(2);
        if(m_apScoreTable.size() <= 1)
        {
            gjScoreTableMap apOutput;
            this->FetchScoreTablesNow(&apOutput);
        }
    }
    gjScoreTable* pPrimary = gjScoreTable::GetPrimary();
    return iID ? (m_apScoreTable.count(iID) ? m_apScoreTable[iID] : m_apScoreTable[0]) : (pPrimary ? pPrimary : m_apScoreTable[0]);
}


// ****************************************************************
/* delete all cached score table objects and score entries */
void gjAPI::gjInterScore::ClearCache()
{
    // save NULL score table
    gjScoreTable* pNull = m_apScoreTable[0]; m_apScoreTable.erase(0);

    // delete score tables and scores entries
    FOR_EACH(it, m_apScoreTable)
        SAFE_DELETE(it->second)

    // clear container
    m_apScoreTable.clear();
    m_apScoreTable[0] = pNull;
}


// ****************************************************************
/* check for cached score table objects */
int gjAPI::gjInterScore::__CheckCache(gjScoreTableMap* papOutput)
{
    // retrieve cached score tables
    if(m_apScoreTable.size() > 1)
    {
        if(papOutput)
        {
            FOR_EACH(it, m_apScoreTable)
                if(it->first) (*papOutput)[it->first] = it->second;
        }
        return GJ_OK;
    }
    return GJ_NO_DATA_FOUND;
}


// ****************************************************************
/* process score table data and cache score table objects */
int gjAPI::gjInterScore::__Process(const std::string& sData, void* pAdd, gjScoreTableMap* papOutput)
{
    // parse output
    gjDataList aaReturn;
    if(m_pAPI->ParseRequestKeypair(sData, &aaReturn) != GJ_OK)
    {
        gjAPI::ErrorLogAdd("API Error: could not parse score tables");
        return GJ_REQUEST_FAILED;
    }

    // create and cache score tables
    FOR_EACH(it, aaReturn)
    {
        gjScoreTable* pNewScoreTable = new gjScoreTable(*it, m_pAPI);
        const int iID = pNewScoreTable->GetID();

        if(m_apScoreTable.count(iID)) SAFE_DELETE(pNewScoreTable)
        else m_apScoreTable[iID] = pNewScoreTable;
    }

    return (this->__CheckCache(papOutput) == GJ_OK) ? GJ_OK : GJ_NO_DATA_FOUND;
}


// ****************************************************************
/* constructor */
gjAPI::gjInterDataStore::gjInterDataStore(const int& iType, gjAPI* pAPI, gjNetwork* pNetwork)noexcept
: m_iType    (iType)
, m_pAPI     (pAPI)
, m_pNetwork (pNetwork)
{
}


// ****************************************************************
/* destructor */
gjAPI::gjInterDataStore::~gjInterDataStore()
{
    this->ClearCache();
}


// ****************************************************************
/* create and access data store items directly */
gjDataItem* gjAPI::gjInterDataStore::GetDataItem(const std::string& sKey)
{
    // create new data store item
    if(!m_apDataItem.count(sKey))
    {
        gjData asDataItemData;
        asDataItemData["key"] = sKey;
        m_apDataItem[sKey] = new gjDataItem(asDataItemData, m_iType, m_pAPI);
    }

    return m_apDataItem.count(sKey) ? m_apDataItem[sKey] : NULL;
}


// ****************************************************************
/* delete all cached data store items */
void gjAPI::gjInterDataStore::ClearCache()
{
    // delete data store items
    FOR_EACH(it, m_apDataItem)
        SAFE_DELETE(it->second)

    // clear container
    m_apDataItem.clear();
}


// ****************************************************************
/* check for cached data store items */
int gjAPI::gjInterDataStore::__CheckCache(gjDataItemMap* papOutput)
{
    // retrieve cached data store items
    if(!m_apDataItem.empty())
    {
        if(papOutput)
        {
            FOR_EACH(it, m_apDataItem)
                (*papOutput)[it->first] = it->second;
        }
        return GJ_OK;
    }
    return GJ_NO_DATA_FOUND;
}


// ****************************************************************
/* process data store data and cache data store items */
int gjAPI::gjInterDataStore::__Process(const std::string& sData, void* pAdd, gjDataItemMap* papOutput)
{
    // parse output
    gjDataList aaReturn;
    if(m_pAPI->ParseRequestKeypair(sData, &aaReturn) != GJ_OK)
    {
        gjAPI::ErrorLogAdd("API Error: could not parse data store items");
        return GJ_REQUEST_FAILED;
    }

    // create and cache data store items
    FOR_EACH(it, aaReturn)
    {
        gjDataItem* pNewDataItem = new gjDataItem(*it, m_iType, m_pAPI);
        const std::string& sKey = pNewDataItem->GetKey();

        if(m_apDataItem.count(sKey))
        {
            SAFE_DELETE(pNewDataItem)
            pNewDataItem = m_apDataItem[sKey];
        }
        else m_apDataItem[sKey] = pNewDataItem;

        if(papOutput) (*papOutput)[sKey] = pNewDataItem;
    }

    return aaReturn.empty() ? GJ_NO_DATA_FOUND : GJ_OK;
}


// ****************************************************************
/* constructor */
gjAPI::gjInterFile::gjInterFile(gjAPI* pAPI, gjNetwork* pNetwork)noexcept
: m_pAPI     (pAPI)
, m_pNetwork (pNetwork)
{
    // reserve some memory
    m_asFile.reserve(GJ_API_RESERVE_FILE);
}


// ****************************************************************
/* destructor */
gjAPI::gjInterFile::~gjInterFile()
{
    this->ClearCache();
}


// ****************************************************************
/* delete all cached file paths */
void gjAPI::gjInterFile::ClearCache()
{
    // clear container
    m_asFile.clear();
}


// ****************************************************************
/* check for cached files */
int gjAPI::gjInterFile::__CheckCache(const std::string& sPath)
{
    // compare cached file paths
    FOR_EACH(it, m_asFile)
    {
        if(sPath == (*it))
            return GJ_OK;
    }
    return GJ_NO_DATA_FOUND;
}


// ****************************************************************
/* process downloaded file */
int gjAPI::gjInterFile::__Process(const std::string& sData, void* pAdd, std::string* psOutput)
{
    // save path of the file
    if(this->__CheckCache(sData) != GJ_OK) m_asFile.push_back(sData);
    if(psOutput) (*psOutput) = sData;

    return GJ_OK;
}


// ****************************************************************
/* constructor */
gjAPI::gjAPI(const int iGameID, const std::string sGamePrivateKey)noexcept
: m_iGameID         (iGameID)
, m_sGamePrivateKey (sGamePrivateKey)
, m_sUserName       ("")
, m_sUserToken      ("")
, m_iNextPing       (0)
, m_bActive         (false)
, m_bConnected      (false)
, m_sProcUserName   ("")
, m_sProcUserToken  ("")
{
    // init error log
    gjAPI::ErrorLogReset();

    // pre-process the game ID
    m_sProcGameID = gjAPI::UtilIntToString(m_iGameID);

    // create network object
    m_pNetwork = new gjNetwork(this);

    // create sub-interface objects
    m_pInterUser            = new gjInterUser(this, m_pNetwork);
    m_pInterTrophy          = new gjInterTrophy(this, m_pNetwork);
    m_pInterScore           = new gjInterScore(this, m_pNetwork);
    m_pInterDataStoreGlobal = new gjInterDataStore(0, this, m_pNetwork);
    m_pInterDataStoreUser   = new gjInterDataStore(1, this, m_pNetwork);
    m_pInterFile            = new gjInterFile(this, m_pNetwork);

    // prefetch score tables
    if(GJ_API_PREFETCH && iGameID) m_pInterScore->FetchScoreTablesCall(GJ_NETWORK_NULL_THIS(gjScoreTableMap));
}


// ****************************************************************
/* destructor */
gjAPI::~gjAPI()
{
    // logout last user
    this->Logout();

    // delete network object
    SAFE_DELETE(m_pNetwork)

    // delete sub-interface objects
    SAFE_DELETE(m_pInterUser)
    SAFE_DELETE(m_pInterTrophy)
    SAFE_DELETE(m_pInterScore)
    SAFE_DELETE(m_pInterDataStoreGlobal)
    SAFE_DELETE(m_pInterDataStoreUser)
    SAFE_DELETE(m_pInterFile)
}


// ****************************************************************
/* explicitly initialize the object */
void gjAPI::Init(const int& iGameID, const std::string& sGamePrivateKey)
{
    // save game data
    m_iGameID         = iGameID;
    m_sGamePrivateKey = sGamePrivateKey;

    // pre-process the game ID
    m_sProcGameID = gjAPI::UtilIntToString(m_iGameID);

    // prefetch score tables
    if(GJ_API_PREFETCH && iGameID) m_pInterScore->FetchScoreTablesCall(GJ_NETWORK_NULL_THIS(gjScoreTableMap));
}


// ****************************************************************
/* main update function of the library */
void gjAPI::Update()
{
    // update network object
    m_pNetwork->Update();

    if(!this->IsUserConnected()) return;

    if(m_iNextPing)
    {
        // update ping for the user session
        const time_t iCurTime = time(NULL);
        if(iCurTime >= m_iNextPing)
        {
            m_iNextPing = iCurTime + GJ_API_PING_TIME;
            this->__PingSession(m_bActive);
        }
    }
}


// ****************************************************************
/* logout with specific user */
int gjAPI::Logout()
{
    if(!this->IsUserConnected()) return GJ_NOT_CONNECTED;

    // clear user specific data
    m_pInterTrophy->ClearCache(false);
    m_pInterDataStoreUser->ClearCache();

    // close the user session
    if(m_iNextPing) this->__CloseSession();

    // clear main user data
    m_sUserName      = "";
    m_sUserToken     = "";
    m_sProcUserName  = "";
    m_sProcUserToken = "";

    // clear connection
    m_bConnected = false;

    return GJ_OK;
}


// ****************************************************************
/* parse a valid response string in keypair format */
int gjAPI::ParseRequestKeypair(const std::string& sInput, gjDataList* paaOutput)
{
    if(!paaOutput) return GJ_INVALID_INPUT;

    gjData aData;
    std::istringstream sStream(sInput);
    std::string sToken;

    // loop through input string
    while(std::getline(sStream, sToken))
    {
        // remove redundant characters
        gjAPI::UtilTrimString(&sToken);
        if(sToken.empty()) continue;

        // separate key and value
        const size_t iPos        = sToken.find(':');
        const std::string sKey   = sToken.substr(0, iPos);
        const std::string sValue = sToken.substr(iPos + 2, sToken.length() - iPos - 3);

        // next data block on same key
        if(aData.count(sKey.c_str()))
        {
            paaOutput->push_back(aData);
            aData.clear();
        }

        // create key and save value
        aData[sKey.c_str()] = sValue;
    }

    // insert last data block and check size
    if(!aData.empty()) paaOutput->push_back(aData);
    if(paaOutput->empty())
    {
        paaOutput->push_back(aData);
        gjAPI::ErrorLogAdd("API Error: string parsing failed");
        return GJ_INVALID_INPUT;
    }

    // check for failed request
    if(paaOutput->front()["success"] != "true")
    {
        gjAPI::ErrorLogAdd("API Error: request was unsuccessful");
        gjAPI::ErrorLogAdd("API Error: " + paaOutput->front()["message"]);
        return GJ_REQUEST_FAILED;
    }

    return GJ_OK;
}


// ****************************************************************
/* parse a valid response string in Dump format */
int gjAPI::ParseRequestDump(const std::string& sInput, std::string* psOutput)
{
    if(!psOutput) return GJ_INVALID_INPUT;

    // read status
    const std::string sStatus = sInput.substr(0, sInput.find_first_of(13));

    // read data
    (*psOutput) = sInput.substr(sStatus.length()+2);

    // check for failed request
    if(sStatus != "SUCCESS")
    {
        gjAPI::ErrorLogAdd("API Error: request was unsuccessful");
        gjAPI::ErrorLogAdd("API Error: " + (*psOutput));
        return GJ_REQUEST_FAILED;
    }

    return GJ_OK;
}


// ****************************************************************
/* delete all cached objects */
void gjAPI::ClearCache()
{
    // clear cache of all sub-interfaces
    m_pInterUser->ClearCache();
    m_pInterTrophy->ClearCache(true);
    m_pInterScore->ClearCache();
    m_pInterDataStoreGlobal->ClearCache();
    m_pInterDataStoreUser->ClearCache();
    m_pInterFile->ClearCache();
}


// ****************************************************************
/* escape a string for proper url calling */
std::string gjAPI::UtilEscapeString(const std::string& sString)
{
    std::string sOutput = "";

    // loop through input string
    for(size_t i = 0; i < sString.length(); ++i)
    {
        // check the character type
        if
        (
            (48 <= sString[i] && sString[i] <=  57) || // 0-9
            (65 <= sString[i] && sString[i] <=  90) || // ABC...XYZ
            (97 <= sString[i] && sString[i] <= 122) || // abc...xyz
            (
                sString[i] == '~' || sString[i] == '.'  ||
                sString[i] == '-' || sString[i] == '_'
            )
        )
        {
            // add valid character
            sOutput += sString[i];
        }
        else
        {
            // convert character to hexadecimal value
            sOutput += "%" + gjAPI::UtilCharToHex(sString[i]);
        }
    }

    return sOutput;
}


// ****************************************************************
/* trim a standard string on both sides */
void gjAPI::UtilTrimString(std::string* psInput)
{
    const size_t iFirst = psInput->find_first_not_of(" \n\r\t");
    if(iFirst != std::string::npos) psInput->erase(0, iFirst);

    const size_t iLast = psInput->find_last_not_of(" \n\r\t");
    if(iLast != std::string::npos) psInput->erase(iLast+1);
}


// ****************************************************************
/* convert a character into his hexadecimal value */
std::string gjAPI::UtilCharToHex(const char& cChar)
{
    int iValue = (int)cChar;
    if(iValue < 0) iValue += 256;

    char acBuffer[8];
    std::sprintf(acBuffer, "%02X", iValue);

    return acBuffer;
}


// ****************************************************************
/* simply convert an integer into a string */
std::string gjAPI::UtilIntToString(const int& iInt)
{
    char acBuffer[32];
    std::sprintf(acBuffer, "%d", iInt);

    return acBuffer;
}


// ****************************************************************
/* create a folder hierarchy */
void gjAPI::UtilCreateFolder(const std::string& sFolder)
{
    size_t iPos = 0;

    // loop through path
    while((iPos = sFolder.find_first_of("/\\", iPos+2)) != std::string::npos)
    {
        const std::string sSubFolder = sFolder.substr(0, iPos);

        // create subfolder
#if defined(_GJ_WINDOWS_)
        CreateDirectoryA(sSubFolder.c_str(), NULL);
#else
        mkdir(sSubFolder.c_str(), S_IRWXU);
#endif
    }
}


// ****************************************************************
/* get timestamp as string */
std::string gjAPI::UtilTimestamp(const time_t iTime)
{
    // format the time value
    tm* pFormat = std::localtime(&iTime);

    // create output
    char acBuffer[16];
    std::sprintf(acBuffer, "%02d:%02d:%02d", pFormat->tm_hour, pFormat->tm_min, pFormat->tm_sec);

    return acBuffer;
}


// ****************************************************************
/* reset error log */
void gjAPI::ErrorLogReset()
{
    if(GJ_API_LOGFILE)
    {
        // remove error log file if empty
        if(s_asLog.empty())
            std::remove(GJ_API_LOGFILE_NAME);
    }
}


// ****************************************************************
/* add error log entry */
void gjAPI::ErrorLogAdd(const std::string& sMsg)
{
    const std::string sTimeMsg = "[" + gjAPI::UtilTimestamp() + "] " + sMsg;

    // add message
    s_asLog.push_back(sTimeMsg);

    if(GJ_API_LOGFILE)
    {
        // add message to error log file
        std::FILE* pFile = std::fopen(GJ_API_LOGFILE_NAME, "a");
        if(pFile)
        {
            std::fprintf(pFile, "%s\n", sTimeMsg.c_str());
            std::fclose(pFile);
        }
    }

#if defined(_GJ_DEBUG_)

    // print message to terminal
    std::cerr << "(!GJ) " << sTimeMsg << std::endl;

#endif
}


// ****************************************************************
/* open the user session */
int gjAPI::__OpenSession()
{
    if(!this->IsUserConnected()) return GJ_NOT_CONNECTED;

    // send non-blocking open request
    if(m_pNetwork->SendRequest("/sessions/open/"
                               "?game_id="    + m_sProcGameID   +
                               "&username="   + m_sProcUserName +
                               "&user_token=" + m_sProcUserToken,
                               NULL, this, &gjAPI::Null, NULL, GJ_NETWORK_NULL_THIS(std::string))) return GJ_REQUEST_FAILED;

    // init session attributes
    m_iNextPing = std::time(NULL) + GJ_API_PING_TIME;
    m_bActive   = true;

    return GJ_OK;
}


// ****************************************************************
/* ping the user session */
int gjAPI::__PingSession(const bool& bActive)
{
    if(!this->IsUserConnected()) return GJ_NOT_CONNECTED;

    // use active status
    const std::string sActive = bActive ? "active" : "idle";

    // send non-blocking ping request
    if(m_pNetwork->SendRequest("/sessions/ping/"
                               "?game_id="    + m_sProcGameID    +
                               "&username="   + m_sProcUserName  +
                               "&user_token=" + m_sProcUserToken +
                               "&status="     + sActive,
                               NULL, this, &gjAPI::Null, NULL, GJ_NETWORK_NULL_THIS(std::string))) return GJ_REQUEST_FAILED;

    return GJ_OK;
}


// ****************************************************************
/* close the user session */
int gjAPI::__CloseSession()
{
    if(!this->IsUserConnected()) return GJ_NOT_CONNECTED;

    // send non-blocking close request
    if(m_pNetwork->SendRequest("/sessions/close/"
                               "?game_id="    + m_sProcGameID   +
                               "&username="   + m_sProcUserName +
                               "&user_token=" + m_sProcUserToken,
                               NULL, this, &gjAPI::Null, NULL, GJ_NETWORK_NULL_THIS(std::string))) return GJ_REQUEST_FAILED;

    // clear session attributes
    m_iNextPing = 0;

    return GJ_OK;
}


// ****************************************************************
/* callback for login with specific user */
int gjAPI::__LoginCallback(const std::string& sData, void* pAdd, int* pbOutput)
{
    // check for success
    gjDataList aaReturn;
    if(this->ParseRequestKeypair(sData, &aaReturn) != GJ_OK)
    {
        gjAPI::ErrorLogAdd("API Error: could not authenticate user <" + m_sUserName + ">");

        // clear main user data
        m_sUserName      = "";
        m_sUserToken     = "";
        m_sProcUserName  = "";
        m_sProcUserToken = "";

        // determine error type
        const int iError = std::strcmp(SAFE_MAP_GET(aaReturn[0], "success").c_str(), "false") ? GJ_NETWORK_ERROR : GJ_REQUEST_FAILED;

        if(pbOutput) (*pbOutput) = iError;
        return pbOutput ? GJ_OK : iError;
    }

    // set connection
    m_bConnected = true;

    // open the user session
    if(pAdd) this->__OpenSession();

    // prefetch user data
    if(GJ_API_PREFETCH)
    {
        m_pInterUser->FetchUserCall(0, GJ_NETWORK_NULL_THIS(gjUserPtr));
        m_pInterTrophy->FetchTrophiesCall(0, GJ_NETWORK_NULL_THIS(gjTrophyList));
        m_pInterDataStoreUser->FetchDataItemsCall(GJ_NETWORK_NULL_THIS(gjDataItemMap));
    }

    if(pbOutput) (*pbOutput) = GJ_OK;
    return GJ_OK;
}