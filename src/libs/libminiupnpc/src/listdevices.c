/* $Id: listdevices.c,v 1.6 2015/07/23 20:40:08 nanard Exp $ */
/* Project : miniupnp
 * Author : Thomas Bernard
 * Copyright (c) 2013-2015 Thomas Bernard
 * This software is subject to the conditions detailed in the
 * LICENCE file provided in this distribution. */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
#include <winsock2.h>
#endif /* _WIN32 */
#include "miniupnpc.h"

struct upnp_dev_list {
	struct upnp_dev_list * next;
	char * descURL;
	struct UPNPDev * * array;
	size_t count;
	size_t allocated_count;
};

#define ADD_DEVICE_COUNT_STEP 16

void add_device(struct upnp_dev_list * * list_head, struct UPNPDev * dev)
{
	struct upnp_dev_list * elt;
	size_t i;

	if(dev == NULL)
		return;
	for(elt = *list_head; elt != NULL; elt = elt->next) {
		if(strcmp(elt->descURL, dev->descURL) == 0) {
			for(i = 0; i < elt->count; i++) {
				if (strcmp(elt->array[i]->st, dev->st) == 0 && strcmp(elt->array[i]->usn, dev->usn) == 0) {
					return;	/* already found */
				}
			}
			if(elt->count >= elt->allocated_count) {
				struct UPNPDev * * tmp;
				elt->allocated_count += ADD_DEVICE_COUNT_STEP;
				tmp = realloc(elt->array, elt->allocated_count * sizeof(struct UPNPDev *));
				if(tmp == NULL) {
					LogCModule( MODULE_PORT_FORWARD, LOG_ERROR, "Failed to realloc(%p, %lu)\n", elt->array, (unsigned long)(elt->allocated_count * sizeof(struct UPNPDev *)));
					return;
				}
				elt->array = tmp;
			}
			elt->array[elt->count++] = dev;
			return;
		}
	}
	elt = malloc(sizeof(struct upnp_dev_list));
	if(elt == NULL) {
		LogCModule( MODULE_PORT_FORWARD, LOG_ERROR, "Failed to malloc(%lu)\n", (unsigned long)sizeof(struct upnp_dev_list));
		return;
	}
	elt->next = *list_head;
	elt->descURL = strdup(dev->descURL);
	if(elt->descURL == NULL) {
		LogCModule( MODULE_PORT_FORWARD, LOG_ERROR, "Failed to strdup(%s)\n", dev->descURL);
		free(elt);
		return;
	}
	elt->allocated_count = ADD_DEVICE_COUNT_STEP;
	elt->array = malloc(ADD_DEVICE_COUNT_STEP * sizeof(struct UPNPDev *));
	if(elt->array == NULL) {
		LogCModule( MODULE_PORT_FORWARD, LOG_ERROR, "Failed to malloc(%lu)\n", (unsigned long)(ADD_DEVICE_COUNT_STEP * sizeof(struct UPNPDev *)));
		free(elt->descURL);
		free(elt);
		return;
	}
	elt->array[0] = dev;
	elt->count = 1;
	*list_head = elt;
}

void free_device(struct upnp_dev_list * elt)
{
	free(elt->descURL);
	free(elt->array);
	free(elt);
}

