///////////////////////////////////////////////////////////////////
//*-------------------------------------------------------------*//
//| Part of the Game Jolt API C++ Library (http://gamejolt.com) |//
//*-------------------------------------------------------------*//
//| Released under the zlib License                             |//
//| More information available in the readme file               |//
//*-------------------------------------------------------------*//
///////////////////////////////////////////////////////////////////
#pragma once
#ifndef _GJ_GUARD_NETWORK_HPP_
#define _GJ_GUARD_NETWORK_HPP_


// ****************************************************************
/* constructor */
template <typename P, typename D> gjNetwork::gjCallTemplate<P,D>::gjCallTemplate(CURL* pSession, const std::string& sInfo, GJ_NETWORK_PROCESS)noexcept
: gjCall            (pSession, sInfo)
, m_pProcessObj     (pProcessObj)
, m_ProcessCallback (ProcessCallback)
, m_pProcessData    (pProcessData)
{
    // reserve some memory
    m_apOutput.reserve(GJ_API_RESERVE_CALL_OUTPUT);
}


// ****************************************************************
/* destructor */
template <typename P, typename D> gjNetwork::gjCallTemplate<P,D>::~gjCallTemplate()
{
    // delete all output structs
    FOR_EACH(it, m_apOutput) 
        SAFE_DELETE(*it) 
        
    // clear memory
    m_apOutput.clear();
}


// ****************************************************************
/* add output struct to the list */
template <typename P, typename D> template <typename T> void gjNetwork::gjCallTemplate<P,D>::AddOutput(GJ_NETWORK_OUTPUT(D))
{
    // create struct
    sOutputSpecific<T,D>* pOutput = new sOutputSpecific<T,D>;
    pOutput->m_pOutputObj     = pOutputObj;
    pOutput->m_OutputCallback = OutputCallback;
    pOutput->m_pOutputData    = pOutputData;

    // add struct
    m_apOutput.push_back(pOutput);
}


// ****************************************************************
/* finish a request session */
template <typename P, typename D> void gjNetwork::gjCallRequest<P,D>::Finish(const bool& bOK)
{
    //if(bOK)
    {
        D pProcessedOutput;

#if defined(_GJ_DEBUG_)

            // show current activity
            gjAPI::ErrorLogAdd("SendRequest.Finish: <" + this->m_sInfo + ">\n(\n" + *m_psResponse +")");

#endif
        // process the response string
        if(!(this->m_pProcessObj->*this->m_ProcessCallback)(*m_psResponse, this->m_pProcessData, &pProcessedOutput))
        {
            // call all callbacks
            FOR_EACH(it, this->m_apOutput)
                (*it)->Execute(pProcessedOutput);
        }
    }

    // delete response string
    SAFE_DELETE(m_psResponse)

    // clear POST data
    if(m_pPostList) curl_formfree(m_pPostList);
}


// ****************************************************************
/* finish a download session */
template <typename P, typename D> void gjNetwork::gjCallDownload<P,D>::Finish(const bool& bOK)
{
    // close file handle
    std::fclose(m_pFile);

    //if(bOK)
    {
        D pProcessedOutput;

#if defined(_GJ_DEBUG_)

            // show current activity
            gjAPI::ErrorLogAdd("DownloadFile.Finish: <" + this->m_sInfo + ">");

#endif
        // process the response string
        if(!(this->m_pProcessObj->*this->m_ProcessCallback)(m_sPath, this->m_pProcessData, &pProcessedOutput))
        {
            // call all callbacks
            FOR_EACH(it, this->m_apOutput)
                (*it)->Execute(pProcessedOutput);
        }
    }
}


