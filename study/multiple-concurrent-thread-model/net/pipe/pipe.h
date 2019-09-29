#pragma once
#include <atomic>
#include <list>
#include <string>

#include "message_loop/message_pump.h"
#include "net/io_buffer_pool.h"

namespace mctm
{
    class PipeDataTransfer
    {
    public:
        PipeDataTransfer();
        virtual ~PipeDataTransfer();

        bool Send();
        bool Read();
        void Close();

    protected:
        //HANDLE pipe_handle_ = INVALID_HANDLE_VALUE;
    };

    class PipeServer 
        : PipeDataTransfer
    {
        enum class AsyncType
        {
            Unknown,
            // PIPE
            Accept,
            Read,
            Write,
        };

        struct AsyncContext
        {
            MessagePumpForIO::IOContext overlapped;
            IOBuffer io_buffer;
            std::atomic_bool is_pending_ = true;
        };

        class ClientInfo : public MessagePumpForIO::IOHandler
        {
        public:
            ClientInfo(HANDLE pipe_handle, PipeServer* delegate);

            ~ClientInfo();

            AsyncType GetAsyncType(MessagePumpForIO::IOContext* context);

        public:
            HANDLE pipe_handle_ = INVALID_HANDLE_VALUE;

            AsyncContext accept_io_context_;
            AsyncContext read_io_context_;
            AsyncContext write_io_context_;

        protected:
            void OnIOCompleted(MessagePumpForIO::IOContext* context, DWORD bytes_transfered,
                DWORD error) override;

        private:
            PipeServer* delegate_ = nullptr;
        };
        using ScopedClient = std::unique_ptr<ClientInfo>;

    public:
        class Delegate
        {
        public:
            virtual ~Delegate() = default;

            virtual void OnClientConnect() = 0;
            virtual void OnReadData() = 0;
            virtual void OnSendData() = 0;
            virtual void OnClientDisconnect() = 0;
        };

        PipeServer(const wchar_t* pipe_name, Delegate* delegate, unsigned int max_pipe_instances_count = 1);
        virtual ~PipeServer();

        bool Start();

    protected:

    private:
        ScopedClient Create();
        bool Accept(ClientInfo* client);
        bool Read(ClientInfo* client);
        bool Write(ClientInfo* client, const char* data, unsigned int len);

        virtual void OnClientConnect(ClientInfo* client) = 0;
        virtual void OnReadData(ClientInfo* client) = 0;
        virtual void OnSendData(ClientInfo* client) = 0;
        virtual void OnClientDisconnect(ClientInfo* client) = 0;

    private:
        friend class ClientInfo;

        std::wstring pipe_name_;
        unsigned int max_pipe_instances_count_ = 1;
        std::list<ScopedClient> clients_;
        Delegate* delegate_ = nullptr;
    };

    class PipeClient
        : PipeDataTransfer
        , MessagePumpForIO::IOHandler
    {
    public:
        class Delegate
        {
        public:
            virtual ~Delegate() = default;

            virtual void OnConnectCompleted() = 0;
        };

        PipeClient(const wchar_t* pipe_name);
        virtual ~PipeClient();

        bool Connect();

    protected:
        // IOHandler
        void OnIOCompleted(MessagePumpForIO::IOContext* context, DWORD bytes_transfered,
            DWORD error) override;

    private:
        std::wstring pipe_name_;
    };
}