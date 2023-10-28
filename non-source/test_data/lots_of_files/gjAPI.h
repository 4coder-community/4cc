////////////////////////////////////////////////////////////////////////////////////////////
//*--------------------------------------------------------------------------------------*//
//|   ______    ______    __    __    ______          __    ______    __        ______   |//
//|  /\  ___\  /\  __ \  /\ "-./  \  /\  ___\        /\ \  /\  __ \  /\ \      /\__  _\  |//
//|  \ \ \__ \ \ \  __ \ \ \ \-./\ \ \ \  __\       _\_\ \ \ \ \/\ \ \ \ \____ \/_/\ \/  |//
//|   \ \_____\ \ \_\ \_\ \ \_\ \ \_\ \ \_____\    /\_____\ \ \_____\ \ \_____\   \ \_\  |//
//|    \/_____/  \/_/\/_/  \/_/  \/_/  \/_____/    \/_____/  \/_____/  \/_____/    \/_/  |//
//|                                                                                      |//
//*--------------------------------------------------------------------------------------*//
////////////////////////////////////////////////////////////////////////////////////////////
//*--------------------------------------------------------------------------------------*//
//| Game Jolt API C++ Library v1.0 (http://gamejolt.com)                                 |//
//*--------------------------------------------------------------------------------------*//
//| Special Thanks to:                                                                   |//
//|                                                                                      |//
//| David "CROS" DeCarmine, Joona "erakko" Melartin, Ashley Gwinnell, Bruno Assarisse,   |//
//| Jani "JNyknn" Nykänen, Jorge Martínez "Sasurai" Vargas                               |//
//*--------------------------------------------------------------------------------------*//
//| Copyright (c) 2013-2015 Martin Mauersics                                             |//
//|                                                                                      |//
//| This software is provided 'as-is', without any express or implied                    |//
//| warranty. In no event will the authors be held liable for any damages                |//
//| arising from the use of this software.                                               |//
//|                                                                                      |//
//| Permission is granted to anyone to use this software for any purpose,                |//
//| including commercial applications, and to alter it and redistribute it               |//
//| freely, subject to the following restrictions:                                       |//
//|                                                                                      |//
//|   1. The origin of this software must not be misrepresented; you must not            |//
//|   claim that you wrote the original software. If you use this software               |//
//|   in a product, an acknowledgment in the product documentation would be              |//
//|   appreciated but is not required.                                                   |//
//|                                                                                      |//
//|   2. Altered source versions must be plainly marked as such, and must not be         |//
//|   misrepresented as being the original software.                                     |//
//|                                                                                      |//
//|   3. This notice may not be removed or altered from any source                       |//
//|   distribution.                                                                      |//
//|                                                                                      |//
//|   4. This software may only be used within the terms of Game Jolt.                   |//
//|   (http://gamejolt.com/terms/)                                                       |//
//*--------------------------------------------------------------------------------------*//
////////////////////////////////////////////////////////////////////////////////////////////
//! \file
#pragma once
#ifndef _GJ_GUARD_API_H_
#define _GJ_GUARD_API_H_


/* --- configuration --- */
#define GJ_API_URL                 "http://gamejolt.com/api/game/v1"
#define GJ_API_AVATAR_DEFAULT      "http://gamejolt.com/img/no-avatar-1.png"
#define GJ_API_AVATAR_FORMAT       ".png"
#define GJ_API_TROPHY_DEFAULT_1    "http://gamejolt.com/img/trophy-bronze-1.jpg"
#define GJ_API_TROPHY_DEFAULT_2    "http://gamejolt.com/img/trophy-silver-1.jpg"
#define GJ_API_TROPHY_DEFAULT_3    "http://gamejolt.com/img/trophy-gold-1.jpg"
#define GJ_API_TROPHY_DEFAULT_4    "http://gamejolt.com/img/trophy-platinum-1.jpg"
#define GJ_API_TROPHY_SECRET       "http://gamejolt.com/img/trophy-secret-1.jpg"
#define GJ_API_PING_TIME           30
#define GJ_API_CRED                "gjapi-credentials.txt"
#define GJ_API_TEXT_NOW            "now"
#define GJ_API_TEXT_SECRET         "???"
#define GJ_API_RESERVE_CALL        16
#define GJ_API_RESERVE_CALL_OUTPUT 4
#define GJ_API_RESERVE_TROPHY      32
#define GJ_API_RESERVE_SCORE       64
#define GJ_API_RESERVE_FILE        32
#define GJ_API_TIMEOUT_CONNECTION  3
#define GJ_API_TIMEOUT_REQUEST     10
#define GJ_API_NET_COMPRESSION     "" // empty for all available compressions (identity, deflate, gzip)
#define GJ_API_NET_KEEPALIVE       true
#define GJ_API_LOGFILE             true
#define GJ_API_LOGFILE_NAME        "gjapi_log.txt"
#define GJ_API_PREFETCH            true
#define GJ_API_OFFCACHE_TROPHY     true // does not work on Android
#define GJ_API_OFFCACHE_NAME       "data/gjapi_cache.dat"
/* --- configuration --- */


// compiler
#if defined(_MSC_VER)
    #define _GJ_MSVC_  (_MSC_VER)
#endif
#if defined(__GNUC__)
    #define _GJ_GCC_   (__GNUC__*10000 + __GNUC_MINOR__*100 + __GNUC_PATCHLEVEL__*1)
#endif
#if defined(__MINGW32__)
    #define _CORE_MINGW_ (__GNUC__*10000 + __GNUC_MINOR__*100 + __GNUC_PATCHLEVEL__*1)
    #undef  _CORE_GCC_
#endif
#if defined(__clang__)
    #define _GJ_CLANG_ (__clang_major__*10000 + __clang_minor__*100 + __clang_patchlevel__*1)
#endif

// operating system
#if defined(_WIN32)
    #define _GJ_WINDOWS_ (1)
#endif
#if defined(__linux__)
    #define _GJ_LINUX_   (1)
#endif
#if defined(__APPLE__)
    #define _GJ_OSX_     (1)
#endif
#if defined(__ANDROID__)
    #define _GJ_ANDROID_ (1)
#endif

// debug mode
#if defined(_DEBUG) || defined(DEBUG) || (defined(_GJ_GCC_) && !defined(__OPTIMIZE__))
    #define _GJ_DEBUG_ (1)
#endif

