/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/**
 * @file net_portforward.cpp
 *     Port forwarding support using NAT-PMP and UPnP.
 * @par Purpose:
 *     Automatic port forwarding for multiplayer hosting.
 *     Tries NAT-PMP first (simpler/faster), falls back to UPnP.
 * @author   KeeperFX Team
 * @date     01 Jan 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "net_portforward.h"
#include "bflib_basics.h"

#define MINIUPNP_STATICLIB
#include <miniupnpc/miniupnpc.h>
#include <miniupnpc/upnpcommands.h>
#include <miniupnpc/upnperrors.h>

#define NATPMP_STATICLIB
#include <natpmp/natpmp.h>

#include <cstdio>
#include <ctime>
#include <thread>
#include <winsock2.h>
#include <iphlpapi.h>

#define NATPMP_TIMEOUT_SECONDS 1.0
#define UPNP_TIMEOUT_MS 3000

#include "post_inc.h"

enum PortForwardMethod {
    PORT_FORWARD_NONE = 0,
    PORT_FORWARD_NATPMP,
    PORT_FORWARD_UPNP
};

static enum PortForwardMethod active_method = PORT_FORWARD_NONE;
static uint16_t mapped_port = 0;

static struct UPNPUrls upnp_urls;
static struct IGDdatas upnp_data;
static char upnp_lanaddr[64];

static natpmp_t natpmp;

static int is_cgnat_detected() {
    ULONG buffer_size = 0;
    if (GetAdaptersInfo(NULL, &buffer_size) != ERROR_BUFFER_OVERFLOW) {
        return 0;
    }
    IP_ADAPTER_INFO *adapter_info = (IP_ADAPTER_INFO *)malloc(buffer_size);
    if (!adapter_info) {
        return 0;
    }
    if (GetAdaptersInfo(adapter_info, &buffer_size) != NO_ERROR) {
        free(adapter_info);
        return 0;
    }
    for (IP_ADAPTER_INFO *adapter = adapter_info; adapter; adapter = adapter->Next) {
        unsigned long local_ip = inet_addr(adapter->IpAddressList.IpAddress.String);
        if (local_ip != INADDR_NONE && local_ip != 0) {
            unsigned char first_octet = local_ip & 0xFF;
            unsigned char second_octet = (local_ip >> 8) & 0xFF;
            if (first_octet == 100 && second_octet >= 64 && second_octet <= 127) {
                free(adapter_info);
                return 1;
            }
        }
        unsigned long gateway_ip = inet_addr(adapter->GatewayList.IpAddress.String);
        if (gateway_ip != INADDR_NONE && gateway_ip != 0) {
            unsigned char first_octet = gateway_ip & 0xFF;
            unsigned char second_octet = (gateway_ip >> 8) & 0xFF;
            if (first_octet == 100 && second_octet >= 64 && second_octet <= 127) {
                free(adapter_info);
                return 1;
            }
        }
    }
    free(adapter_info);
    return 0;
}

static int natpmp_add_port_mapping(uint16_t port) {
    clock_t start_time = clock();
    if (initnatpmp(&natpmp, 0, 0) < 0) {
        LbNetLog("NAT-PMP: Failed to initialize\n");
        return 0;
    }
    if (sendpublicaddressrequest(&natpmp) < 0) {
        LbNetLog("NAT-PMP: Failed to send public address request\n");
        closenatpmp(&natpmp);
        return 0;
    }
    natpmpresp_t response;
    fd_set file_descriptors;
    struct timeval timeout;
    int result;
    do {
        double elapsed = (double)(clock() - start_time) / CLOCKS_PER_SEC;
        if (elapsed > NATPMP_TIMEOUT_SECONDS) {
            LbNetLog("NAT-PMP: Timeout getting public address\n");
            closenatpmp(&natpmp);
            return 0;
        }
        FD_ZERO(&file_descriptors);
        FD_SET(natpmp.s, &file_descriptors);
        getnatpmprequesttimeout(&natpmp, &timeout);
        select(FD_SETSIZE, &file_descriptors, NULL, NULL, &timeout);
        result = readnatpmpresponseorretry(&natpmp, &response);
    } while (result == NATPMP_TRYAGAIN);
    if (result < 0) {
        LbNetLog("NAT-PMP: Failed to get public address\n");
        closenatpmp(&natpmp);
        return 0;
    }
    if (sendnewportmappingrequest(&natpmp, NATPMP_PROTOCOL_UDP, port, port, 0x7FFFFFFF) < 0) {
        LbNetLog("NAT-PMP: Failed to send port mapping request\n");
        closenatpmp(&natpmp);
        return 0;
    }
    do {
        double elapsed = (double)(clock() - start_time) / CLOCKS_PER_SEC;
        if (elapsed > NATPMP_TIMEOUT_SECONDS) {
            LbNetLog("NAT-PMP: Timeout on port mapping\n");
            closenatpmp(&natpmp);
            return 0;
        }
        FD_ZERO(&file_descriptors);
        FD_SET(natpmp.s, &file_descriptors);
        getnatpmprequesttimeout(&natpmp, &timeout);
        select(FD_SETSIZE, &file_descriptors, NULL, NULL, &timeout);
        result = readnatpmpresponseorretry(&natpmp, &response);
    } while (result == NATPMP_TRYAGAIN);
    if (result < 0) {
        LbNetLog("NAT-PMP: Permanent lease rejected, trying timed lease\n");
        if (sendnewportmappingrequest(&natpmp, NATPMP_PROTOCOL_UDP, port, port, 3600) < 0) {
            LbNetLog("NAT-PMP: Failed to send timed port mapping request\n");
            closenatpmp(&natpmp);
            return 0;
        }
        do {
            double elapsed = (double)(clock() - start_time) / CLOCKS_PER_SEC;
            if (elapsed > NATPMP_TIMEOUT_SECONDS) {
                LbNetLog("NAT-PMP: Timeout on timed lease\n");
                closenatpmp(&natpmp);
                return 0;
            }
            FD_ZERO(&file_descriptors);
            FD_SET(natpmp.s, &file_descriptors);
            getnatpmprequesttimeout(&natpmp, &timeout);
            select(FD_SETSIZE, &file_descriptors, NULL, NULL, &timeout);
            result = readnatpmpresponseorretry(&natpmp, &response);
        } while (result == NATPMP_TRYAGAIN);
        if (result < 0) {
            LbNetLog("NAT-PMP: Failed to add port mapping\n");
            closenatpmp(&natpmp);
            return 0;
        }
    }
    LbNetLog("NAT-PMP: Mapped port %u\n", port);
    mapped_port = port;
    active_method = PORT_FORWARD_NATPMP;
    closenatpmp(&natpmp);
    return 1;
}

static void port_forward_add_mapping_internal(uint16_t port) {
    if (is_cgnat_detected()) {
        LbNetLog("CGNAT detected, automatic port forwarding unavailable\n");
        return;
    }
    if (active_method != PORT_FORWARD_NONE) {
        port_forward_remove_mapping();
    }
    if (natpmp_add_port_mapping(port)) {
        return;
    }
    int error = 0;
    struct UPNPDev *device_list = upnpDiscover(UPNP_TIMEOUT_MS, NULL, NULL, 0, 0, 2, &error);
    if (!device_list) {
        LbNetLog("UPnP: No devices found\n");
        return;
    }
    int internet_gateway_device_result = UPNP_GetValidIGD(device_list, &upnp_urls, &upnp_data, upnp_lanaddr, sizeof(upnp_lanaddr), NULL, 0);
    freeUPNPDevlist(device_list);
    if (internet_gateway_device_result == 0) {
        LbNetLog("UPnP: Failed to get valid IGD\n");
        FreeUPNPUrls(&upnp_urls);
        return;
    }
    char port_string[16];
    snprintf(port_string, sizeof(port_string), "%u", port);
    UPNP_DeletePortMapping(upnp_urls.controlURL, upnp_data.first.servicetype, port_string, "UDP", "");
    int result = UPNP_AddPortMapping(upnp_urls.controlURL, upnp_data.first.servicetype, port_string, port_string, upnp_lanaddr, "KeeperFX", "UDP", "", "0");
    if (result != UPNPCOMMAND_SUCCESS) {
        LbNetLog("UPnP: Permanent lease rejected, trying timed lease\n");
        result = UPNP_AddPortMapping(upnp_urls.controlURL, upnp_data.first.servicetype, port_string, port_string, upnp_lanaddr, "KeeperFX", "UDP", "", "3600");
        if (result != UPNPCOMMAND_SUCCESS) {
            LbNetLog("UPnP: Failed to add port mapping\n");
            FreeUPNPUrls(&upnp_urls);
            return;
        }
    }
    LbNetLog("UPnP: Mapped port %u\n", port);
    mapped_port = port;
    active_method = PORT_FORWARD_UPNP;
}

int port_forward_add_mapping(uint16_t port) {
    std::thread(port_forward_add_mapping_internal, port).detach();
    return 1;
}

void port_forward_remove_mapping(void) {
    if (active_method == PORT_FORWARD_NONE || mapped_port == 0) {
        return;
    }
    if (active_method == PORT_FORWARD_NATPMP) {
        if (initnatpmp(&natpmp, 0, 0) < 0) {
            LbNetLog("NAT-PMP: Failed to initialize for removal\n");
            mapped_port = 0;
            active_method = PORT_FORWARD_NONE;
            return;
        }
        sendnewportmappingrequest(&natpmp, NATPMP_PROTOCOL_UDP, mapped_port, 0, 0);
        natpmpresp_t response;
        fd_set file_descriptors;
        struct timeval timeout;
        int result;
        clock_t start_time = clock();
        do {
            double elapsed = (double)(clock() - start_time) / CLOCKS_PER_SEC;
            if (elapsed > NATPMP_TIMEOUT_SECONDS) {
                break;
            }
            FD_ZERO(&file_descriptors);
            FD_SET(natpmp.s, &file_descriptors);
            getnatpmprequesttimeout(&natpmp, &timeout);
            select(FD_SETSIZE, &file_descriptors, NULL, NULL, &timeout);
            result = readnatpmpresponseorretry(&natpmp, &response);
        } while (result == NATPMP_TRYAGAIN);
        closenatpmp(&natpmp);
    } else if (active_method == PORT_FORWARD_UPNP) {
        char port_string[16];
        snprintf(port_string, sizeof(port_string), "%u", mapped_port);
        UPNP_DeletePortMapping(upnp_urls.controlURL, upnp_data.first.servicetype, port_string, "UDP", NULL);
        FreeUPNPUrls(&upnp_urls);
    }
    mapped_port = 0;
    active_method = PORT_FORWARD_NONE;
}
