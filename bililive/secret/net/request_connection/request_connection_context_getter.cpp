/*
 @ 0xCCCCCCCC
*/

#include "bililive/secret/net/request_connection/request_connection_context_getter.h"

#include "base/command_line.h"

#include "net/proxy/proxy_service.h"
#include "net/url_request/url_request_context_builder.h"

namespace {

const char kSwitchIgnoreCertificateErrors[] = "ignore-certificate-errors";
const char kSwitchEnableSystemProxy[] = "enable-system-proxy";

bool ShouldUseSystemProxy()
{
    return CommandLine::ForCurrentProcess()->HasSwitch(kSwitchEnableSystemProxy);
}

}   // namespace

namespace secret {

RequestConnectionContextGetter::RequestConnectionContextGetter(
    const scoped_refptr<base::SingleThreadTaskRunner>& task_runner,
    const scoped_refptr<base::SingleThreadTaskRunner>& relay_runner)
    : task_thread_runner_(task_runner),
      relay_thread_runner_(relay_runner),
      use_system_proxy_(ShouldUseSystemProxy()),
      shutting_down_(false),
      shutdown_event_(false, false)
{}

RequestConnectionContextGetter::~RequestConnectionContextGetter()
{
    Cleanup();
    shutdown_event_.Wait();
}

scoped_refptr<base::SingleThreadTaskRunner> RequestConnectionContextGetter::GetRelayThreadRunner() const
{
    return relay_thread_runner_;
}

net::URLRequestContext* RequestConnectionContextGetter::GetURLRequestContext()
{
    if (!shutting_down_.load(std::memory_order_acquire) && !context_) {
        net::URLRequestContextBuilder builder;

        builder.DisableHttpCache();

        // Force to ignore SSL certificate errors by default.
        // Screw fucking cyber security.
        net::URLRequestContextBuilder::HttpNetworkSessionParams session_params;
        session_params.ignore_certificate_errors = true;
        builder.set_http_network_session_params(session_params);

        context_.reset(builder.Build());
    }

    return context_.get();
}

scoped_refptr<base::SingleThreadTaskRunner> RequestConnectionContextGetter::GetNetworkTaskRunner() const
{
    return task_thread_runner_;
}

void RequestConnectionContextGetter::Cleanup()
{
    if (!task_thread_runner_->BelongsToCurrentThread()) {
        task_thread_runner_->PostTask(
            FROM_HERE,
            base::Bind(&RequestConnectionContextGetter::Cleanup, this));
        return;
    }

    context_ = nullptr;

    shutdown_event_.Signal();
}

void RequestConnectionContextGetter::MarkAsShuttingDown()
{
    shutting_down_.store(true, std::memory_order_release);
}

bool RequestConnectionContextGetter::IsShuttingDown() const
{
    return shutting_down_.load(std::memory_order_acquire);
}

bool RequestConnectionContextGetter::use_system_proxy() const
{
    return use_system_proxy_;
}

}   // namespace secret