// missing functionality
#if defined(_GJ_MSVC_)
    #if (_GJ_MSVC_) < 1800
        #define delete_func
    #else
        #define delete_func = delete
    #endif
    #if (_GJ_MSVC_) < 1700
        #define final
    #endif
    #define noexcept       throw()
    #define constexpr_func inline
    #define constexpr_var  const
#else
    #define delete_func    = delete
    #define constexpr_func constexpr
    #define constexpr_var  constexpr
#endif
#if defined(_GJ_GCC_)
    #if (_GJ_GCC_) < 40700
        #define override
        #define final
    #endif
#endif

// base libraries
#define _HAS_EXCEPTIONS (0)
#define _CRT_SECURE_NO_WARNINGS
#define _ALLOW_KEYWORD_MACROS

#if !defined(_GJ_WINDOWS_)
    #include <sys/stat.h>
#endif
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <string>
#include <map>
#include <vector>

/*! \param pOutputObj     output receiving object of **class T**
 *  \param OutputCallback callback function from **class T** with a specific return **type x**
 *  \param pOutputData    additional data which will be forwarded to the callback function
 *
 *  **Code Example**
 *  \code{.cpp}
 *  void Function(gjAPI& API, myClass& myObj)
 *  {
 *      // fetch an user with a callback (does not block)
 *      API.InterUser()->FetchUserCall("CROS", &myObj, &myClass::ReceiveUser, NULL);
 *  }
 *  \endcode */
#define GJ_NETWORK_OUTPUT(x)    T* pOutputObj,  void (T::*OutputCallback)(const x&, void*),               void* pOutputData
#define GJ_NETWORK_PROCESS      P* pProcessObj, int (P::*ProcessCallback)(const std::string&, void*, D*), void* pProcessData

#define GJ_NETWORK_OUTPUT_FW    pOutputObj,  OutputCallback,  pOutputData
#define GJ_NETWORK_PROCESS_FW   pProcessObj, ProcessCallback, pProcessData

#define GJ_NETWORK_NULL_THIS(d) this,   &gjAPI::Null<d>, NULL
#define GJ_NETWORK_NULL_API(d)  m_pAPI, &gjAPI::Null<d>, NULL

#define SAFE_DELETE(p)          {if(p) {delete   (p); (p) = NULL;}}
#define SAFE_DELETE_ARRAY(p)    {if(p) {delete[] (p); (p) = NULL;}}
#define SAFE_MAP_GET(o,s)       ((o).count(s) ? (o).at(s) : std::string(""))

#define FOR_EACH(i,c)           for(auto i = (c).begin(),  i ## __e = (c).end();  i != i ## __e; ++i)
#define FOR_EACH_REV(i,c)       for(auto i = (c).rbegin(), i ## __e = (c).rend(); i != i ## __e; ++i)

#if !defined(ARRAY_SIZE)
    template <typename T, std::size_t iSize> char (&__ARRAY_SIZE(T (&)[iSize]))[iSize];
    #define ARRAY_SIZE(a) (sizeof(__ARRAY_SIZE(a)))
#endif

#if !defined(DISABLE_COPY)
    #define DISABLE_COPY(c)                  \
        c             (const c&)delete_func; \
        c& operator = (const c&)delete_func;
#endif

#if !defined(P_TO_I)
    #define P_TO_I(x) ((int)(std::intptr_t)(void*)(x))   //!< pointer to int
    #define I_TO_P(x) ((void*)(std::intptr_t)(int)(x))   //!< int to pointer
#endif

#undef GetUserName

#include "MD5.h"
#include "Base64.h"
#include "gjLookup.h"
#include "curl/curl.h"

class gjAPI;
class gjUser;
class gjTrophy;
class gjScoreTable;
class gjScore;
class gjDataItem;

typedef gjLookup<std::string>              gjData;
typedef std::vector<gjData>                gjDataList;
typedef void*                              gjVoidPtr;
typedef gjUser*                            gjUserPtr;
typedef std::vector<gjTrophy*>             gjTrophyList;
typedef gjTrophy*                          gjTrophyPtr;
typedef std::map<int, gjScoreTable*>       gjScoreTableMap;
typedef std::vector<gjScore*>              gjScoreList;
typedef gjScore*                           gjScorePtr;
typedef std::map<std::string, gjDataItem*> gjDataItemMap;
typedef gjDataItem*                        gjDataItemPtr;

enum GJ_ERROR : unsigned char
{
    GJ_OK               = 0x00,   //!< everything is fine
    GJ_INVALID_CALL     = 0x01,   //!< function cannot be called
    GJ_INVALID_INPUT    = 0x02,   //!< function parameters are invalid
    GJ_REQUEST_FAILED   = 0x04,   //!< request to the API failed
    GJ_REQUEST_CANCELED = 0x08,   //!< request was canceled because of redundancy
    GJ_NOT_CONNECTED    = 0x10,   //!< no main user connected (login first)
    GJ_NO_DATA_FOUND    = 0x20,   //!< no data was found
    GJ_NETWORK_ERROR    = 0x40,   //!< error sending or receiving data, or failed to establish a connection
    GJ_FILE_ERROR       = 0x80    //!< error on opening, writing or finding a file
};

enum GJ_SORT_DIRECTION : int
{
    GJ_SORT_UNDEF =  0,   //!< undefined sorting
    GJ_SORT_DESC  =  1,   //!< descending sorting (3, 2, 1)
    GJ_SORT_ASC   = -1    //!< ascending sorting (1, 2, 3)
};

enum GJ_TROPHY_TYPE : int
{
    GJ_TROPHY_ALL          =  0,   //!< all trophies
    GJ_TROPHY_ACHIEVED     =  1,   //!< only achieved trophies
    GJ_TROPHY_NOT_ACHIEVED = -1    //!< only unachieved trophies
};

#include "gjNetwork.h"   // other header files are post-included


// ****************************************************************
/*! Main interface class of the library to connect with the Game Jolt API.\n
 *  Manages sessions, users, trophies, scores, data items and downloaded files.\n
 *  http://gamejolt.com/api/doc/game/
 *  \brief Main Interface */
class gjAPI final
{
private:
    // ****************************************************************
    /*! Sub-Interface class for user operations.\n
     *  http://gamejolt.com/api/doc/game/users/
     *  \brief User Sub-Interface */
    class gjInterUser final
    {
    private:
        std::map<int, gjUser*> m_apUser;   //!< cached user objects

