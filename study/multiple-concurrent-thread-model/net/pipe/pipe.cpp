#include "pipe.h"

#include "logging/logging.h"
#include "message_loop/message_loop.h"

namespace
{
    static const size_t kMaximumMessageSize = 128 * 1024 * 1024;
}

namespace mctm
{
    PipeServer::PipeServer(const wchar_t* pipe_name, Delegate* delegate,
        unsigned int max_pipe_instances_count/* = 1*/)
        : pipe_name_(pipe_name)
        , delegate_(delegate)
        , max_pipe_instances_count_(max_pipe_instances_count)
    {
    }

    PipeServer::~PipeServer()
    {
    }

    bool PipeServer::Start()
    {
        auto loop = MessageLoopForIO::current();
        if (!loop)
        {
            return false;
        }

        for (auto i = 0; i < max_pipe_instances_count_; ++i)
        {
            PipeServer::ScopedClient client = Create();
            if (client)
            {
                if (loop->RegisterIOHandler(client->pipe_handle_, client.get()))
                {
                    if (Accept(client.get()))
                    {
                        clients_.push_back(std::move(client));
                    }
                }
            }
        }
        DCHECK(!clients_.empty());
        return !clients_.empty();
    }

    PipeServer::ScopedClient PipeServer::Create()
    {
        DWORD open_mode = PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED;
        if(clients_.empty())
        {
            open_mode |= FILE_FLAG_FIRST_PIPE_INSTANCE;
        }

        HANDLE pipe_handle = ::CreateNamedPipe(pipe_name_.c_str(),
            open_mode,
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
            max_pipe_instances_count_,
            kIOBufferSize,
            kIOBufferSize,
            5000,
            nullptr);
        if (pipe_handle == INVALID_HANDLE_VALUE)
        {
            DWORD err = ::GetLastError();
            DLOG(ERROR) << "PipeServer Create failed, code = " << err;
            return nullptr;
        }
        ScopedClient client = std::make_unique<ClientInfo>(pipe_handle, this);
        return client;
    }

    bool PipeServer::Accept(ClientInfo* client)
    {
        if (INVALID_HANDLE_VALUE != client->pipe_handle_)
        {
            bool ret = ::ConnectNamedPipe(client->pipe_handle_, &client->accept_io_context_.overlapped);
            if (ret)
            {
                // Uhm, the API documentation says that this function should never
                // return success when used in overlapped mode.
                NOTREACHED();
                return false;
            }

            DWORD err = ::GetLastError();
            switch (err)
            {
            case ERROR_IO_PENDING:
                ret = true;
                break;
            case ERROR_PIPE_CONNECTED:
                // 调用ConnectNamedPipe之前就已经连接成功了的client，是否还会走iocp的通知？要验证一下
                //////////////////////////////////////////////////////////////////////////
                ret = true;
                break;
            case ERROR_NO_DATA:
                // The pipe is being closed.
                break;
            default:
                NOTREACHED();
                break;
            }

            return ret;
        }
        return false;
    }

    bool PipeServer::Read(ClientInfo* client)
    {
        if ((INVALID_HANDLE_VALUE != client->pipe_handle_) && 
            (!client->accept_io_context_.is_pending_) &&
            (!client->read_io_context_.is_pending_))
        {
            BOOL ok = ::ReadFile(client->pipe_handle_, 
                client->read_io_context_.io_buffer.buffer, 
                kIOBufferSize,
                nullptr, 
                &client->read_io_context_.overlapped);
            if (!ok)
            {
                DWORD err = ::GetLastError();
                if (err == ERROR_IO_PENDING)
                {
                    return true;
                }
                DLOG(ERROR) << "pipe read error: " << err;
                return false;
            }
            return true;
        }
        return false;
    }

    bool PipeServer::Write(ClientInfo* client, const char* data, unsigned int len)
    {
        if ((INVALID_HANDLE_VALUE != client->pipe_handle_) &&
            (!client->accept_io_context_.is_pending_) &&
            (!client->write_io_context_.is_pending_))
        {
            if (len > kMaximumMessageSize)
            {
                DLOG(ERROR) << "write data out of memory";
                return false;
            }

            // 申请足够内存块存放pending数据


            BOOL ok = ::WriteFile(client->pipe_handle_,
                client->write_io_context_.io_buffer.buffer,
                len,
                nullptr,
                &client->write_io_context_.overlapped);
            if (!ok)
            {
                DWORD err = ::GetLastError();
                if (err == ERROR_IO_PENDING)
                {
                    return true;
                }
                DLOG(ERROR) << "pipe write error: " << err;
                return false;
            }
            return true;
        }
        return false;
    }

    // PipeServer::ClientInfo
    PipeServer::ClientInfo::ClientInfo(HANDLE pipe_handle, PipeServer* delegate)
        : pipe_handle_(pipe_handle)
        , delegate_(delegate)
    {
    }

    PipeServer::ClientInfo::~ClientInfo()
    {
        if (pipe_handle_ != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle(pipe_handle_);
            pipe_handle_ = INVALID_HANDLE_VALUE;
        }
    }

    PipeServer::AsyncType PipeServer::ClientInfo::GetAsyncType(MessagePumpForIO::IOContext* context)
    {
        PipeServer::AsyncType type = PipeServer::AsyncType::Unknown;
        if (&accept_io_context_.overlapped == context)
        {
            type = PipeServer::AsyncType::Accept;
        }
        else if (&read_io_context_.overlapped == context)
        {
            type = PipeServer::AsyncType::Read;
        }
        else if (&write_io_context_.overlapped == context)
        {
            type = PipeServer::AsyncType::Write;
        }
        return type;
    }

    void PipeServer::ClientInfo::OnIOCompleted(MessagePumpForIO::IOContext* context, DWORD bytes_transfered, DWORD error)
    {
        auto type = GetAsyncType(context);
        switch (type)
        {
        case PipeServer::AsyncType::Accept:
            {
                accept_io_context_.is_pending_ = false;

                if (error == NOERROR)
                {
                    // 通知有新的连接到来，
                    // delegate应该在这个通知里面抛送Read异步操作，否则无法接收数据
                    if (delegate_)
                    {
                        delegate_->OnClientConnect(this);
                    }
                }
            }
            break;
        case PipeServer::AsyncType::Read:
            {
                read_io_context_.is_pending_ = false;

                if (error == NOERROR)
                {
                    // 通知数据接收完毕，
                    // delegate应该在这个通知里面继续抛送Read异步操作，否则无法持续接收数据
                    if (delegate_)
                    {
                        delegate_->OnReadData(this);
                    }
                }
            }
            break;
        case PipeServer::AsyncType::Write:
            {
                write_io_context_.is_pending_ = false;

                if (delegate_)
                {
                    delegate_->OnSendData(this);
                }
            }
            break;
        default:
            if (delegate_)
            {
                delegate_->OnClientDisconnect(this);
            }
            break;
        }
    }

}