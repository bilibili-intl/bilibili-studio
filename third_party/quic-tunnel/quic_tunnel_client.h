#ifndef QUIC_TUNNEL_CLIENT_H_
#define QUIC_TUNNEL_CLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef void* tQuicTunnelClientHandle;

typedef struct tQuicTunnelClientOptions {
    int   port;
    char* tunnel_host;
    char* tunnel_ip;
    int   tunnel_port;
    char* forward_ip;
    int   forward_port;
} tQuicTunnelClientOptions;

typedef struct tQuicTunnelClientResult {
    int err_code;
    int tcp_bind_port;
} tQuicTunnelClientResult;

typedef enum tQuicTunnelErrorCode {
    QUIC_TUNNEL_SUCCESS = 0,
    QUIC_TUNNEL_ILLEGAL_OPTIONS = -1001,
    QUIC_TUNNEL_NOENOUGH_MEMORY = -2001,
    QUIC_TUNNEL_ERROR_STARTUP   = -3001,
    QUIC_TUNNEL_ERROR_OTHER     = -9001,
} tQuicTunnelErrorCode;

tQuicTunnelClientHandle
quic_tunnel_client_start(
    const tQuicTunnelClientOptions* opts,
    tQuicTunnelClientResult* res);

void
quic_tunnel_client_stop(tQuicTunnelClientHandle handler);

#ifdef __cplusplus
}
#endif

#endif // QUIC_TUNNEL_CLIENT_H_
