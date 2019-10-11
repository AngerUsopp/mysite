#include "url_request.h"

#include <algorithm>
#include <atomic>
#include <tuple>

#include "logging/logging.h"
#include "third_party/libcurl/include/curl/curl.h"

#if LIBCURL_VERSION_NUM >= 0x073d00
/* In libcurl 7.61.0, support was added for extracting the time in plain
   microseconds. Older libcurl versions are stuck in using 'double' for this
   information so we complicate this example a bit by supporting either
   approach. */
#define TIME_IN_US 1  
#define TIMETYPE curl_off_t
#define TIMEOPT CURLINFO_TOTAL_TIME_T
#define MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL     3000000
#else
#define TIMETYPE double
#define TIMEOPT CURLINFO_TOTAL_TIME
#define MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL     3
#endif

namespace
{
    class CURLInitializer
    {
    public:
        CURLInitializer()
        {
            CURLcode ret = curl_global_init(CURL_GLOBAL_ALL);
            DCHECK(CURLE_OK == ret);
        }

        ~CURLInitializer()
        {
            curl_global_cleanup();
        }
    };
    CURLInitializer curl_init_obj;

    class URLRequestImpl
    {
    public:
        explicit URLRequestImpl(mctm::URLRequest* request)
            : request_(request)
        {
            Init();
        }

        virtual ~URLRequestImpl()
        {
            Uninit();
        }

        bool SetRequestTimeout(long timeout_in_ms)
        {
            if (!curl_ || do_request_now_)
            {
                return false;
            }

            return (CURLE_OK == curl_easy_setopt(curl_, CURLOPT_TIMEOUT_MS, timeout_in_ms));
        }

        bool SetConnectTimeout(long timeout_in_ms)
        {
            if (!curl_ || do_request_now_)
            {
                return false;
            }

            return (CURLE_OK == curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT_MS, timeout_in_ms));
        }

        bool SetVerbose(bool onoff)
        {
            if (!curl_ || do_request_now_)
            {
                return false;
            }

            return (CURLE_OK == curl_easy_setopt(curl_, CURLOPT_VERBOSE, onoff ? 1 : 0));
        }

