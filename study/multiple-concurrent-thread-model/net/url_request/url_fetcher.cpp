#include "url_fetcher.h"

#include "logging/logging.h"
#include "message_loop/message_loop.h"


namespace mctm
{
    URLFetcher::URLFetcher(const CanonURL& url, URLFetcher::RequestType request_type, URLFetcherDelegate* delegate)
        : url_(url)
        , request_type_(request_type)
        , delegate_(delegate)
    {
    }

    URLFetcher::~URLFetcher()
    {
    }

    void URLFetcher::SetRequestContext(URLRequestContext* request_context)
    {
        request_context_ = request_context;
    }

    void URLFetcher::SetNetworkTaskRunner(SingleThreadTaskRunner network_task_runner)
    {
        network_task_runner_ = network_task_runner;
    }

    bool URLFetcher::Start()
    {
        DCHECK(MessageLoop::current());
        if (!MessageLoop::current())
        {
            return false;
        }

        delegate_task_runner_ = MessageLoop::current()->shared_from_this();

        DCHECK(network_task_runner_);
        if (!network_task_runner_)
        {
            return false;
        }
        
        network_task_runner_->PostTask(FROM_HERE, Bind(&URLFetcher::StartOnIOThread, weak_from_this()));
        return true;
    }

    void URLFetcher::Stop()
    {
        if (request_)
        {
            request_->Cancel();
        }
    }

    void URLFetcher::SetUploadData(const std::string & upload_content_type, const std::string & upload_content)
    {
    }

    void URLFetcher::SetUploadFilePath(const std::string& upload_content_type, 
        const std::wstring& file_path, unsigned __int64 range_offset, unsigned __int64 range_length
        /*, scoped_refptr<base::TaskRunner> file_task_runner*/)
    {
    }

    void URLFetcher::StartOnIOThread()
    {
        DCHECK(request_context_);
        DCHECK(!request_);

        request_ = request_context_->CreateURLRequest(url_, this);
        if (request_->Start())
        {

        }
    }

}