/*
 * Copyright (c) 2012-2015 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC : wlan_hdd_scan.h
 *
 * WLAN Host Device Driver scan related implementation
 *
 */

#if !defined(WLAN_HDD_SCAN_H)
#define WLAN_HDD_SCAN_H

#include "wlan_hdd_main.h"

#define MAX_PENDING_LOG 5

/*
 * enum scan_source - scan request source
 *
 * @NL_SCAN: Scan initiated from NL
 * @VENDOR_SCAN: Scan intiated from vendor command
*/
enum scan_source {
	NL_SCAN,
	VENDOR_SCAN,
};

int iw_get_scan(struct net_device *dev, struct iw_request_info *info,
		union iwreq_data *wrqu, char *extra);

int iw_set_scan(struct net_device *dev, struct iw_request_info *info,
		union iwreq_data *wrqu, char *extra);

int wlan_hdd_cfg80211_scan(struct wiphy *wiphy,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 6, 0))
			   struct net_device *dev,
#endif
			   struct cfg80211_scan_request *request);

#ifdef FEATURE_WLAN_SCAN_PNO
int wlan_hdd_cfg80211_sched_scan_start(struct wiphy *wiphy,
				       struct net_device *dev,
				       struct cfg80211_sched_scan_request
				       *request);

int wlan_hdd_cfg80211_sched_scan_stop(struct wiphy *wiphy,
				      struct net_device *dev);
#endif /* End of FEATURE_WLAN_SCAN_PNO */

int wlan_hdd_cfg80211_vendor_scan(struct wiphy *wiphy,
		struct wireless_dev *wdev, const void *data,
		int data_len);

int hdd_vendor_put_ifindex(struct sk_buff *skb, int ifindex);
#endif /* end #if !defined(WLAN_HDD_SCAN_H) */

