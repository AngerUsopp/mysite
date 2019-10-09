#include "ipc_channel.h"

#include <algorithm>

#include "logging/logging.h"

namespace mctm
{
    IPCChannel::IPCChannel(const wchar_t* pipe_name, Mode mode, IPCListener* listener)
        : pipe_name_(pipe_name)
        , mode_(mode)
        , listener_(listener)
    {
        switch (mode)
        {
        case mctm::IPCChannel::MODE_SERVER:
            pipe_srv_ = std::make_unique<PipeServer>(pipe_name, this);
            break;
        case mctm::IPCChannel::MODE_CLIENT:
            pipe_clt_ = std::make_unique<PipeClient>(pipe_name, this);
            break;
        default:
            break;
        }
    }

    IPCChannel::~IPCChannel()
    {
    }

    bool IPCChannel::Connect()
    {
        bool ret = false;
        switch (mode_)
        {
        case mctm::IPCChannel::MODE_SERVER:
            ret = pipe_srv_->Start();
            break;
        case mctm::IPCChannel::MODE_CLIENT:
            ret = pipe_clt_->Connect();
            break;
        default:
            break;
        }

        if (ret)
        {
            std::unique_ptr<IPCMessage> msg = std::make_unique<IPCMessage>(
                MSG_ROUTING_NONE,
                HELLO_MESSAGE_TYPE,
                IPCMessage::PRIORITY_NORMAL);
            if (!msg->WriteInt(GetCurrentProcessId())/* ||
                (secret && !m->WriteUInt32(secret))*/)
            {
                Close();
                return false;
            }
            Send(msg.release());
        }

        return ret;
    }

    bool IPCChannel::Send(IPCMessage* message)
    {
        DCHECK(message);
        bool ret = false;
        if (mode_ != MODE_NONE)
        {
            std::unique_ptr<Message> msg = std::make_unique<Message>(message);
            output_queue_.push(std::move(msg));

            ProcessOutgoingMessages();
        }
        return ret;
    }

    void IPCChannel::Close()
    {
        switch (mode_)
        {
        case mctm::IPCChannel::MODE_SERVER:
            pipe_srv_->Stop();
            break;
        case mctm::IPCChannel::MODE_CLIENT:
            pipe_clt_->Close();
            break;
        default:
            break;
        }
    }

    // server mode
    void IPCChannel::OnPipeServerAccept(ULONG_PTR client_key, DWORD error)
    {
        // wait for hello msg
        if (error == NOERROR)
        {
            client_key_ = client_key;

            ProcessOutgoingMessages();
        }
        else
        {
            OnChannelError();
        }
    }

    void IPCChannel::OnPipeServerReadData(ULONG_PTR client_key, DWORD error, const char* data, unsigned int len)
    {
        if (error == NOERROR)
        {
            OnChannelReadData(data, len);
        }
        else
        {
            OnChannelError();
        }
    }

    void IPCChannel::OnPipeServerWriteData(ULONG_PTR client_key, DWORD error, const char* data, unsigned int len)
    {
        if (error == NOERROR)
        {
            ProcessOutgoingMessages();
        }
        else
        {
            OnChannelError();
        }
    }

    void IPCChannel::OnPipeServerDisconnect(ULONG_PTR client_key)
    {
        client_key_ = 0;

        OnChannelClosed();
    }

    // client mode
    void IPCChannel::OnPipeClientConnect(PipeClient* client_key, DWORD error)
    {
        // send hello msg
        if (error == NOERROR)
        {
            ProcessOutgoingMessages();
        }
        else
        {
            OnChannelError();
        }
    }

    void IPCChannel::OnPipeClientReadData(PipeClient* client_key, DWORD error, const char* data, unsigned int len)
    {
        if (error == NOERROR)
        {
            OnChannelReadData(data, len);
        }
        else
        {
            OnChannelError();
        }
    }

    void IPCChannel::OnPipeClientWriteData(PipeClient* client_key, DWORD error, const char* data, unsigned int len)
    {
        if (error == NOERROR)
        {
            ProcessOutgoingMessages();
        }
        else
        {
            OnChannelError();
        }
    }

