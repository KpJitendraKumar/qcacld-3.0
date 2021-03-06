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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

#ifndef WMA_API_H
#define WMA_API_H

#include "osdep.h"
#include "cds_mq.h"
#include "ani_global.h"
#include "a_types.h"
#include "wmi_unified.h"
#ifdef NOT_YET
#include "htc_api.h"
#endif
#include "lim_global.h"

typedef void *WMA_HANDLE;

/**
 * enum GEN_PARAM - general parameters
 * @GEN_VDEV_PARAM_AMPDU: Set ampdu size
 * @GEN_VDEV_PARAM_AMSDU: Set amsdu size
 * @GEN_PARAM_DUMP_AGC_START: dump agc start
 * @GEN_PARAM_DUMP_AGC: dump agc
 * @GEN_PARAM_DUMP_CHANINFO_START: dump channel info start
 * @GEN_PARAM_DUMP_CHANINFO: dump channel info
 * @GEN_PARAM_CRASH_INJECT: inject crash
 * @GEN_PARAM_DUMP_PCIE_ACCESS_LOG: dump pcie access log
 * @GEN_PARAM_TX_CHAIN_MASK_CCK: cck tx chain mask
 */
typedef enum {
	GEN_VDEV_PARAM_AMPDU = 0x1,
	GEN_VDEV_PARAM_AMSDU,
	GEN_PARAM_DUMP_AGC_START,
	GEN_PARAM_DUMP_AGC,
	GEN_PARAM_DUMP_CHANINFO_START,
	GEN_PARAM_DUMP_CHANINFO,
	GEN_PARAM_DUMP_WATCHDOG,
	GEN_PARAM_CRASH_INJECT,
#ifdef CONFIG_ATH_PCIE_ACCESS_DEBUG
	GEN_PARAM_DUMP_PCIE_ACCESS_LOG,
#endif
	GEN_PARAM_MODULATED_DTIM,
} GEN_PARAM;

#define VDEV_CMD 1
#define PDEV_CMD 2
#define GEN_CMD  3
#define DBG_CMD  4
#define PPS_CMD  5
#define QPOWER_CMD 6
#define GTX_CMD  7

typedef void (*wma_peer_authorized_fp) (uint32_t vdev_id);


CDF_STATUS wma_pre_start(void *cds_context);

CDF_STATUS wma_mc_process_msg(void *cds_context, cds_msg_t *msg);

CDF_STATUS wma_start(void *cds_context);

CDF_STATUS wma_stop(void *cds_context, uint8_t reason);

CDF_STATUS wma_close(void *cds_context);

CDF_STATUS wma_wmi_service_close(void *cds_context);

CDF_STATUS wma_wmi_work_close(void *cds_context);

void wma_rx_ready_event(WMA_HANDLE handle, void *ev);

void wma_rx_service_ready_event(WMA_HANDLE handle, void *ev);

void wma_rx_service_ready_ext_event(WMA_HANDLE handle, void *ev);

void wma_setneedshutdown(void *cds_context);

bool wma_needshutdown(void *cds_context);

CDF_STATUS wma_wait_for_ready_event(WMA_HANDLE handle);

uint8_t wma_map_channel(uint8_t mapChannel);

int wma_cli_get_command(int vdev_id, int param_id, int vpdev);
int wma_cli_set_command(int vdev_id, int param_id, int sval, int vpdev);
int wma_cli_set2_command(int vdev_id, int param_id, int sval1,
			 int sval2, int vpdev);

CDF_STATUS wma_set_htconfig(uint8_t vdev_id, uint16_t ht_capab, int value);
CDF_STATUS wma_set_reg_domain(void *clientCtxt, v_REGDOMAIN_t regId);

CDF_STATUS wma_get_wcnss_software_version(void *p_cds_gctx,
					  uint8_t *pVersion,
					  uint32_t versionBufferSize);
int wma_bus_suspend(void);
int wma_suspend_target(WMA_HANDLE handle, int disable_target_intr);
void wma_target_suspend_acknowledge(void *context);
int wma_bus_resume(void);
int wma_resume_target(WMA_HANDLE handle);
CDF_STATUS wma_disable_wow_in_fw(WMA_HANDLE handle);
CDF_STATUS wma_disable_d0wow_in_fw(WMA_HANDLE handle);
int wma_is_wow_mode_selected(WMA_HANDLE handle);
CDF_STATUS wma_enable_wow_in_fw(WMA_HANDLE handle);
CDF_STATUS wma_enable_d0wow_in_fw(WMA_HANDLE handle);
bool wma_check_scan_in_progress(WMA_HANDLE handle);
void wma_set_peer_authorized_cb(void *wma_ctx, wma_peer_authorized_fp auth_cb);
int wma_set_peer_param(void *wma_ctx, uint8_t *peer_addr, uint32_t param_id,
		       uint32_t param_value, uint32_t vdev_id);