        gjAPI* m_pAPI;                     //!< main interface access pointer
        gjNetwork* m_pNetwork;             //!< network access pointer


    public:
        gjInterUser(gjAPI* pAPI, gjNetwork* pNetwork)noexcept;
        ~gjInterUser();

        /*! \name Direct Access */
        //! @{
        /*! Get direct access to the user objects.\n
         *  This function may block to cache the specific user.
         *  \pre    Login maybe required
         *  \param  iID   Unique ID of an user (0 = current main user, Login required)
         *  \param  sName Unique name of an user
         *  \return Pointer to specific user or empty object (ID == 0) on error */
        gjUser* GetUser(const int& iID);
        gjUser* GetUser(const std::string& sName);
        gjUser* GetMainUser();
        //! @}

        /*! \name Fetch User Request */
        //! @{
        /*! Fetch and cache a specific user through an API request.
         *  \pre    Login maybe required
         *  \note   \b -Now blocks, \b -Call uses non-blocking callbacks
         *  \param  iID   Unique ID of an user (0 = current main user, Login required)
         *  \param  sName Unique name of an user
         *  \return **GJ_OK** on success\n
         *          **GJ_REQUEST_FAILED** if request was unsuccessful\n
         *          **GJ_INVALID_INPUT** if name string is empty\n
         *          (see #GJ_ERROR) */
                              inline int FetchUserNow(const int& iID, gjUserPtr* ppOutput)                     {if(!ppOutput) return GJ_INVALID_INPUT; return this->__FetchUser(iID, ppOutput, GJ_NETWORK_NULL_API(gjUserPtr));}
                              inline int FetchUserNow(const std::string& sName, gjUserPtr* ppOutput)           {if(!ppOutput) return GJ_INVALID_INPUT; return this->__FetchUser(sName, ppOutput, GJ_NETWORK_NULL_API(gjUserPtr));}
        template <typename T> inline int FetchUserCall(const int& iID, GJ_NETWORK_OUTPUT(gjUserPtr))           {return this->__FetchUser(iID, NULL, GJ_NETWORK_OUTPUT_FW);}
        template <typename T> inline int FetchUserCall(const std::string& sName, GJ_NETWORK_OUTPUT(gjUserPtr)) {return this->__FetchUser(sName, NULL, GJ_NETWORK_OUTPUT_FW);}
        //! @}

        /*! \name Check Cache */
        //! @{
        /*! Check and retrieve already cached user objects.
         *  \pre    Login maybe required
         *  \param  iID   Unique ID of an user (0 = current main user, Login required)
         *  \param  sName Unique name of an user
         *  \return **GJ_OK** on success\n
         *          **GJ_NO_DATA_FOUND** if user is not cached yet or does not even exist\n
         *          (see #GJ_ERROR) */
        inline int CheckCache(const int& iID, gjUserPtr* ppOutput)           {return this->__CheckCache(iID, ppOutput);}
        inline int CheckCache(const std::string& sName, gjUserPtr* ppOutput) {return this->__CheckCache(sName, ppOutput);}
        //! @}

        /*! \name Clear Cache */
        //! @{
        /*! Delete all cached user objects.
         *  \warning All external pointers will be invalid */
        void ClearCache();
        //! @}


    private:
        /*! \name Superior Request Functions */
        //! @{
        template <typename T> int __FetchUser(const int& iID, gjUserPtr* ppOutput, GJ_NETWORK_OUTPUT(gjUserPtr));
        template <typename T> int __FetchUser(const std::string& sName, gjUserPtr* ppOutput, GJ_NETWORK_OUTPUT(gjUserPtr));
        //! @}

        /*! \name Management Functions */
        //! @{
        int __CheckCache(const int& iID, gjUserPtr* ppOutput);
        int __CheckCache(const std::string& sName, gjUserPtr* ppOutput);
        int __Process(const std::string& sData, void* pAdd, gjUserPtr* ppOutput);
        //! @}
    };


    // ****************************************************************
    /*! Sub-Interface class for trophy operations.\n
     *  http://gamejolt.com/api/doc/game/trophies/
     *  \brief Trophy Sub-Interface */
    class gjInterTrophy final
    {
    private:
        std::map<int, gjTrophy*> m_apTrophy;   //!< cached trophy objects
        int m_iCache;                          //!< cache status (0 = empty, 1 = offline, 2 = online)

        std::vector<int> m_aiSort;             //!< layout of the returned trophy list
        std::vector<int> m_aiSecret;           //!< secret trophies (only fully visible when achieved)
        std::vector<int> m_aiHidden;           //!< hidden trophies (should never be visible, removed from memory)

        gjAPI* m_pAPI;                         //!< main interface access pointer
        gjNetwork* m_pNetwork;                 //!< network access pointer


    public:
        gjInterTrophy(gjAPI* pAPI, gjNetwork* pNetwork)noexcept;
        ~gjInterTrophy();

        /*! \name Direct Access */
        //! @{
        /*! Get direct access to the trophy objects.\n
         *  This function may block to cache all trophies.
         *  \pre    Login required
         *  \param  iID Unique ID of a trophy
         *  \return Pointer to specific trophy or empty object (ID == 0) on error */
        gjTrophy* GetTrophy(const int& iID);
        //! @}

        /*! \name Fetch Trophies Request */
        //! @{
        /*! Fetch and cache all trophies through an API request.\n
         *  You can sort the returned list with #SetSort.
         *  \pre    Login required
         *  \note   \b -Now blocks, \b -Call uses non-blocking callbacks
         *  \param  iAchieved Status of the requested trophies (see #GJ_TROPHY_TYPE)
         *  \return **GJ_OK** on success\n
         *          **GJ_REQUEST_FAILED** if request was unsuccessful\n
         *          **GJ_NOT_CONNECTED** if connection/login is missing\n
         *          **GJ_NO_DATA_FOUND** if no trophies were found\n
         *          (see #GJ_ERROR) */
                              inline int FetchTrophiesNow(const long& iAchieved, gjTrophyList* papOutput)          {if(!papOutput) return GJ_INVALID_INPUT; return this->__FetchTrophies(iAchieved, papOutput, GJ_NETWORK_NULL_API(gjTrophyList));}
        template <typename T> inline int FetchTrophiesCall(const long& iAchieved, GJ_NETWORK_OUTPUT(gjTrophyList)) {return this->__FetchTrophies(iAchieved, NULL, GJ_NETWORK_OUTPUT_FW);}
        //! @}

