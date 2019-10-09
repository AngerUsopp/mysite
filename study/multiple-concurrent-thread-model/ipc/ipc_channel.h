#pragma once

#include <limits>
#include <queue>

#include "ipc_message.h"
#include "net/pipe/pipe.h"

namespace mctm
{
    class IPCListener
    {
    public:
        virtual bool OnMessageReceived(const IPCMessage& message) = 0;

        // Called when the channel is connected and we have received the internal
        // Hello message from the peer.
        virtual void OnChannelConnected(int peer_pid) {}

        // Called when the channel is connected and we have received the internal
        // Hello message from the peer.
        virtual void OnChannelClosed(int peer_pid) {}

        // Called when an error is detected that causes the channel to close.
        // This method is not called when a channel is closed normally.
        virtual void OnChannelError() {}

    protected:
        virtual ~IPCListener() = default;
    };

    class IPCChannel 
        : public PipeServer::Delegate
        , public PipeClient::Delegate
    {
        class Message
        {
        public:
            Message(IPCMessage* msg)
            {
                ipc_message_.reset(msg);
            }

            size_t size() const { return ipc_message_->size(); }

            const void* data() const { return reinterpret_cast<char*>(const_cast<void*>(ipc_message_->data())) + offset_; }

            size_t offset() const { return offset_; }
            void increase_offset(size_t increament) { offset_ += increament; }

        private:
            std::unique_ptr<IPCMessage> ipc_message_;
            size_t offset_ = 0;
        };
        using MessageQueue = std::queue<std::unique_ptr<Message>>;

    public:
        enum ModeFlags
        {
            MODE_NO_FLAG = 0x0,
            MODE_SERVER_FLAG = 0x1,
            MODE_CLIENT_FLAG = 0x2,
        };

        enum Mode
        {
            MODE_NONE = MODE_NO_FLAG,
            MODE_SERVER = MODE_SERVER_FLAG,
            MODE_CLIENT = MODE_CLIENT_FLAG,
        };

        IPCChannel(const wchar_t* pipe_name, Mode mode, IPCListener* listener);
        virtual ~IPCChannel();

        bool Connect();
        bool Send(IPCMessage* message);
        void Close();

    protected:
        // PipeServer::Delegate
        void OnPipeServerAccept(ULONG_PTR client_key, DWORD error) override;
        void OnPipeServerReadData(ULONG_PTR client_key, DWORD error, const char* data, unsigned int len) override;
        void OnPipeServerWriteData(ULONG_PTR client_key, DWORD error, const char* data, unsigned int len) override;
        void OnPipeServerDisconnect(ULONG_PTR client_key) override;

        // PipeClient::Delegate
        void OnPipeClientConnect(PipeClient* client_key, DWORD error) override;
        void OnPipeClientReadData(PipeClient* client_key, DWORD error, const char* data, unsigned int len) override;
        void OnPipeClientWriteData(PipeClient* client_key, DWORD error, const char* data, unsigned int len) override;
        void OnPipeClientDisconnect(PipeClient* client_key) override;

    private:
        bool ProcessOutgoingMessages();
        std::unique_ptr<IPCMessage> ProcessIncomingMessages(const char* data, unsigned int len);
        void HandleHelloMessage(const IPCMessage& msg);
        void ClearMessageQueue();
        void OnChannelReadData(const char* data, unsigned int len);
        void OnChannelError();
        void OnChannelClosed();

    private:
        IPCListener* listener_ = nullptr;
        std::wstring pipe_name_;
        Mode mode_ = MODE_NONE;

        std::unique_ptr<PipeServer> pipe_srv_;
        ULONG_PTR client_key_ = 0;

        std::unique_ptr<PipeClient> pipe_clt_;

        DWORD peer_pid_ = 0;
        std::string input_overflow_buf_;
        MessageQueue output_queue_;
    };
}