        bool SetRedirect(bool enable)
        {
            if (!curl_ || do_request_now_)
            {
                return false;
            }

            return (CURLE_OK == curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, enable ? 1 : 0));
        }

        bool SetUrl(const char* url)
        {
            if (!curl_ || do_request_now_)
            {
                return false;
            }

            return (CURLE_OK == curl_easy_setopt(curl_, CURLOPT_URL, url));
        }

        bool SetHeader(const char* header_item)
        {
            if (!curl_ || do_request_now_)
            {
                return false;
            }

            header_ = curl_slist_append(header_, header_item);
            return !!header_;
        }

        bool SetData(const std::string* data)
        {
            if (do_request_now_)
            {
                return false;
            }

            std::get<0>(request_data_) = data;
            std::get<1>(request_data_) = 0;
            return true;
        }

        bool DoRequest()
        {
            if (curl_ && !do_request_now_)
            {
                CURLcode ret = CURLE_OK;

                if (header_)
                {
                    ret = curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, header_);
                    if (ret != CURLE_OK)
                    {
                        return false;
                    }
                }

                do_request_now_ = true;
                ret = curl_easy_perform(curl_);
                do_request_now_ = false;
                if (ret != CURLE_OK)
                {
                    const char * err_msg = curl_easy_strerror(ret);

                    // callback
                    request_->OnRequestFailed(ret, err_msg);

                    return false;
                }

                ret = curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &response_info_.response_code_);
                if (ret != CURLE_OK)
                {
                    return false;
                }

                // callback
                request_->OnRequestCompleted(response_info_.response_code(), &response_info_);

                return true;
            }
            return false;
        }

        bool GetResponseCode(long* code)
        {
            if (!curl_ || do_request_now_)
            {
                return false;
            }

            if (!code)
            {
                return false;
            }

            *code = response_info_.response_code();
            return true;
        }

        bool GetResponseHeaderSize(long* header_size)
        {
            if (!curl_ || do_request_now_)
            {
                return false;
            }

            if (!header_size)
            {
                return false;
            }

            *header_size = -1;
            return (CURLE_OK == curl_easy_getinfo(curl_, CURLINFO_HEADER_SIZE, header_size));
        }

        bool GetResponseHeader(const std::string** const response_header)
        {
            if (!curl_ || do_request_now_)
            {
                return false;
            }

            if (!response_header)
            {
                return false;
            }

            *response_header = &response_info_.response_header();
            return true;
        }

        bool GetResponseData(const std::string** const response_data)
        {
            if (!curl_ || do_request_now_)
            {
                return false;
            }

            if (!response_data)
            {
                return false;
            }

            *response_data = &response_info_.response_data();
            return true;
        }

    private:
        bool Init()
        {
            if (!curl_)
            {
                curl_ = curl_easy_init();
                if (curl_)
                {
                    //curl_easy_setopt(curl_, CURLOPT_USERAGENT, "libcurl-agent/1.0");

                    curl_easy_setopt(curl_, CURLOPT_READFUNCTION, OnRequestSendDataFunction);
                    curl_easy_setopt(curl_, CURLOPT_READDATA, &request_data_);

                    curl_easy_setopt(curl_, CURLOPT_HEADERFUNCTION, OnResponseHeaderFunction);
                    curl_easy_setopt(curl_, CURLOPT_HEADERDATA, &response_header_);

                    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, OnResponseRecvDataFunction);
                    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response_data_);

                    curl_easy_setopt(curl_, CURLOPT_NOPROGRESS, 0);
#if LIBCURL_VERSION_NUM >= 0x072000
                    curl_easy_setopt(curl_, CURLOPT_XFERINFOFUNCTION, OnProgressFunction);
                    curl_easy_setopt(curl_, CURLOPT_XFERINFODATA, 0);
#else
                    curl_easy_setopt(curl_, CURLOPT_PROGRESSFUNCTION, OnProgressFunction);
                    curl_easy_setopt(curl_, CURLOPT_PROGRESSDATA, 0);
#endif
                }

                return !!curl_;
            }
            return false;
        }

        void Uninit()
        {
            DCHECK(!do_request_now_);;

            if (header_)
            {
                curl_slist_free_all(header_);
                header_ = nullptr;
            }
            if (curl_)
            {
                curl_easy_cleanup(curl_);
                curl_ = nullptr;
            }

            std::tuple<const std::string*, size_t> ept{};
            request_data_.swap(ept);
            response_header_.clear();
            response_data_.clear();
            response_code_ = 0;
        }

        // static callback
        static size_t OnRequestSendDataFunction(char *buffer, size_t size, size_t nitems, void *userdata)
        {
            std::tuple<const std::string*, int>* send_data = 
                reinterpret_cast<std::tuple<const std::string*, int>*>(userdata);
            size_t at_most = size * nitems;
            size_t total = std::min(at_most, std::get<0>(*send_data)->length() - std::get<1>(*send_data));

            std::copy_n(std::get<0>(*send_data)->data() + std::get<1>(*send_data), total, buffer);
            std::get<1>(*send_data) += total;

            // return CURL_READFUNC_ABORT;

            return total;
        }

        static size_t OnResponseHeaderFunction(char *buffer, size_t size, size_t nitems, void *userdata)
        {
            size_t total = 0;
            std::string* response_header = reinterpret_cast<std::string*>(userdata);
            if (response_header)
            {
                total = size * nitems;
                response_header->append(reinterpret_cast<const char*>(buffer), total);
            }
            return total;
        }

        static size_t OnResponseRecvDataFunction(char *ptr, size_t size, size_t nmemb, void *userdata)
        {
            size_t total = 0;
            std::string* response_data = reinterpret_cast<std::string*>(userdata);
            if (response_data)
            {
                total = size * nmemb;
                response_data->append(ptr, total);
            }

            //return 0;

            return total;
        }

        static int OnProgressFunction(void *userdata, double dltotal, double dlnow, double ultotal, double ulnow)
        {
            // download: dltotal¡¢dlnow
            // upload: ultotal¡¢ulnow
            //TIMETYPE curtime = 0;
            //curl_easy_getinfo(curl, TIMEOPT, &curtime);

            // return -1;
            return 0;
        }

    private:
        mctm::URLRequest* request_ = nullptr;
        CURL *curl_ = nullptr;
        curl_slist* header_ = nullptr;
        std::tuple<const std::string*, size_t> request_data_;

        mctm::URLResponseInfo response_info_;

        std::atomic_bool do_request_now_ = false;
    };
}

namespace mctm
{
    URLRequest::URLRequest(const CanonURL& url, Delegate* delegate, const URLRequestContext* context)
        : url_(url)
        , delegate_(delegate)
        , context_(context)
    {
        request_ = std::make_unique<URLRequestImpl>(this);
        request_->SetUrl(url_.spec().c_str());
    }

    URLRequest::~URLRequest()
    {
    }

    bool URLRequest::Start()
    {
        if (status_.is_io_pending())
        {
            return false;
        }

        if (delegate_)
        {
            //////////////////////////////////////////////////////////////////////////
        }
        status_.set_status(URLRequestStatus::Status::IO_PENDING);
        if (!request_->DoRequest())
        {
            return false;
        }

        long http_code = -1;
        if (!request_->GetResponseCode(&http_code))
        {
            return false;
        }

        return true;
    }

    void URLRequest::Cancel()
    {
    }

    void URLRequest::CancelWithError(int error)
    {
    }

    void URLRequest::Restart()
    {
    }

    void URLRequest::OnRequestFailed(int err_code, const char* err_msg)
    {
        DLOG(WARNING) << "url request failed, code = " << err_code << ", msg = " << err_msg;

        status_.set_status(URLRequestStatus::Status::FAILED);

        if (delegate_)
        {
            //////////////////////////////////////////////////////////////////////////
        }
    }

    void URLRequest::OnRequestCompleted(int http_status, const URLResponseInfo* rsp_info)
    {
        /*DLOG_IF(WARNING, http_status != 200) <<
            "http request failed, http_status = " << http_status;*/
        status_.set_status(URLRequestStatus::Status::SUCCESS);

        if (delegate_)
        {
            //////////////////////////////////////////////////////////////////////////
        }
    }

}