        /*! \name Clear Cache */
        //! @{
        /*! Delete all cached trophy objects.
         *  \warning All external pointers will be invalid
         *  \param   bFull Delete also the offline-cache */
        void ClearCache(const bool& bFull);
        //! @}

        /*! \name Control Trophies */
        //! @{
        /*! Define the way trophies are handled and returned from the interface.
         *  \param  piIDList Array with trophy IDs
         *  \param  iNum     Number of elements in the array */
        void SetSort(const int* piIDList, const size_t& iNum);
        void SetSecret(const int* piIDList, const size_t& iNum);
        void SetHidden(const int* piIDList, const size_t& iNum);
        //! @}


    private:
        /*! \name Superior Request Functions */
        //! @{
        template <typename T> int __FetchTrophies(const long& iAchieved, gjTrophyList* papOutput, GJ_NETWORK_OUTPUT(gjTrophyList));
        //! @}

        /*! \name Management Functions */
        //! @{
        int __CheckCache(const int& iAchieved, gjTrophyList* papOutput);
        int __Process(const std::string& sData, void* pAdd, gjTrophyList* papOutput);
        //! @}

        /*! \name Offline Cache Functions */
        //! @{
        void __SaveOffCache(const std::string& sData);
        void __LoadOffCache();
        //! @}
    };


    // ****************************************************************
    /*! Sub-Interface class for score operations.\n
     *  http://gamejolt.com/api/doc/game/scores/
     *  \brief Score Sub-Interface */
    class gjInterScore final
    {
    private:
        std::map<int, gjScoreTable*> m_apScoreTable;   //!< cached score table objects with semi-cached score entries

        gjAPI* m_pAPI;                                 //!< main interface access pointer
        gjNetwork* m_pNetwork;                         //!< network access pointer


    public:
        gjInterScore(gjAPI* pAPI, gjNetwork* pNetwork)noexcept;
        ~gjInterScore();

        /*! \name Direct Access */
        //! @{
        /*! Get direct access to score table objects.\n
         *  This function may block to cache all score tables.
         *  \param  iID Unique ID of a score table (0 = primary score table)
         *  \return Pointer to specific score table or empty object (ID == 0) on error */
               gjScoreTable* GetScoreTable(const int& iID);
        inline gjScoreTable* GetPrimaryTable() {return this->GetScoreTable(0);}
        //! @}

        /*! \name Fetch Score Tables Request */
        //! @{
        /*! Fetch and cache all score tables through an API request.
         *  \bug    The API returns already deleted score tables
         *
         *  \note   \b -Now blocks, \b -Call uses non-blocking callbacks
         *  \return **GJ_OK** on success\n
         *          **GJ_REQUEST_FAILED** if request was unsuccessful\n
         *          **GJ_NO_DATA_FOUND** if no score tables were found\n
         *          (see #GJ_ERROR) */
                              inline int FetchScoreTablesNow(gjScoreTableMap* papOutput)          {if(!papOutput) return GJ_INVALID_INPUT; return this->__FetchScoreTables(papOutput, GJ_NETWORK_NULL_API(gjScoreTableMap));}
        template <typename T> inline int FetchScoreTablesCall(GJ_NETWORK_OUTPUT(gjScoreTableMap)) {return this->__FetchScoreTables(NULL, GJ_NETWORK_OUTPUT_FW);}
        //! @}

        /*! \name Clear Cache */
        //! @{
        /*! Delete all cached score table objects and score entries.
         *  \warning All external pointers will be invalid */
        void ClearCache();
        //! @}


    private:
        /*! \name Superior Request Functions */
        //! @{
        template <typename T> int __FetchScoreTables(gjScoreTableMap* papOutput, GJ_NETWORK_OUTPUT(gjScoreTableMap));
        //! @}

        /*! \name Management Functions */
        //! @{
        int __CheckCache(gjScoreTableMap* papOutput);
        int __Process(const std::string& sData, void* pAdd, gjScoreTableMap* papOutput);
        //! @}
    };


    // ****************************************************************
    /*! Sub-Interface class for data store operations.\n
     *  http://gamejolt.com/api/doc/game/data-store/
     *  \brief Data Store Sub-Interface */
    class gjInterDataStore final
    {
    private:
        std::map<std::string, gjDataItem*> m_apDataItem;   //!< semi-cached data store items

        int m_iType;                                       //!< type of this interface (0 = global, 1 = user)

        gjAPI* m_pAPI;                                     //!< main interface access pointer
        gjNetwork* m_pNetwork;                             //!< network access pointer


    public:
        gjInterDataStore(const int& iType, gjAPI* pAPI, gjNetwork* pNetwork)noexcept;
        ~gjInterDataStore();

        /*! \name Direct Access */
        //! @{
        /*! Get direct access to data store items.\n
         *  This function creates a new data store item, if the key does not exist.\n
         *  To get all existing items, use \link FetchDataItemsNow FetchDataItems\endlink.
         *  \pre    Login maybe required
         *  \param  sKey Unique key of a data store item
         *  \return Pointer to specific data store item or NULL on error */
        gjDataItem* GetDataItem(const std::string& sKey);
        //! @}

        /*! \name Fetch Data Items Request */
        //! @{
        /*! Fetch and semi-cache all data store items through an API request.
         *  \note   \b -Now blocks, \b -Call uses non-blocking callbacks
         *  \return **GJ_OK** on success\n
         *          **GJ_REQUEST_FAILED** if request was unsuccessful\n
         *          **GJ_NOT_CONNECTED** if connection/login is missing\n
         *          **GJ_NO_DATA_FOUND** if no data items were found\n
         *          (see #GJ_ERROR) */
                              inline int FetchDataItemsNow(gjDataItemMap* papOutput)          {if(!papOutput) return GJ_INVALID_INPUT; return this->__FetchDataItems(papOutput, GJ_NETWORK_NULL_API(gjDataItemMap));}
        template <typename T> inline int FetchDataItemsCall(GJ_NETWORK_OUTPUT(gjDataItemMap)) {return this->__FetchDataItems(NULL, GJ_NETWORK_OUTPUT_FW);}
        //! @}

        /*! \name Clear Cache */
        //! @{
        /*! Delete all cached data store items.
         *  \warning All external pointers will be invalid */
        void ClearCache();
        //! @}

