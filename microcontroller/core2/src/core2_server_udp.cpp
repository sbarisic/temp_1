
#include <core2.h>
#include <core2_server_udp.h>

#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_netif.h>
#include <esp_sntp.h>
#include <esp_tls.h>

#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <lwip/dns.h>

#include <WiFi.h>
#include <HTTPClient.h>

#define CORE2_WEB_USER_AGENT "Core2_ESP32"
#define PORT 5871

void udp_server_task(void *a)
{
    dprintf("Starting UDP server\n");

    char rx_buffer[128];
    char addr_str[128];
    int addr_family = AF_INET;
    int ip_protocol = 0;
    struct sockaddr_in6 dest_addr;

    struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
    dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr_ip4->sin_family = AF_INET;
    dest_addr_ip4->sin_port = htons(PORT);
    ip_protocol = IPPROTO_IP;

    int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
    if (sock < 0)
    {
        dprintf("Cannot open UDP socket\n");
        vTaskDelete(NULL);
        return;
    }

    dprintf("Socket created\n");

#if defined(CONFIG_LWIP_NETBUF_RECVINFO) && !defined(CONFIG_EXAMPLE_IPV6)
    // Receive info 1
    int enable = 1;
    lwip_setsockopt(sock, IPPROTO_IP, IP_PKTINFO, &enable, sizeof(enable));
#endif

    // Set timeout
    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0)
    {
        dprintf("Socket unable to bind: errno %d\n", errno);
        vTaskDelete(NULL);
        return;
    }

    dprintf("Socket bound, port %d\n", PORT);

    struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
    socklen_t socklen = sizeof(source_addr);

#if defined(CONFIG_LWIP_NETBUF_RECVINFO) && !defined(CONFIG_EXAMPLE_IPV6)
    // Receive info 2
    struct iovec iov;
    struct msghdr msg;
    struct cmsghdr *cmsgtmp;
    u8_t cmsg_buf[CMSG_SPACE(sizeof(struct in_pktinfo))];

    iov.iov_base = rx_buffer;
    iov.iov_len = sizeof(rx_buffer);
    msg.msg_control = cmsg_buf;
    msg.msg_controllen = sizeof(cmsg_buf);
    msg.msg_flags = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_name = (struct sockaddr *)&source_addr;
    msg.msg_namelen = socklen;
#endif

    dprintf("UDP - OK\n");
    while (true)
    {
        vTaskDelay(1);

#if defined(CONFIG_LWIP_NETBUF_RECVINFO) && !defined(CONFIG_EXAMPLE_IPV6)
        int len = recvmsg(sock, &msg, 0);
#else
        int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);
#endif

        // Error occurred during receiving
        if (len < 0)
        {
            dprintf("recvfrom failed: errno %d\n", errno);
            break;
        }
        // Data received
        else
        {
            // Get the sender's ip address as string
            if (source_addr.ss_family == PF_INET)
            {
                inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);

#if defined(CONFIG_LWIP_NETBUF_RECVINFO) && !defined(CONFIG_EXAMPLE_IPV6)
                for (cmsgtmp = CMSG_FIRSTHDR(&msg); cmsgtmp != NULL; cmsgtmp = CMSG_NXTHDR(&msg, cmsgtmp))
                {
                    if (cmsgtmp->cmsg_level == IPPROTO_IP && cmsgtmp->cmsg_type == IP_PKTINFO)
                    {
                        struct in_pktinfo *pktinfo;
                        pktinfo = (struct in_pktinfo *)CMSG_DATA(cmsgtmp);
                        ESP_LOGI(TAG, "dest ip: %s", inet_ntoa(pktinfo->ipi_addr));
                    }
                }
#endif
            }
            else if (source_addr.ss_family == PF_INET6)
            {
                inet6_ntoa_r(((struct sockaddr_in6 *)&source_addr)->sin6_addr, addr_str, sizeof(addr_str) - 1);
            }

            rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string...
            dprintf("Received %d bytes from %s: ", len, addr_str);
            dprintf("'%s'\n", rx_buffer);

            int err = sendto(sock, rx_buffer, len, 0, (struct sockaddr *)&source_addr, sizeof(source_addr));
            if (err < 0)
            {
                dprintf("Error occurred during sending: errno %d", errno);
                break;
            }
        }
    }

    dprintf("Shutting down socket\n");
    shutdown(sock, 0);
    close(sock);
    vTaskDelete(NULL);
}

void core2_server_udp_start()
{
    xTaskCreate(udp_server_task, "core2.udp_server", 4096, NULL, 5, NULL);
}