// ****************************************************************
/* send direct or non-blocking request */
template <typename T, typename P, typename D> int gjNetwork::SendRequest(const std::string& sURL, std::string* psOutput, GJ_NETWORK_PROCESS, GJ_NETWORK_OUTPUT(D))noexcept
{
    if(sURL == "") return GJ_INVALID_INPUT;

    const size_t iPostPos = sURL.find("&POST");
    const bool bPost      = (iPostPos != std::string::npos) ? true : false;
    const bool bHttp      = (int(sURL.substr(0, 8).find("://")) >= 0) ? true : false;

    const bool bNow         = psOutput ? true : false;
    const std::string sInfo = bPost ? sURL.substr(0, iPostPos) : sURL;

    if(!bNow)
    {
        // check for existing session with same request string
        gjCall* pOldCall = this->__CheckCall(sInfo);
        if(pOldCall)
        {
            if(bPost)
            {
                // remove old session
                this->__KillCall(pOldCall);
            }
            else
            {
                // append new callback to old session
                gjCallRequest<P,D>* pOldCallRequest = (gjCallRequest<P,D>*)pOldCall;
                pOldCallRequest->AddOutput(GJ_NETWORK_OUTPUT_FW);
                return GJ_OK;
            }
        }
    }

    // open cURL session
    CURL* pSession = curl_easy_init();
    if(pSession)
    {
        std::string sRequest;

        curl_httppost* pPostList = NULL;
        curl_httppost* pEndList  = NULL;

        // create new response string for non-blocking session
        if(!bNow) psOutput = new std::string();

        // check for POST parameter
        if(bPost)
        {
            // set POST data
            curl_formadd(&pPostList, &pEndList,
                         CURLFORM_COPYNAME,     "data",
                         CURLFORM_COPYCONTENTS, sURL.substr(iPostPos+5).c_str(),
                         CURLFORM_END);

            // create a POST request
            sRequest = (bHttp ? "" : GJ_API_URL) + sURL.substr(0, iPostPos);
            curl_easy_setopt(pSession, CURLOPT_POST,     1);
            curl_easy_setopt(pSession, CURLOPT_HTTPPOST, pPostList);
        }
        else
        {
            // create a GET request
            sRequest = (bHttp ? "" : GJ_API_URL) + sURL;
            curl_easy_setopt(pSession, CURLOPT_TIMEOUT, GJ_API_TIMEOUT_REQUEST);
        }

        // add MD5 signature
        sRequest += "&signature=" + md5(sRequest + m_pAPI->GetGamePrivateKey());

#if defined(_GJ_DEBUG_)

        // show current activity
        gjAPI::ErrorLogAdd("SendRequest.Execute: <" + sRequest + ">");

#endif
        // set all session parameters
        curl_easy_setopt(pSession, CURLOPT_URL,             sRequest.c_str());
        curl_easy_setopt(pSession, CURLOPT_WRITEFUNCTION,   write_to_string);
        curl_easy_setopt(pSession, CURLOPT_WRITEDATA,       psOutput);
        curl_easy_setopt(pSession, CURLOPT_CONNECTTIMEOUT,  GJ_API_TIMEOUT_CONNECTION);
        curl_easy_setopt(pSession, CURLOPT_ACCEPT_ENCODING, GJ_API_NET_COMPRESSION);
        curl_easy_setopt(pSession, CURLOPT_TCP_KEEPALIVE,   GJ_API_NET_KEEPALIVE ? 1 : 0);

        if(bNow)
        {
            // perform request and close cURL session
            CURLcode res = curl_easy_perform(pSession);
            curl_easy_cleanup(pSession);

            // clear POST data
            if(pPostList) curl_formfree(pPostList);

            // check for errors
            if(res != CURLE_OK)
            {
                gjAPI::ErrorLogAdd("Network Error: sending direct request failed <" + sInfo + ">");
                gjAPI::ErrorLogAdd("Network Error: " + std::string(curl_easy_strerror(res)));
                return GJ_REQUEST_FAILED;
            }
        }
        else
        {
            // append session to multi handle
            curl_multi_add_handle(m_pMultiHandle, pSession);
            curl_multi_perform(m_pMultiHandle, &m_iNumSessions);

            // create and save callback object
            gjCallRequest<P,D>* pCall = new gjCallRequest<P,D>(psOutput, pPostList, pSession, sInfo, GJ_NETWORK_PROCESS_FW);
            pCall->AddOutput(GJ_NETWORK_OUTPUT_FW);
            m_apCall.push_back(pCall);
        }
    }
    else
    {
        gjAPI::ErrorLogAdd("Network Error: cannot establish curl session");
        return GJ_NETWORK_ERROR;
    }

    return GJ_OK;
}