        /*! \name Get Attributes */
        //! @{
        inline const int& GetType()const {return m_iType;}   //!< \copybrief m_iType
        /*! */ //! @}


    private:
        /*! \name Superior Request Functions */
        //! @{
        template <typename T> int __FetchDataItems(gjDataItemMap* papOutput, GJ_NETWORK_OUTPUT(gjDataItemMap));
        //! @}

        /*! \name Management Functions */
        //! @{
        int __CheckCache(gjDataItemMap* papOutput);
        int __Process(const std::string& sData, void* pAdd, gjDataItemMap* papOutput);
        //! @}

        /*! \name Callback Functions */
        //! @{
        int __AddDataItemCallback(const std::string& sData, void* pAdd, gjTrophyPtr* pOutput);
        int __RemoveDataItemCallback(const std::string& sData, void* pAdd, gjTrophyPtr* ppOutput);
        //! @}
    };


    // ****************************************************************
    /*! Sub-Interface class for file downloads.\n
     *  \brief File Download Sub-Interface */
    class gjInterFile final
    {
    private:
        std::vector<std::string> m_asFile;   //!< cached file paths

        gjAPI* m_pAPI;                       //!< main interface access pointer
        gjNetwork* m_pNetwork;               //!< network access pointer


    public:
        gjInterFile(gjAPI* pAPI, gjNetwork* pNetwork)noexcept;
        ~gjInterFile();

        /*! \name Download File */
        /*! Download a file from any URL.\n
         *  Retrieve the local path of the file when finished.
         *  \note   \b -Now blocks, \b -Call uses non-blocking callbacks
         *  \warning You need to overwrite the file name if it's not apparent from the URL
         *  \param  sURL               Full path of the remote file
         *  \param  sToFolder          Relative path of the download target folder
         *  \param  sFileNameOverwrite Custom target file name (mandatory, if file name is not apparent from the URL)
         *  \return **GJ_OK** on success\n
         *          **GJ_REQUEST_FAILED** if request was unsuccessful\n
         *          **GJ_INVALID_INPUT** if URL string or target folder string is empty\n
         *          (see #GJ_ERROR) */
                              inline int DownloadFileNow(const std::string& sURL, const std::string& sToFolder, const std::string& sFileNameOverwrite, std::string* psOutput)           {if(!psOutput) return GJ_INVALID_INPUT; return this->__DownloadFile(sURL, sToFolder, sFileNameOverwrite, psOutput, GJ_NETWORK_NULL_API(std::string));}
        template <typename T> inline int DownloadFileCall(const std::string& sURL, const std::string& sToFolder, const std::string& sFileNameOverwrite, GJ_NETWORK_OUTPUT(std::string)) {return this->__DownloadFile(sURL, sToFolder, sFileNameOverwrite, NULL, GJ_NETWORK_OUTPUT_FW);}

        /*! \name Clear Cache */
        //! @{
        /*! Delete all cached file paths. */
        void ClearCache();
        //! @}


    private:
        /*! \name Superior Request Functions */
        //! @{
        template <typename T> int __DownloadFile(const std::string& sURL, const std::string& sToFolder, const std::string& sFileNameOverwrite, std::string* psOutput, GJ_NETWORK_OUTPUT(std::string));
        //! @}

        /*! \name Management Functions */
        //! @{
        int __CheckCache(const std::string& sPath);
        int __Process(const std::string& sData, void* pAdd, std::string* psOutput);
        //! @}
    };


private:
    int m_iGameID;                               //!< ID to identify the game
    std::string m_sGamePrivateKey;               //!< private key to generate MD5 signature

    std::string m_sUserName;                     //!< name of the main user
    std::string m_sUserToken;                    //!< token of the main user

    time_t m_iNextPing;                          //!< next ping for the user session
    bool m_bActive;                              //!< current status for the user session
    bool m_bConnected;                           //!< current connection status

    gjInterUser* m_pInterUser;                   //!< user Sub-Interface object
    gjInterTrophy* m_pInterTrophy;               //!< trophy Sub-Interface object
    gjInterScore* m_pInterScore;                 //!< score Sub-Interface object
    gjInterDataStore* m_pInterDataStoreGlobal;   //!< global data store Sub-Interface object
    gjInterDataStore* m_pInterDataStoreUser;     //!< user data store Sub-Interface object
    gjInterFile* m_pInterFile;                   //!< file download Sub-Interface object

    gjNetwork* m_pNetwork;                       //!< network object

    std::string m_sProcGameID;                   //!< already processed/converted game ID
    std::string m_sProcUserName;                 //!< already processed/escaped user name
    std::string m_sProcUserToken;                //!< already processed/escaped user token

    static std::vector<std::string> s_asLog;     //!< error log


public:
    gjAPI(const int iGameID = 0, const std::string sGamePrivateKey = "")noexcept;
    ~gjAPI();

    /*! \name Init */
    //! @{
    /*! Explicitely initialize the object after construction.
     *  \note Needs to be called after empty construction, and only once */
    void Init(const int& iGameID, const std::string& sGamePrivateKey);
    //! @}

    /*! \name Update */
    //! @{
    /*! Main update function of the library.
     *  \brief Must be executed in the main loop of the application
     *  \note Must be executed in the main loop of the application */
    void Update();
    //! @}

    /*! \name Login User*/
    //! @{
    /*! Login with a specific user.\n
     *  Authenticate user and establish an user session through the API.\n
     *  Prefetch the user object, trophies and user related data store items.
     *  \note   \b -Now blocks, \b -Call uses non-blocking callbacks
     *  \param  bSession   Establish an user session
     *  \param  sUserName  Login name of the user
     *  \param  sUserToken Token for that user
     *  \param  sCredPath  Relative path to the credentials file of the quick play function
     *  \return **GJ_OK** on success\n
     *          **GJ_REQUEST_FAILED** if request was unsuccessful\n
     *          **GJ_INVALID_CALL** if already connected\n
     *          **GJ_INVALID_INPUT** if user name or user token is missing\n
     *          **GJ_FILE_ERROR** if credentials file was not found\n
     *          **GJ_NETWORK_ERROR** if service is unavailable or request timed out\n
     *          (see #GJ_ERROR) */
                          inline int LoginNow(const bool& bSession, const std::string& sUserName, const std::string& sUserToken)                          {return __Login(bSession, sUserName, sUserToken, true, GJ_NETWORK_NULL_THIS(int));}
                          inline int LoginNow(const bool& bSession, const std::string& sCredPath)                                                         {return __Login(bSession, sCredPath, true, GJ_NETWORK_NULL_THIS(int));}
                          inline int LoginNow(const bool& bSession)                                                                                       {return __Login(bSession, GJ_API_CRED, true, GJ_NETWORK_NULL_THIS(int));}
    template <typename T> inline int LoginCall(const bool& bSession, const std::string& sUserName, const std::string& sUserToken, GJ_NETWORK_OUTPUT(int)) {return __Login(bSession, sUserName, sUserToken, false, GJ_NETWORK_OUTPUT_FW);}
    template <typename T> inline int LoginCall(const bool& bSession, const std::string& sCredPath, GJ_NETWORK_OUTPUT(int))                                {return __Login(bSession, sCredPath, false, GJ_NETWORK_OUTPUT_FW);}
    template <typename T> inline int LoginCall(const bool& bSession, GJ_NETWORK_OUTPUT(int))                                                              {return __Login(bSession, GJ_API_CRED, false, GJ_NETWORK_OUTPUT_FW);}
    //! @}