    void IPCChannel::OnPipeClientDisconnect(PipeClient* client_key)
    {
        OnChannelClosed();
    }

    bool IPCChannel::ProcessOutgoingMessages()
    {
        if (output_queue_.empty())
        {
            return false;
        }

        bool ret = false;
        size_t current_send_bytes = 0;
        switch (mode_)
        {
        case mctm::IPCChannel::MODE_SERVER:
            if (client_key_ != 0)
            {
                auto& msg = output_queue_.front();
                DCHECK(msg);
                size_t surplus = msg->size() - msg->offset();
                current_send_bytes = std::min(surplus, kIOBufferSize);
                ret = pipe_srv_->Send(client_key_, (const char*)msg->data(), current_send_bytes);
            }
            break;
        case mctm::IPCChannel::MODE_CLIENT:
            {
                auto& msg = output_queue_.front();
                DCHECK(msg);
                size_t surplus = msg->size() - msg->offset();
                current_send_bytes = std::min(surplus, kIOBufferSize);
                ret = pipe_clt_->Send((const char*)msg->data(), current_send_bytes);
            }
            break;
        default:
            break;
        }

        if (ret)
        {
            auto& msg = output_queue_.front();
            DCHECK(msg);
            msg->increase_offset(current_send_bytes);
            if (msg->offset() == msg->size())
            {
                output_queue_.pop();
            }
        }

        return ret;
    }

    std::unique_ptr<IPCMessage> IPCChannel::ProcessIncomingMessages(const char* data, unsigned int len)
    {
        input_overflow_buf_.append(data, len);

        std::unique_ptr<IPCMessage> msg;
        const char* msg_pickle = IPCMessage::FindNext(
            input_overflow_buf_.data(), input_overflow_buf_.data() + input_overflow_buf_.length());
        if (msg_pickle)
        {
            int len = static_cast<int>(msg_pickle - input_overflow_buf_.data());
            DCHECK(len > 0);
            msg = std::make_unique<IPCMessage>(input_overflow_buf_.data(), len);
        }
        return msg;
    }

    void IPCChannel::HandleHelloMessage(const IPCMessage& msg)
    {
        // The hello message contains one parameter containing the PID.
        PickleIterator it(msg);
        int claimed_pid;
        bool failed = !it.ReadInt(&claimed_pid);

        /*if (!failed && validate_client_)
        {
            int secret;
            failed = it.ReadInt(&secret) ? (secret != client_secret_) : true;
        }*/

        if (failed)
        {
            NOTREACHED();
            Close();
            OnChannelError();
            return;
        }

        peer_pid_ = claimed_pid;
        // Validation completed.
        //validate_client_ = false;

        if (listener_)
        {
            listener_->OnChannelConnected(peer_pid_);
        }
    }

    void IPCChannel::OnChannelError()
    {
        if (listener_)
        {
            listener_->OnChannelError();
        }
        ClearMessageQueue();
    }

    void IPCChannel::OnChannelClosed()
    {
        if (listener_)
        {
            listener_->OnChannelClosed(peer_pid_);
        }
        ClearMessageQueue();
    }

    void IPCChannel::ClearMessageQueue()
    {
        MessageQueue empty;
        output_queue_.swap(empty);
    }

    void IPCChannel::OnChannelReadData(const char* data, unsigned int len)
    {
        std::unique_ptr<IPCMessage> msg = ProcessIncomingMessages(data, len);
        if (!msg)
        {
            return;
        }

        if (msg->type() == HELLO_MESSAGE_TYPE)
        {
            HandleHelloMessage(*msg);
        }
        else
        {
            DCHECK(peer_pid_) << "the first msg must be hello msg";

            if (listener_)
            {
                listener_->OnMessageReceived(*msg);
            }
        }

        input_overflow_buf_ = input_overflow_buf_.substr(std::min(msg->size(), input_overflow_buf_.length()));
    }

}