#ifdef NOT_YET
CDF_STATUS wma_update_channel_list(WMA_HANDLE handle, void *scan_chan_info);
#endif

uint8_t *wma_get_vdev_address_by_vdev_id(uint8_t vdev_id);
struct wma_txrx_node *wma_get_interface_by_vdev_id(uint8_t vdev_id);
bool wma_is_vdev_up(uint8_t vdev_id);

void *wma_get_beacon_buffer_by_vdev_id(uint8_t vdev_id, uint32_t *buffer_size);

uint8_t wma_get_fw_wlan_feat_caps(uint8_t featEnumValue);
tSirRetStatus wma_post_ctrl_msg(tpAniSirGlobal pMac, tSirMsgQ *pMsg);

void wma_enable_disable_wakeup_event(WMA_HANDLE handle,
				uint32_t vdev_id,
				uint32_t bitmap,
				bool enable);
void wma_register_wow_wakeup_events(WMA_HANDLE handle, uint8_t vdev_id,
					uint8_t vdev_type, uint8_t sub_type);
void wma_register_wow_default_patterns(WMA_HANDLE handle, uint8_t vdev_id);
int8_t wma_get_hw_mode_idx_from_dbs_hw_list(enum hw_mode_ss_config mac0_ss,
		enum hw_mode_bandwidth mac0_bw,
		enum hw_mode_ss_config mac1_ss,
		enum hw_mode_bandwidth mac1_bw,
		enum hw_mode_dbs_capab dbs,
		enum hw_mode_agile_dfs_capab dfs);
CDF_STATUS wma_get_hw_mode_from_idx(uint32_t idx,
		struct sir_hw_mode_params *hw_mode);
int8_t wma_get_num_dbs_hw_modes(void);
bool wma_is_hw_dbs_capable(void);
bool wma_is_hw_agile_dfs_capable(void);
int8_t wma_get_mac_id_of_vdev(uint32_t vdev_id);
void wma_update_intf_hw_mode_params(uint32_t vdev_id, uint32_t mac_id,
				uint32_t cfgd_hw_mode_index);
CDF_STATUS wma_get_old_and_new_hw_index(uint32_t *old_hw_mode_index,
		uint32_t *new_hw_mode_index);
void wma_set_dbs_capability_ut(uint32_t dbs);
CDF_STATUS wma_get_dbs_hw_modes(bool *one_by_one_dbs, bool *two_by_two_dbs);
CDF_STATUS wma_get_current_hw_mode(struct sir_hw_mode_params *hw_mode);
bool wma_is_dbs_enable(void);
bool wma_is_agile_dfs_enable(void);
CDF_STATUS wma_get_updated_scan_config(uint32_t *scan_config,
		bool dbs_scan,
		bool dbs_plus_agile_scan,
		bool single_mac_scan_with_dfs);
CDF_STATUS wma_get_updated_fw_mode_config(uint32_t *fw_mode_config,
		bool dbs,
		bool agile_dfs);
bool wma_get_dbs_scan_config(void);
bool wma_get_dbs_plus_agile_scan_config(void);
bool wma_get_single_mac_scan_with_dfs_config(void);
bool wma_get_dbs_config(void);
bool wma_get_agile_dfs_config(void);
bool wma_is_dual_mac_disabled_in_ini(void);
bool wma_get_prev_dbs_config(void);
bool wma_get_prev_agile_dfs_config(void);
bool wma_get_prev_dbs_scan_config(void);
bool wma_get_prev_dbs_plus_agile_scan_config(void);
bool wma_get_prev_single_mac_scan_with_dfs_config(void);

#define LRO_IPV4_SEED_ARR_SZ 5
#define LRO_IPV6_SEED_ARR_SZ 11

/**
 * struct wma_lro_init_cmd_t - set LRO init parameters
 * @lro_enable: indicates whether lro is enabled
 * @tcp_flag: If the TCP flags from the packet do not match
 * the values in this field after masking with TCP flags mask
 * below, packet is not LRO eligible
 * @tcp_flag_mask: field for comparing the TCP values provided
 * above with the TCP flags field in the received packet
 * @toeplitz_hash_ipv4: contains seed needed to compute the flow id
 * 5-tuple toeplitz hash for ipv4 packets
 * @toeplitz_hash_ipv6: contains seed needed to compute the flow id
 * 5-tuple toeplitz hash for ipv6 packets
 */
struct wma_lro_config_cmd_t {
	uint32_t lro_enable;
	uint32_t tcp_flag:9,
		tcp_flag_mask:9;
	uint32_t toeplitz_hash_ipv4[LRO_IPV4_SEED_ARR_SZ];
	uint32_t toeplitz_hash_ipv6[LRO_IPV6_SEED_ARR_SZ];
};

#if defined(FEATURE_LRO)
int wma_lro_init(struct wma_lro_config_cmd_t *lro_config);
#endif
bool wma_is_scan_simultaneous_capable(void);
#endif
