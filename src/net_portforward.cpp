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
#include <winsock2.h>

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

static int natpmp_add_port_mapping(uint16_t port) {
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

int port_forward_add_mapping(uint16_t port) {
    if (active_method != PORT_FORWARD_NONE) {
        port_forward_remove_mapping();
    }
    if (natpmp_add_port_mapping(port)) {
        return 1;
    }
    int error = 0;
    struct UPNPDev *device_list = upnpDiscover(2000, NULL, NULL, 0, 0, 2, &error);
    if (!device_list) {
        LbNetLog("UPnP: No devices found\n");
        return 0;
    }
    int internet_gateway_device_result = UPNP_GetValidIGD(device_list, &upnp_urls, &upnp_data, upnp_lanaddr, sizeof(upnp_lanaddr), NULL, 0);
    freeUPNPDevlist(device_list);
    if (internet_gateway_device_result == 0) {
        LbNetLog("UPnP: Failed to get valid IGD\n");
        FreeUPNPUrls(&upnp_urls);
        return 0;
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
            return 0;
        }
    }
    LbNetLog("UPnP: Mapped port %u\n", port);
    mapped_port = port;
    active_method = PORT_FORWARD_UPNP;
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
        do {
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