// ****************************************************************
/* download file direct or non-blocking */
template <typename T, typename P, typename D> int gjNetwork::DownloadFile(const std::string& sURL, const std::string& sToFile, std::string* psOutput, GJ_NETWORK_PROCESS, GJ_NETWORK_OUTPUT(D))noexcept
{
    if(sURL == "" || sToFile == "") return GJ_INVALID_INPUT;

    const bool bNow = psOutput ? true : false;
    const std::string sInfo = sURL;

    if(!bNow)
    {
        // check for existing session with same request string
        gjCall* pOldCall = this->__CheckCall(sInfo);
        if(pOldCall)
        {
            // append new callback to old session
            gjCallDownload<P,D>* pOldCallDownload = (gjCallDownload<P,D>*)pOldCall;
            pOldCallDownload->AddOutput(GJ_NETWORK_OUTPUT_FW);
            return GJ_OK;
        }
    }

    // open cURL session
    CURL* pSession = curl_easy_init();
    if(pSession)
    {
        // open file
        std::FILE* pFile = std::fopen(sToFile.c_str(), "wb");
        if(pFile)
        {
#if defined(_GJ_DEBUG_)

            // show current activity
            gjAPI::ErrorLogAdd("DownloadFile.Execute: <" + sURL + ">");

#endif
            // set all session parameters
            curl_easy_setopt(pSession, CURLOPT_URL,             sURL.c_str());
            curl_easy_setopt(pSession, CURLOPT_WRITEFUNCTION,   write_to_file);
            curl_easy_setopt(pSession, CURLOPT_WRITEDATA,       pFile);
            curl_easy_setopt(pSession, CURLOPT_CONNECTTIMEOUT,  GJ_API_TIMEOUT_CONNECTION);
            curl_easy_setopt(pSession, CURLOPT_ACCEPT_ENCODING, GJ_API_NET_COMPRESSION);
            curl_easy_setopt(pSession, CURLOPT_TCP_KEEPALIVE,   GJ_API_NET_KEEPALIVE ? 1 : 0);

            if(bNow)
            {
                // perform download and close cURL session
                CURLcode res = curl_easy_perform(pSession);
                curl_easy_cleanup(pSession);

                // close file
                std::fclose(pFile);

                // check for errors
                if(res != CURLE_OK)
                {
                    gjAPI::ErrorLogAdd("Network Error: direct file download failed <" + sInfo + ">");
                    gjAPI::ErrorLogAdd("Network Error: " + std::string(curl_easy_strerror(res)));
                    return GJ_REQUEST_FAILED;
                }

                (*psOutput) = sToFile;
            }
            else
            {
                // append session to multi handle
                curl_multi_add_handle(m_pMultiHandle, pSession);
                curl_multi_perform(m_pMultiHandle, &m_iNumSessions);

                // create and save callback object
                gjCallDownload<P,D>* pCall = new gjCallDownload<P,D>(pFile, sToFile, pSession, sInfo, GJ_NETWORK_PROCESS_FW);
                pCall->AddOutput(GJ_NETWORK_OUTPUT_FW);
                m_apCall.push_back(pCall);
            }
        }
        else
        {
            gjAPI::ErrorLogAdd("File Error: cannot write file <" + sToFile + ">");
            curl_easy_cleanup(pSession);
            return GJ_FILE_ERROR;
        }
    }
    else
    {
        gjAPI::ErrorLogAdd("Network Error: cannot establish curl session");
        return GJ_NETWORK_ERROR;
    }

    return GJ_OK;
}


#endif /* _GJ_GUARD_NETWORK_HPP_ */