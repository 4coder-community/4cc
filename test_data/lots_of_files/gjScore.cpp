///////////////////////////////////////////////////////////////////
//*-------------------------------------------------------------*//
//| Part of the Game Jolt API C++ Library (http://gamejolt.com) |//
//*-------------------------------------------------------------*//
//| Released under the zlib License                             |//
//| More information available in the readme file               |//
//*-------------------------------------------------------------*//
///////////////////////////////////////////////////////////////////
#include "gjAPI.h"

#include <algorithm>
gjScoreTable* gjScoreTable::s_pPrimary = NULL;


// ****************************************************************
/* constructor */
gjScoreTable::gjScoreTable(const gjData& aScoreTableData, gjAPI* pAPI)noexcept
: m_iID          (atoi(SAFE_MAP_GET(aScoreTableData, "id").c_str()))
, m_sTitle       (SAFE_MAP_GET(aScoreTableData, "name"))
, m_sDescription (SAFE_MAP_GET(aScoreTableData, "description"))
, m_iSortDir     (0)
, m_pAPI         (pAPI)
{
    // set the primary status
    m_bPrimary = (SAFE_MAP_GET(aScoreTableData, "primary") == "1") ? true : false;

    // set pointer to the primary score table
    if(m_bPrimary && !s_pPrimary) s_pPrimary = this;

    // reserve some memory
    m_apScore.reserve(GJ_API_RESERVE_SCORE);
    m_apVerify.reserve(4);
}


// ****************************************************************
/* destructor */
gjScoreTable::~gjScoreTable()
{
    // reset pointer to the primary score table
    if(s_pPrimary == this) s_pPrimary = NULL;

    // delete score entries
    FOR_EACH(it, m_apScore)
        SAFE_DELETE(*it)
    FOR_EACH(it, m_apVerify)
        SAFE_DELETE(*it)

    // clear container
    m_apScore.clear();
    m_apVerify.clear();
}


// ****************************************************************
/* check for cached score entries */
int gjScoreTable::__CheckCache(const bool bOnlyUser, const int& iLimit, gjScoreList* papOutput)
{
    if(!bOnlyUser)  return GJ_INVALID_INPUT;
    if(!m_iSortDir) return GJ_INVALID_CALL;

    // fetch cached main user
    gjUserPtr pMainUser;
    if(m_pAPI->InterUser()->CheckCache(m_pAPI->GetUserName(), &pMainUser) != GJ_OK) 
        return GJ_REQUEST_CANCELED;

    if(papOutput)
    {
        // retrieve cached main user score entries
        int iCount = 0;
        FOR_EACH(it, m_apScore)
        {
            gjScore* pScore = (*it);

            if(pScore->GetUserID() == pMainUser->GetID())
            {
                (*papOutput).push_back(pScore);
                if(++iCount >= iLimit) break;
            }
        }
        return iCount ? GJ_OK : GJ_NO_DATA_FOUND;
    }

    return GJ_NO_DATA_FOUND;
}


// ****************************************************************
/* process score data and cache score entries */
int gjScoreTable::__Process(const std::string& sData, void* pAdd, gjScoreList* papOutput)
{
    // parse output
    gjDataList aaReturn;
    if(m_pAPI->ParseRequestKeypair(sData, &aaReturn) != GJ_OK)
    {
        gjAPI::ErrorLogAdd("API Error: could not parse scores");
        return GJ_REQUEST_FAILED;
    }

    // create score entries
    FOR_EACH(it, aaReturn)
    {
        gjScore* pNewScore = new gjScore(*it, this, m_pAPI);
        bool bNew = true;

        // remove invalid score
        if(pNewScore->GetUserName() == "")
        {
            SAFE_DELETE(pNewScore)
            continue;
        }

        // check if specific score entry is already available
        FOR_EACH(it, m_apScore)
        {
            gjScore* pOldScore = (*it);

            if(pOldScore->GetUserName() == pNewScore->GetUserName() &&
               pOldScore->GetSort()     == pNewScore->GetSort())
            {
                // specific score entry already available
                SAFE_DELETE(pNewScore)
                pNewScore = pOldScore;
                bNew = false;
                break;
            }
        }

        if(bNew) m_apScore.push_back(pNewScore);
        if(papOutput) (*papOutput).push_back(pNewScore);
    }

    // try to determine the sort direction
    if(!m_iSortDir)
    {
        if(aaReturn.size() >= 2)
        {
            if(std::atoi(SAFE_MAP_GET(aaReturn[0], "sort").c_str()) > std::atoi(SAFE_MAP_GET(aaReturn[1], "sort").c_str())) m_iSortDir = GJ_SORT_DESC;
            if(std::atoi(SAFE_MAP_GET(aaReturn[0], "sort").c_str()) < std::atoi(SAFE_MAP_GET(aaReturn[1], "sort").c_str())) m_iSortDir = GJ_SORT_ASC;
        }
    }

    // sort score entries
    if(m_iSortDir) std::sort(m_apScore.begin(), m_apScore.end(), (m_iSortDir == GJ_SORT_DESC) ? SortDescending : SortAscending);

    return papOutput->size() ? GJ_OK : GJ_NO_DATA_FOUND;
}


// ****************************************************************
/* callback for successfully adding a score entry */
int gjScoreTable::__AddScoreCallback(const std::string& sData, void* pAdd, gjScorePtr* pOutput)
{
    gjScore* pNewScore = static_cast<gjScore*>(pAdd);

    // remove score entry from verification list
    FOR_EACH(it, m_apVerify)
    {
        if((*it) == pNewScore)
        {
            m_apVerify.erase(it);
            break;
        }
    }

    // cache score entry
    m_apScore.push_back(pNewScore);

    // sort score entries
    if(m_iSortDir) std::sort(m_apScore.begin(), m_apScore.end(), (m_iSortDir == GJ_SORT_DESC) ? SortDescending : SortAscending);

    if(pOutput) (*pOutput) = pNewScore;
    return GJ_OK;
}


// ****************************************************************
/* constructor */
gjScore::gjScore(const gjData& sScoreData, gjScoreTable* pScoreTable, gjAPI* pAPI)noexcept
: m_sScore      (SAFE_MAP_GET(sScoreData, "score"))
, m_iSort       (atoi(SAFE_MAP_GET(sScoreData, "sort").c_str()))
, m_sExtraData  (SAFE_MAP_GET(sScoreData, "extra_data"))
, m_sDate       (SAFE_MAP_GET(sScoreData, "stored"))
, m_pScoreTable (pScoreTable)
, m_pAPI        (pAPI)
{
    // check for guest
    const std::string sGuest = SAFE_MAP_GET(sScoreData, "guest");
    const bool bGuest = (sGuest == "") ? false : true;

    // set user ID and name
    m_iUserID   = bGuest ?     -1 : atoi(SAFE_MAP_GET(sScoreData, "user_id").c_str());
    m_sUserName = bGuest ? sGuest : SAFE_MAP_GET(sScoreData, "user");
}


// ****************************************************************
/* callbacks for sorting the score table */
bool SortAscending(const gjScore* i, const gjScore* j)  {return (i->GetSort() < j->GetSort());}
bool SortDescending(const gjScore* i, const gjScore* j) {return (i->GetSort() > j->GetSort());}