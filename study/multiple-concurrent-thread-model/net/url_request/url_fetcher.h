#pragma once

#include <memory>

#include "data_encapsulation/smart_pointer.h"
#include "net/url_request/url_request_context.h"

namespace mctm
{
    class URLFetcherDelegate
    {
    public:
        virtual ~URLFetcherDelegate() = default;
    };

    class URLRequest;
    class URLFetcher 
        : public URLRequest::Delegate
        , public std::enable_shared_from_this<URLFetcher>
    {
    public:
        enum RequestType
        {
            GET,
            POST,
            HEAD,
            DELETE_REQUEST,   // DELETE is already taken on Windows.
                              // <winnt.h> defines a DELETE macro.
            PUT,
            PATCH,
        };

        URLFetcher(const CanonURL& url,
            RequestType request_type,
            URLFetcherDelegate* delegate);
        virtual ~URLFetcher();

        void SetRequestContext(URLRequestContext* request_context);
        void SetNetworkTaskRunner(SingleThreadTaskRunner network_task_runner);

        bool Start();

        void Stop();

        void SetUploadData(const std::string& upload_content_type,
            const std::string& upload_content);
        void SetUploadFilePath(const std::string& upload_content_type,
            const std::wstring& file_path,
            unsigned __int64 range_offset,
            unsigned __int64 range_length/*,
            scoped_refptr<base::TaskRunner> file_task_runner*/);

    private:
        void StartOnIOThread();

    private:
        CanonURL url_;
        RequestType request_type_ = RequestType::GET;
        URLFetcherDelegate* delegate_ = nullptr;
        URLRequestContext* request_context_;
        std::unique_ptr<URLRequest> request_;   // The actual request this wraps

        bool upload_content_set_ = false;   // SetUploadData has been called
        std::string upload_content_type_;   // MIME type of POST payload
        std::string upload_content_;        // HTTP POST payload

        std::wstring upload_file_path_;  // Path to file containing POST payload
        unsigned __int64 upload_range_offset_ = 0;       // Offset from the beginning of the file to be uploaded.
        unsigned __int64 upload_range_length_ = 0;       // The length of the part of file to be uploaded.

        SingleThreadTaskRunner delegate_task_runner_;
        SingleThreadTaskRunner network_task_runner_;
        //SingleThreadTaskRunner file_task_runner_;
        //SingleThreadTaskRunner upload_file_task_runner_;
    };
}