    /*! \name Logout User */
    //! @{
    /*! Logout with the current main user.
     *  \warning External pointers to trophies and user data store items will be invalid
     *  \return **GJ_OK** on success\n
     *          **GJ_NOT_CONNECTED** if connection/login is missing\n
     *          (see #GJ_ERROR) */
    int Logout();
    //! @}

    /*! \name Sub-Interface Access */
    //! @{
    inline gjInterUser*      InterUser()const            {return m_pInterUser;}
    inline gjInterTrophy*    InterTrophy()const          {return m_pInterTrophy;}
    inline gjInterScore*     InterScore()const           {return m_pInterScore;}
    inline gjInterDataStore* InterDataStoreGlobal()const {return m_pInterDataStoreGlobal;}
    inline gjInterDataStore* InterDataStoreUser()const   {return m_pInterDataStoreUser;}
    inline gjInterFile*      InterFile()const            {return m_pInterFile;}
    inline gjNetwork*        AccessNetwork()const        {return m_pNetwork;}
    //! @}

    /*! \name Send Custom Request */
    //! @{
    /*! Send a custom request to the API.\n
     *  Retrieve a response string when finished.
     *  \note   \b -Now blocks, \b -Call uses non-blocking callbacks\n
     *          Use "&POST<data>" at the end of the URL for a POST request
     *  \param  sURL Relative API request string
     *  \return **GJ_OK** on success\n
     *          **GJ_REQUEST_FAILED** if request was unsuccessful\n
     *          **GJ_NETWORK_ERROR** if session cannot be established\n
     *          (see #GJ_ERROR) */
                                                  inline int SendRequestNow(const std::string& sURL, std::string* psOutput)                                        {return m_pNetwork->SendRequest(sURL, psOutput, this, &gjAPI::Null, NULL, GJ_NETWORK_NULL_THIS(std::string));}
    template <typename T,             typename D> inline int SendRequestCall(const std::string& sURL, GJ_NETWORK_OUTPUT(D))                                        {return m_pNetwork->SendRequest(sURL, NULL, this, &gjAPI::Null, NULL, GJ_NETWORK_OUTPUT_FW);}
    template <typename T, typename P, typename D> inline int SendRequestCall(const std::string& sURL, GJ_NETWORK_PROCESS, GJ_NETWORK_OUTPUT(D))                    {return m_pNetwork->SendRequest(sURL, NULL, GJ_NETWORK_PROCESS_FW, GJ_NETWORK_OUTPUT_FW);}
    template <typename T, typename P, typename D> inline int SendRequest(const std::string& sURL, std::string* psOutput, GJ_NETWORK_PROCESS, GJ_NETWORK_OUTPUT(D)) {return m_pNetwork->SendRequest(sURL, psOutput, GJ_NETWORK_PROCESS_FW, GJ_NETWORK_OUTPUT_FW);}
    //! @}

    /*! \name Parse Request */
    //! @{
    /*! Parse a valid response string from the API.\n
     *  Retrieve a list of data objects or a single string.
     *  \param  sInput Valid response string
     *  \return **GJ_OK** on success\n
     *          **GJ_REQUEST_FAILED** if the parsed request was unsuccessful\n
     *          **GJ_INVALID_INPUT** if the string parsing failed\n
     *          (see #GJ_ERROR) */
    int ParseRequestKeypair(const std::string& sInput, gjDataList* paaOutput);
    int ParseRequestDump(const std::string& sInput, std::string* psOutput);
    //! @}

    /*! \name Clear Cache */
    //! @{
    /*! Delete all cached objects.
     *  \warning All external pointers will be invalid\n
     *           Try to avoid using this function */
    void ClearCache();
    //! @}

    /*! \name Utility Functions */
    //! @{
    static std::string UtilEscapeString(const std::string& sString);
    static void        UtilTrimString(std::string* psInput);
    static std::string UtilCharToHex(const char& cChar);
    static std::string UtilIntToString(const int& iInt);
    static void        UtilCreateFolder(const std::string& sFolder);
    static std::string UtilTimestamp(const time_t iTime = std::time(NULL));
    //! @}

    /*! \name Error Log */
    //! @{
    static              void                      ErrorLogReset();
    static              void                      ErrorLogAdd(const std::string& sMsg);
    static inline const std::vector<std::string>& ErrorLogGet(){return s_asLog;}
    //! @}

    /*! \name Set Attributes */
    //! @{
    inline void SetSessionActive(const bool& bActive) {m_bActive = bActive;}   //!< \copybrief m_bActive
    /*! */ //! @}

    /*! \name Get Attributes */
    //! @{
    inline const int&         GetGameID()const         {return m_iGameID;}           //!< \copybrief m_iGameID
    inline const std::string& GetGamePrivateKey()const {return m_sGamePrivateKey;}   //!< \copybrief m_sGamePrivateKey
    inline const std::string& GetUserName()const       {return m_sUserName;}         //!< \copybrief m_sUserName
    inline const std::string& GetUserToken()const      {return m_sUserToken;}        //!< \copybrief m_sUserToken
    /*! */ //! @}

