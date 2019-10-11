#pragma once

#include "url_canon.h"

namespace
{
    class URLRequestImpl;
}

namespace mctm
{
    class URLResponseInfo
    {
    public:
        long response_code() const
        {
            return response_code_;
        }

        const std::string& response_header() { return response_header_; }
        const std::string& response_data() { return response_data_; }

        long response_code_ = 0;
        std::string response_header_;
        std::string response_data_;
    };

    class HttpResponseInfo : public URLResponseInfo
    {
    public:
    protected:
    private:
    };

    class URLRequestStatus
    {
    public:
        enum class Status
        {
            // Request succeeded, |error_| will be 0.
            SUCCESS = 0,

            // An IO request is pending, and the caller will be informed when it is
            // completed.
            IO_PENDING,

            // Request was cancelled programatically.
            CANCELED,

            // The request failed for some reason. |error_| may have more information.
            FAILED,
        };

        URLRequestStatus() : status_(Status::SUCCESS), error_(0) {}
        URLRequestStatus(Status s, int e) : status_(s), error_(e) {}

        Status status() const { return status_; }
        void set_status(Status s) { status_ = s; }

        int error() const { return error_; }
        void set_error(int e) { error_ = e; }

        // Returns true if the status is success, which makes some calling code more
        // convenient because this is the most common test.
        bool is_success() const
        {
            return status_ == Status::SUCCESS || status_ == Status::IO_PENDING;
        }

        // Returns true if the request is waiting for IO.
        bool is_io_pending() const
        {
            return status_ == Status::IO_PENDING;
        }

    private:
        // Application level status.
        Status status_ = Status::SUCCESS;

        // Error code from the network layer if an error was encountered.
        int error_ = 0;
    };

    class URLRequestContext;
    class URLRequest
    {
    public:
        class Delegate
        {
        public:
        protected:
        private:
        };

        URLRequest(const CanonURL& url,
            Delegate* delegate,
            const URLRequestContext* context);
        virtual ~URLRequest();

        bool Start();
        void Cancel();
        void CancelWithError(int error);
        void Restart();

        const CanonURL& url() const { return url_; }
        const std::string& method() const { return method_; }
        void set_method(const std::string& method) { method_ = method; }

    private:
        // invoke by URLRequestImpl
        void OnRequestFailed(int err_code, const char* err_msg);
        void OnRequestCompleted(int http_status, const URLResponseInfo* rsp_info);
        //void OnRequestSucceeded(int err_code, const char* err_msg);

    private:
        friend class URLRequestImpl;

        CanonURL url_;
        Delegate* delegate_ = nullptr;
        const URLRequestContext* context_ = nullptr;
        std::unique_ptr<URLRequestImpl> request_;
        std::string method_;  // "GET", "POST", etc. Should be all uppercase.
        URLRequestStatus status_;
    };
}