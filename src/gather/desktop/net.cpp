#include "gather/gather.hpp"
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <cstdio>
#include <cstring>

namespace rfxh::gather {

void gather_ip(GatherContext& ctx) {
    struct ifaddrs* ifa_list;
    if (getifaddrs(&ifa_list) != 0)
        return;
    for (struct ifaddrs* ifa = ifa_list; ifa; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET)
            continue;
        if (std::strcmp(ifa->ifa_name, "lo") == 0)
            continue;
        auto* sa = reinterpret_cast<struct sockaddr_in*>(ifa->ifa_addr);
        char addr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &sa->sin_addr, addr, sizeof(addr));

        auto* mask = reinterpret_cast<struct sockaddr_in*>(ifa->ifa_netmask);
        unsigned int bits = 0;
        if (mask) {
            unsigned int m = ntohl(mask->sin_addr.s_addr);
            while (m & 0x80000000) {
                bits++;
                m <<= 1;
            }
        }

        char lbl[64];
        std::snprintf(lbl, sizeof(lbl), "Local IP (%s)", ifa->ifa_name);
        if (bits > 0) {
            char full[80];
            std::snprintf(full, sizeof(full), "%s/%u", addr, bits);
            add_info(ctx, lbl, "%s", full);
        } else {
            add_info(ctx, lbl, "%s", addr);
        }
        break;
    }
    freeifaddrs(ifa_list);
}

} // namespace rfxh::gather