    /*! \name Get Processed Attributes */
    //! @{
    inline const std::string& GetProcGameID()const    {return m_sProcGameID;}      //!< \copybrief m_sProcGameID
    inline const std::string& GetProcUserName()const  {return m_sProcUserName;}    //!< \copybrief m_sProcUserName
    inline const std::string& GetProcUserToken()const {return m_sProcUserToken;}   //!< \copybrief m_sProcUserToken
    /*! */ //! @}

    /*! \name Check Status */
    //! @{
    inline const bool& IsSessionActive()const {return m_bActive;}      //!< \copybrief m_bActive
    inline const bool& IsUserConnected()const {return m_bConnected;}   //!< \copybrief m_bConnected
    //! @}

#if !defined(DOXYGEN_SHOULD_SKIP_THIS)

    /*! \name Callback Placeholder */
    //! @{
                          inline int  Null(const std::string& sData, void* pAdd, std::string* psOutput) {if(psOutput) (*psOutput) = sData; return GJ_OK;}
    template <typename D> inline void Null(const D& pObject, void* pData)                               {}
    //! @}

#endif


private:
    DISABLE_COPY(gjAPI)

    /*! \name Session Functions */
    //! @{
    int __OpenSession();
    int __PingSession(const bool& bActive);
    int __CloseSession();
    //! @}

    /*! \name Superior Request Functions */
    //! @{
    template <typename T> int __Login(const bool& bSession, const std::string& sUserName, const std::string& sUserToken, const bool& bNow, GJ_NETWORK_OUTPUT(int));
    template <typename T> int __Login(const bool& bSession, const std::string& sCredPath, const bool& bNow, GJ_NETWORK_OUTPUT(int));
    //! @}

    /*! \name Callback Functions */
    //! @{
    int __LoginCallback(const std::string& sData, void* pAdd, int* pbOutput);
    //! @}
};


// ****************************************************************
/* fetch and cache a specific user with user ID */
template <typename T> int gjAPI::gjInterUser::__FetchUser(const int& iID, gjUserPtr* ppOutput, GJ_NETWORK_OUTPUT(gjUserPtr))
{
    const bool bNow = ppOutput ? true : false;

    // fetch current main user
    if(!iID) return this->__FetchUser(m_pAPI->GetUserName(), ppOutput, GJ_NETWORK_OUTPUT_FW);

    // check for cached user
    gjUserPtr pCache = NULL;
    if(this->__CheckCache(iID, &pCache) == GJ_OK)
    {
        if(bNow) (*ppOutput) = pCache;
        else (pOutputObj->*OutputCallback)(pCache, pOutputData);
        return GJ_OK;
    }

    // send get user request
    std::string sResponse;
    if(m_pNetwork->SendRequest("/users/"
                               "?game_id=" + m_pAPI->GetProcGameID() +
                               "&user_id=" + gjAPI::UtilIntToString(iID),
                               bNow ? &sResponse : NULL, this, &gjAPI::gjInterUser::__Process, NULL, GJ_NETWORK_OUTPUT_FW)) return GJ_REQUEST_FAILED;

    if(bNow) return this->__Process(sResponse, NULL, ppOutput);
    return GJ_OK;
}


// ****************************************************************
/* fetch and cache a specific user with user name */
template <typename T> int gjAPI::gjInterUser::__FetchUser(const std::string& sName, gjUserPtr* ppOutput, GJ_NETWORK_OUTPUT(gjUserPtr))
{
    if(sName == "") return GJ_INVALID_INPUT;

    const bool bNow = ppOutput ? true : false;

    // check for cached user
    gjUserPtr pCache = NULL;
    if(this->__CheckCache(sName, &pCache) == GJ_OK)
    {
        if(bNow) (*ppOutput) = pCache;
        else (pOutputObj->*OutputCallback)(pCache, pOutputData);
        return GJ_OK;
    }

    // send get user request
    std::string sResponse;
    if(m_pNetwork->SendRequest("/users/"
                               "?game_id="  + m_pAPI->GetProcGameID() +
                               "&username=" + gjAPI::UtilEscapeString(sName),
                               bNow ? &sResponse : NULL, this, &gjAPI::gjInterUser::__Process, NULL, GJ_NETWORK_OUTPUT_FW)) return GJ_REQUEST_FAILED;

    if(bNow) return this->__Process(sResponse, NULL, ppOutput);
    return GJ_OK;
}


// ****************************************************************
/* fetch and cache all trophies */
template <typename T> int gjAPI::gjInterTrophy::__FetchTrophies(const long& iAchieved, gjTrophyList* papOutput, GJ_NETWORK_OUTPUT(gjTrophyList))
{
    if(!m_pAPI->IsUserConnected() && m_iCache == 0) return GJ_NOT_CONNECTED;

    const bool bNow = papOutput ? true : false;

    // handle offline-cached trophies
    if(m_pAPI->IsUserConnected() && m_iCache != 2)
        m_iCache = 0;

    // wait for prefetching
    if(bNow && GJ_API_PREFETCH)
    {
        if(m_apTrophy.size() <= 1)
            m_pNetwork->Wait(1);
    }

    if(m_iCache != 0)
    {
        // check for cached trophies
        gjTrophyList apCache;
        if(this->__CheckCache(iAchieved, &apCache) == GJ_OK)
        {
            if(bNow) (*papOutput) = apCache;
            else (pOutputObj->*OutputCallback)(apCache, pOutputData);
            return GJ_OK;
        }
    }

    // send get trophies request
    std::string sResponse;
    if(m_pNetwork->SendRequest("/trophies/"
                               "?game_id="    + m_pAPI->GetProcGameID()   +
                               "&username="   + m_pAPI->GetProcUserName() +
                               "&user_token=" + m_pAPI->GetProcUserToken(),
                               bNow ? &sResponse : NULL, this, &gjAPI::gjInterTrophy::__Process, I_TO_P(iAchieved), GJ_NETWORK_OUTPUT_FW)) return GJ_REQUEST_FAILED;

    if(bNow) return this->__Process(sResponse, I_TO_P(iAchieved), papOutput);
    return GJ_OK;
}