int main(int argc, char * * argv)
{
	const char * searched_device = NULL;
	const char * * searched_devices = NULL;
	const char * multicastif = 0;
	const char * minissdpdpath = 0;
	int ipv6 = 0;
	unsigned char ttl = 2;
	int error = 0;
	struct UPNPDev * devlist = 0;
	struct UPNPDev * dev;
	struct upnp_dev_list * sorted_list = NULL;
	struct upnp_dev_list * dev_array;
	int i;

#ifdef _WIN32
	WSADATA wsaData;
	int nResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if(nResult != NO_ERROR)
	{
		LogCModule( MODULE_PORT_FORWARD, LOG_ERROR, "WSAStartup() failed.\n");
		return -1;
	}
#endif

	for(i = 1; i < argc; i++) {
		if(strcmp(argv[i], "-6") == 0)
			ipv6 = 1;
		else if(strcmp(argv[i], "-d") == 0) {
			if(++i >= argc) {
				LogCModule( MODULE_PORT_FORWARD, LOG_ERROR, "%s option needs one argument\n", "-d");
				return 1;
			}
			searched_device = argv[i];
		} else if(strcmp(argv[i], "-t") == 0) {
			if(++i >= argc) {
				LogCModule( MODULE_PORT_FORWARD, LOG_ERROR, "%s option needs one argument\n", "-t");
				return 1;
			}
			ttl = (unsigned char)atoi(argv[i]);
		} else if(strcmp(argv[i], "-l") == 0) {
			if(++i >= argc) {
				LogCModule( MODULE_PORT_FORWARD, LOG_ERROR, "-l option needs at least one argument\n");
				return 1;
			}
			searched_devices = (const char * *)(argv + i);
			break;
		} else if(strcmp(argv[i], "-m") == 0) {
			if(++i >= argc) {
				LogCModule( MODULE_PORT_FORWARD, LOG_ERROR, "-m option needs one argument\n");
				return 1;
			}
			multicastif = argv[i];
		} else {
			LogCModule( MODULE_PORT_FORWARD, LOG_DEBUG, "usage : %s [options] [-l <device1> <device2> ...]\n", argv[0]);
			LogCModule( MODULE_PORT_FORWARD, LOG_DEBUG, "options :\n");
			LogCModule( MODULE_PORT_FORWARD, LOG_DEBUG, "   -6 : use IPv6\n");
			LogCModule( MODULE_PORT_FORWARD, LOG_DEBUG, "   -m address/ifname : network interface to use for multicast\n");
			LogCModule( MODULE_PORT_FORWARD, LOG_DEBUG, "   -d <device string> : search only for this type of device\n");
			LogCModule( MODULE_PORT_FORWARD, LOG_DEBUG, "   -l <device1> <device2> ... : search only for theses types of device\n");
			LogCModule( MODULE_PORT_FORWARD, LOG_DEBUG, "   -t ttl : set multicast TTL. Default value is 2.\n");
			LogCModule( MODULE_PORT_FORWARD, LOG_DEBUG, "   -h : this help\n");
			return 1;
		}
	}

	if(searched_device) {
		LogCModule( MODULE_PORT_FORWARD, LOG_DEBUG, "searching UPnP device type %s\n", searched_device);
		devlist = upnpDiscoverDevice(searched_device,
		                             2000, multicastif, minissdpdpath,
		                             0/*localport*/, ipv6, ttl, &error);
	} else if(searched_devices) {
		LogCModule( MODULE_PORT_FORWARD, LOG_DEBUG, "searching UPnP device types :\n");
		for(i = 0; searched_devices[i]; i++)
			LogCModule( MODULE_PORT_FORWARD, LOG_DEBUG, "\t%s\n", searched_devices[i]);
		devlist = upnpDiscoverDevices(searched_devices,
		                              2000, multicastif, minissdpdpath,
		                              0/*localport*/, ipv6, ttl, &error, 1);
	} else {
		LogCModule( MODULE_PORT_FORWARD, LOG_DEBUG, "searching all UPnP devices\n");
		devlist = upnpDiscoverAll(2000, multicastif, minissdpdpath,
		                             0/*localport*/, ipv6, ttl, &error);
	}
	if(devlist) {
		for(dev = devlist, i = 1; dev != NULL; dev = dev->pNext, i++) {
			LogCModule( MODULE_PORT_FORWARD, LOG_DEBUG, "%3d: %-48s\n", i, dev->st);
			LogCModule( MODULE_PORT_FORWARD, LOG_DEBUG, "     %s\n", dev->descURL);
			LogCModule( MODULE_PORT_FORWARD, LOG_DEBUG, "     %s\n", dev->usn);
			add_device(&sorted_list, dev);
		}
		putchar('\n');
		for (dev_array = sorted_list; dev_array != NULL ; dev_array = dev_array->next) {
			LogCModule( MODULE_PORT_FORWARD, LOG_DEBUG, "%s :\n", dev_array->descURL);
			for(i = 0; (unsigned)i < dev_array->count; i++) {
				LogCModule( MODULE_PORT_FORWARD, LOG_DEBUG, "%2d: %s\n", i+1, dev_array->array[i]->st);
				LogCModule( MODULE_PORT_FORWARD, LOG_DEBUG, "    %s\n", dev_array->array[i]->usn);
			}
			putchar('\n');
		}
		freeUPNPDevlist(devlist);
		while(sorted_list != NULL) {
			dev_array = sorted_list;
			sorted_list = sorted_list->next;
			free_device(dev_array);
		}
	} else {
		LogCModule( MODULE_PORT_FORWARD, LOG_ERROR,"No UPnP devices found\n");
		return 1;
	}

	return 0;
}

