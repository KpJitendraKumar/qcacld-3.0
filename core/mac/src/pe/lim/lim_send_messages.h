/*
 * Copyright (c) 2011-2015 The Linux Foundation. All rights reserved.
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/*
 *
 * lim_send_messages.h: Provides functions to send messages or Indications to HAL.
 * Author:    Sunit Bhatia
 * Date:       09/21/2006
 * History:-
 * Date        Modified by            Modification Information
 *
 * --------------------------------------------------------------------------
 *
 */
#ifndef __LIM_SEND_MESSAGES_H
#define __LIM_SEND_MESSAGES_H

#include "ani_global.h"
#include "lim_types.h"
#include "wma_if.h"
#include "sir_params.h"
tSirRetStatus lim_send_cf_params(tpAniSirGlobal pMac, uint8_t bssIdx,
				 uint8_t cfpCount, uint8_t cfpPeriod);
tSirRetStatus lim_send_beacon_params(tpAniSirGlobal pMac,
				     tpUpdateBeaconParams pUpdatedBcnParams,
				     tpPESession psessionEntry);
/* tSirRetStatus lim_send_beacon_params(tpAniSirGlobal pMac, tpUpdateBeaconParams pUpdatedBcnParams); */
#ifdef WLAN_FEATURE_11AC
tSirRetStatus lim_send_mode_update(tpAniSirGlobal pMac,
				   tUpdateVHTOpMode *tempParam,
				   tpPESession psessionEntry);
tSirRetStatus lim_send_rx_nss_update(tpAniSirGlobal pMac,
				     tUpdateRxNss *tempParam,
				     tpPESession psessionEntry);

uint32_t lim_get_center_channel(tpAniSirGlobal pMac,
				uint8_t primarychanNum,
				ePhyChanBondState secondaryChanOffset,
				uint8_t chanWidth);

tSirRetStatus lim_set_membership(tpAniSirGlobal pMac,
				 tUpdateMembership *pTempParam,
				 tpPESession psessionEntry);

tSirRetStatus lim_set_user_pos(tpAniSirGlobal pMac,
			       tUpdateUserPos *pTempParam,
			       tpPESession psessionEntry);
#endif
#if defined WLAN_FEATURE_VOWIFI
tSirRetStatus lim_send_switch_chnl_params(tpAniSirGlobal pMac,
					  uint8_t chnlNumber,
					  uint8_t ch_center_freq_seg0,
					  uint8_t ch_center_freq_seg1,
					  phy_ch_width ch_width,
					  tPowerdBm maxTxPower,
					  uint8_t peSessionId,
					  uint8_t is_restart);
#else
tSirRetStatus lim_send_switch_chnl_params(tpAniSirGlobal pMac,
					  uint8_t chnlNumber,
					  uint8_t ch_center_freq_seg0,
					  uint8_t ch_center_freq_seg1,
					  phy_ch_width ch_width,
					  uint8_t localPwrConstraint,
					  uint8_t peSessionId,
					  uint8_t is_restart);
#endif
tSirRetStatus lim_send_edca_params(tpAniSirGlobal pMac,
				   tSirMacEdcaParamRecord *pUpdatedEdcaParams,
				   uint16_t bssIdx);
tSirRetStatus lim_set_link_state(tpAniSirGlobal pMac, tSirLinkState state,
				 tSirMacAddr bssId, tSirMacAddr selfMac,
				 tpSetLinkStateCallback callback,
				 void *callbackArg);
#ifdef WLAN_FEATURE_VOWIFI_11R
extern tSirRetStatus lim_set_link_state_ft(tpAniSirGlobal pMac, tSirLinkState
					   state, tSirMacAddr bssId,
					   tSirMacAddr selfMacAddr, int ft,
					   tpPESession psessionEntry);
#endif
void lim_set_active_edca_params(tpAniSirGlobal pMac,
				tSirMacEdcaParamRecord *plocalEdcaParams,
				tpPESession psessionEntry);
#define CAPABILITY_FILTER_MASK  0x73CF
#define ERP_FILTER_MASK         0xF8
#define EDCA_FILTER_MASK        0xF0
#define QOS_FILTER_MASK         0xF0
#define HT_BYTE0_FILTER_MASK    0x0
#define HT_BYTE2_FILTER_MASK    0xEB
#define HT_BYTE5_FILTER_MASK    0xFD
#define DS_PARAM_CHANNEL_MASK   0x0
#define VHTOP_CHWIDTH_MASK      0xFC

tSirRetStatus lim_send_beacon_filter_info(tpAniSirGlobal pMac,
					  tpPESession psessionEntry);

#ifdef WLAN_FEATURE_11W
tSirRetStatus lim_send_exclude_unencrypt_ind(tpAniSirGlobal pMac,
					     bool excludeUnenc,
					     tpPESession psessionEntry);
#endif
#endif