// ****************************************************************
/* fetch and cache all score tables */
template <typename T> int gjAPI::gjInterScore::__FetchScoreTables(gjScoreTableMap* papOutput, GJ_NETWORK_OUTPUT(gjScoreTableMap))
{
    const bool bNow = papOutput ? true : false;

    // wait for prefetching
    if(bNow && GJ_API_PREFETCH)
    {
        if(m_apScoreTable.size() <= 1)
            m_pNetwork->Wait(1);
    }

    // check for cached score tables
    gjScoreTableMap apCache;
    if(this->__CheckCache(&apCache) == GJ_OK)
    {
        if(bNow) (*papOutput) = apCache;
        else (pOutputObj->*OutputCallback)(apCache, pOutputData);
        return GJ_OK;
    }

    // send get score tables request
    std::string sResponse;
    if(m_pNetwork->SendRequest("/scores/tables/"
                               "?game_id=" + m_pAPI->GetProcGameID(),
                               bNow ? &sResponse : NULL, this, &gjAPI::gjInterScore::__Process, NULL, GJ_NETWORK_OUTPUT_FW)) return GJ_REQUEST_FAILED;

    if(bNow) return this->__Process(sResponse, NULL, papOutput);
    return GJ_OK;
}


// ****************************************************************
/* fetch and semi-cache all data store items */
template <typename T> int gjAPI::gjInterDataStore::__FetchDataItems(gjDataItemMap* papOutput, GJ_NETWORK_OUTPUT(gjDataItemMap))
{
    if(!m_pAPI->IsUserConnected() && m_iType) return GJ_NOT_CONNECTED;

    const bool bNow = papOutput ? true : false;

    if(m_iType)
    {
        // wait for prefetching
        if(bNow && GJ_API_PREFETCH)
        {
            if(m_apDataItem.empty())
                m_pNetwork->Wait(1);
        }

        // check for cached data store items
        gjDataItemMap apCache;
        if(this->__CheckCache(&apCache) == GJ_OK)
        {
            if(bNow) (*papOutput) = apCache;
            else (pOutputObj->*OutputCallback)(apCache, pOutputData);
            return GJ_OK;
        }
    }

    // access user or global data store items
    const std::string sUserData = m_iType ?
                                  "&username="   + m_pAPI->GetProcUserName()  +
                                  "&user_token=" + m_pAPI->GetProcUserToken() :
                                  "";

    // send get data store item keys request
    std::string sResponse;
    if(m_pNetwork->SendRequest("/data-store/get-keys/"
                               "?game_id=" + m_pAPI->GetProcGameID() +
                               sUserData, bNow ? &sResponse : NULL, this, &gjAPI::gjInterDataStore::__Process, NULL, GJ_NETWORK_OUTPUT_FW)) return GJ_REQUEST_FAILED;

    if(bNow) return this->__Process(sResponse, NULL, papOutput);
    return GJ_OK;
}


// ****************************************************************
/* download a file from any URL */
template <typename T> int gjAPI::gjInterFile::__DownloadFile(const std::string& sURL, const std::string& sToFolder, const std::string& sFileNameOverwrite, std::string* psOutput, GJ_NETWORK_OUTPUT(std::string))
{
    if(sURL == "" || sToFolder == "") return GJ_INVALID_INPUT;

    const bool bNow = psOutput ? true : false;

    // create output path
    const std::string sFileName = (sFileNameOverwrite == "") ? sURL.substr(sURL.find_last_of("/\\")+1) : sFileNameOverwrite;
    const std::string sToFile   = sToFolder + "/" + sFileName;

    // check for cached file
    if(this->__CheckCache(sToFile) == GJ_OK)
    {
        if(bNow) (*psOutput) = sToFile;
        else (pOutputObj->*OutputCallback)(sToFile, pOutputData);
        return GJ_OK;
    }

    // create folder
    gjAPI::UtilCreateFolder(sToFolder);

    // download file
    if(m_pNetwork->DownloadFile(sURL, sToFile, psOutput, this, &gjAPI::gjInterFile::__Process, NULL, GJ_NETWORK_OUTPUT_FW)) return GJ_REQUEST_FAILED;

    if(bNow) this->__Process(*psOutput, NULL, NULL);
    return GJ_OK;
}


// ****************************************************************
/* login with specific user */
template <typename T> int gjAPI::__Login(const bool& bSession, const std::string& sUserName, const std::string& sUserToken, const bool& bNow, GJ_NETWORK_OUTPUT(int))
{
    if(this->IsUserConnected()) return GJ_INVALID_CALL;

    // check for missing credentials
    if(sUserName == "" || sUserToken == "")
    {
        if(!bNow) (pOutputObj->*OutputCallback)(GJ_INVALID_INPUT, pOutputData);
        return GJ_INVALID_INPUT;
    }

    // set main user data
    m_sUserName      = sUserName;
    m_sUserToken     = sUserToken;
    m_sProcUserName  = gjAPI::UtilEscapeString(m_sUserName);
    m_sProcUserToken = gjAPI::UtilEscapeString(m_sUserToken);

    // convert session parameter
    void* pSession = I_TO_P(bSession ? 1 : 0);

    // authenticate user
    std::string sResponse;
    if(m_pNetwork->SendRequest("/users/auth/"
                               "?game_id="    + m_sProcGameID   +
                               "&username="   + m_sProcUserName +
                               "&user_token=" + m_sProcUserToken,
                               bNow ? &sResponse : NULL, this, &gjAPI::__LoginCallback, pSession, GJ_NETWORK_OUTPUT_FW)) return GJ_REQUEST_FAILED;

    if(bNow) return this->__LoginCallback(sResponse, pSession, NULL);
    return GJ_OK;
}

template <typename T> int gjAPI::__Login(const bool& bSession, const std::string& sCredPath, const bool& bNow, GJ_NETWORK_OUTPUT(int))
{
    // open credentials file
    std::FILE* pFile = std::fopen(sCredPath.c_str(), "rb");
    if(!pFile) return GJ_FILE_ERROR;

    char acName[128], acToken[128];
    char* pcEnd;

    // get user name
    std::fscanf(pFile, "%127[^\n]%*c", acName);
    pcEnd = std::strchr(acName, 13);
    if(pcEnd) *pcEnd = '\0';

    // get user token
    std::fscanf(pFile, "%127[^\n]%*c", acToken);
    pcEnd = std::strchr(acToken, 13);
    if(pcEnd) *pcEnd = '\0';

    // close file and login
    std::fclose(pFile);
    return this->__Login(bSession, acName, acToken, bNow, GJ_NETWORK_OUTPUT_FW);
}


// ****************************************************************
/* post-include because of generic dependencies */
#include "gjNetwork.hpp"
#include "gjUser.h"
#include "gjTrophy.h"
#include "gjScore.h"
#include "gjDataItem.h"


#endif /* _GJ_GUARD_API_H_ */