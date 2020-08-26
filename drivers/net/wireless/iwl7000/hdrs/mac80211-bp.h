/*
 * ChromeOS backport definitions
 * Copyright (C) 2015-2017 Intel Deutschland GmbH
 * Copyright (C) 2018-2020 Intel Corporation
 */
#include <linux/if_ether.h>
#include <net/cfg80211.h>
#include <linux/errqueue.h>
#include <generated/utsrelease.h>
/* ipv6_addr_is_multicast moved - include old header */
#include <net/addrconf.h>
#include <net/ieee80211_radiotap.h>
#include <crypto/hash.h>

/* make sure we include iw_handler.h to get wireless_nlevent_flush() */
#include <net/iw_handler.h>

/* common backward compat code */

#define BACKPORTS_GIT_TRACKED "chromium:" UTS_RELEASE
#define BACKPORTS_BUILD_TSTAMP __DATE__ " " __TIME__

/* Dummy RHEL macros */
#define RHEL_RELEASE_CODE 0
#define RHEL_RELEASE_VERSION(a,b) 1

#ifndef netdev_alloc_pcpu_stats
#define netdev_alloc_pcpu_stats(type)				\
({								\
	typeof(type) __percpu *pcpu_stats = alloc_percpu(type); \
	if (pcpu_stats)	{					\
		int i;						\
		for_each_possible_cpu(i) {			\
			typeof(type) *stat;			\
			stat = per_cpu_ptr(pcpu_stats, i);	\
			u64_stats_init(&stat->syncp);		\
		}						\
	}							\
	pcpu_stats;						\
})
#endif /* netdev_alloc_pcpu_stats */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,13,0)
/* consider properly backporting this? */
static inline int crypto_memneq(const void *a, const void *b, size_t size)
{
	unsigned long neq = 0;

#if defined(CONFIG_HAVE_EFFICIENT_UNALIGNED_ACCESS)
	while (size >= sizeof(unsigned long)) {
		neq |= *(unsigned long *)a ^ *(unsigned long *)b;
		/* OPTIMIZER_HIDE_VAR(neq); */
		barrier();
		a += sizeof(unsigned long);
		b += sizeof(unsigned long);
		size -= sizeof(unsigned long);
	}
#endif /* CONFIG_HAVE_EFFICIENT_UNALIGNED_ACCESS */
	while (size > 0) {
		neq |= *(unsigned char *)a ^ *(unsigned char *)b;
		/* OPTIMIZER_HIDE_VAR(neq); */
		barrier();
		a += 1;
		b += 1;
		size -= 1;
	}
	return neq != 0UL ? 1 : 0;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,14,0)

static inline u32 reciprocal_scale(u32 val, u32 ep_ro)
{
	return (u32)(((u64) val * ep_ro) >> 32);
}

#include "u64_stats_sync.h"

struct pcpu_sw_netstats {
	u64     rx_packets;
	u64     rx_bytes;
	u64     tx_packets;
	u64     tx_bytes;
	struct u64_stats_sync   syncp;
};

#define netdev_tstats(dev)	((struct pcpu_sw_netstats *)dev->ml_priv)
#define netdev_assign_tstats(dev, e)	dev->ml_priv = (e);
#else
#define netdev_tstats(dev)	dev->tstats
#define netdev_assign_tstats(dev, e)	dev->tstats = (e);
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3,14,0) */

#ifndef BIT_ULL
#define BIT_ULL(nr) (1ULL << (nr))
#endif

#ifndef GENMASK
#define GENMASK(h, l)		(((U32_C(1) << ((h) - (l) + 1)) - 1) << (l))
#define GENMASK_ULL(h, l)	(((U64_C(1) << ((h) - (l) + 1)) - 1) << (l))
#endif

static inline void netdev_attach_ops(struct net_device *dev,
				     const struct net_device_ops *ops)
{
	dev->netdev_ops = ops;
}

#define WIPHY_FLAG_HAS_CHANNEL_SWITCH 0

#define WIPHY_PARAM_DYN_ACK		(1 << 5)

#define mc_addr(ha)	(ha)->addr

#define NL80211_FEATURE_STATIC_SMPS		(1 << 24)
#define NL80211_FEATURE_DYNAMIC_SMPS		(1 << 25)
#define NL80211_FEATURE_SUPPORTS_WMM_ADMISSION	(1 << 26)
/* cannot be supported on this kernel */
#define NL80211_FEATURE_TDLS_CHANNEL_SWITCH	0

#if CFG80211_VERSION < KERNEL_VERSION(3,9,0)
struct wiphy_wowlan_tcp_support {
	const struct nl80211_wowlan_tcp_data_token_feature *tok;
	u32 data_payload_max;
	u32 data_interval_max;
	u32 wake_payload_max;
	bool seq;
};

struct cfg80211_wowlan_tcp {
	struct socket *sock;
	__be32 src, dst;
	u16 src_port, dst_port;
	u8 dst_mac[ETH_ALEN];
	int payload_len;
	const u8 *payload;
	struct nl80211_wowlan_tcp_data_seq payload_seq;
	u32 data_interval;
	u32 wake_len;
	const u8 *wake_data, *wake_mask;
	u32 tokens_size;
	/* must be last, variable member */
	struct nl80211_wowlan_tcp_data_token payload_tok;
};

static inline const struct ieee80211_reg_rule *
LINUX_BACKPORT(freq_reg_info)(struct wiphy *wiphy, u32 center_freq)
{
	const struct ieee80211_reg_rule *rule = NULL;
	int err = freq_reg_info(wiphy, center_freq, MHZ_TO_KHZ(20), &rule);

	if (err)
		return ERR_PTR(err);
	return rule;
}
#define freq_reg_info LINUX_BACKPORT(freq_reg_info)
#endif /* CFG80211_VERSION < KERNEL_VERSION(3,9,0) */

#if CFG80211_VERSION < KERNEL_VERSION(3,10,0)
#define NL80211_FEATURE_USERSPACE_MPM 0

enum cfg80211_station_type {
	CFG80211_STA_AP_CLIENT,
	CFG80211_STA_AP_MLME_CLIENT,
	CFG80211_STA_AP_STA,
	CFG80211_STA_IBSS,
	CFG80211_STA_TDLS_PEER_SETUP,
	CFG80211_STA_TDLS_PEER_ACTIVE,
	CFG80211_STA_MESH_PEER_KERNEL,
	CFG80211_STA_MESH_PEER_USER,
};

static inline int
cfg80211_check_station_change(struct wiphy *wiphy,
			      struct station_parameters *params,
			      enum cfg80211_station_type type)
{
	return 0;
}
#endif /* CFG80211_VERSION < KERNEL_VERSION(3,10,0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,11,0)
#define IEEE80211_RADIOTAP_MCS_HAVE_STBC 0
#endif

#if CFG80211_VERSION < KERNEL_VERSION(3,11,0)
#define NL80211_CHAN_WIDTH_5	6
#define NL80211_CHAN_WIDTH_10	7

#define IEEE80211_MAX_CHAINS 4

#define MONITOR_FLAG_ACTIVE 0

static inline void cfg80211_rx_unprot_mlme_mgmt(struct net_device *dev,
						void *data, int len)
{
	struct ieee80211_hdr *hdr = data;

	if (ieee80211_is_deauth(hdr->frame_control))
		cfg80211_send_unprot_deauth(dev, data, len);
	else
		cfg80211_send_unprot_disassoc(dev, data, len);
}

static inline void cfg80211_tx_mlme_mgmt(struct net_device *dev,
					 const void *data, int len)
{
	const struct ieee80211_hdr *hdr = data;

	if (ieee80211_is_deauth(hdr->frame_control))
		cfg80211_send_deauth(dev, data, len);
	else
		cfg80211_send_disassoc(dev, data, len);
}

static inline void cfg80211_rx_mlme_mgmt(struct net_device *dev,
					 const void *data, int len)
{
	const struct ieee80211_hdr *hdr = data;

	if (ieee80211_is_auth(hdr->frame_control))
		cfg80211_send_rx_auth(dev, data, len);
	else if (ieee80211_is_deauth(hdr->frame_control))
		cfg80211_send_deauth(dev, data, len);
	else
		cfg80211_send_disassoc(dev, data, len);
}

static inline void cfg80211_assoc_timeout(struct net_device *dev,
					  struct cfg80211_bss *bss)
{
	cfg80211_send_assoc_timeout(dev, bss->bssid);
}

static inline void cfg80211_auth_timeout(struct net_device *dev,
					 const u8 *bssid)
{
	cfg80211_send_auth_timeout(dev, bssid);
}

static inline void cfg80211_rx_assoc_resp(struct net_device *dev,
					  struct cfg80211_bss *bss,
					  void *data, int len,
					  int uapsd_queues)
{
	cfg80211_send_rx_assoc(dev, bss, data, len);
}

static inline enum ieee80211_rate_flags
ieee80211_chandef_rate_flags(struct cfg80211_chan_def *chandef)
{
	return 0;
}

#define IEEE80211_RADIOTAP_MCS_STBC_SHIFT	5

/* on older versions this is safe - no RTNL use there */
#define cfg80211_sched_scan_stopped_rtnl(wiphy, reqid)	\
	cfg80211_sched_scan_stopped(wiphy, 0)
#endif /* CFG80211_VERSION < KERNEL_VERSION(3,11,0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,12,0)
#define IEEE80211_CHAN_HALF 0
#define IEEE80211_CHAN_QUARTER 0
#endif

#if CFG80211_VERSION < KERNEL_VERSION(3,12,0)
#define NL80211_BSS_CHAN_WIDTH_20	0
#define NL80211_BSS_CHAN_WIDTH_10	1
#define NL80211_BSS_CHAN_WIDTH_5	2
/* use a macro to be compatible with scan_request and sched_scan_request */
#define cfg_scan_req_width(req) NL80211_BSS_CHAN_WIDTH_20

static inline int
ieee80211_chandef_max_power(struct cfg80211_chan_def *chandef)
{
	return chandef->chan->max_power;
}

static inline struct cfg80211_bss * __must_check
cfg80211_inform_bss_width_frame(struct wiphy *wiphy,
				struct ieee80211_channel *channel,
				enum nl80211_bss_scan_width scan_width,
				struct ieee80211_mgmt *mgmt, size_t len,
				s32 signal, gfp_t gfp)
{
	return cfg80211_inform_bss_frame(wiphy, channel, mgmt,
					 len, signal, gfp);
}

static inline enum nl80211_bss_scan_width
cfg80211_chandef_to_scan_width(const struct cfg80211_chan_def *chandef)
{
	return NL80211_BSS_CHAN_WIDTH_20;
}

static inline bool
iwl7000_cfg80211_rx_mgmt(struct wireless_dev *wdev, int freq, int sig_dbm,
			 const u8 *buf, size_t len, u32 flags)
{
	return cfg80211_rx_mgmt(wdev, freq, sig_dbm, buf, len, GFP_ATOMIC);
}
#define cfg80211_rx_mgmt iwl7000_cfg80211_rx_mgmt

struct cfg80211_csa_settings {
	struct cfg80211_chan_def chandef;
	struct cfg80211_beacon_data beacon_csa;
	u16 counter_offset_beacon, counter_offset_presp;
	struct cfg80211_beacon_data beacon_after;
	bool radar_required;
	bool block_tx;
	u8 count;
};

static inline u32
ieee80211_mandatory_rates(struct ieee80211_supported_band *sband)
{
	struct ieee80211_rate *bitrates;
	u32 mandatory_rates = 0;
	enum ieee80211_rate_flags mandatory_flag;
	int i;

	if (WARN_ON(!sband))
		return 1;

	if (sband->band == NL80211_BAND_2GHZ)
		mandatory_flag = IEEE80211_RATE_MANDATORY_B;
	else
		mandatory_flag = IEEE80211_RATE_MANDATORY_A;

	bitrates = sband->bitrates;
	for (i = 0; i < sband->n_bitrates; i++)
		if (bitrates[i].flags & mandatory_flag)
			mandatory_rates |= BIT(i);
	return mandatory_rates;
}

#define ieee80211_mandatory_rates(sband, width) ieee80211_mandatory_rates(sband)
#else
#define cfg_scan_req_width(req) ((req)->scan_width)
#endif /* CFG80211_VERSION < KERNEL_VERSION(3,12,0) */

#if CFG80211_VERSION < KERNEL_VERSION(3,13,0)
static inline int cfg80211_chandef_get_width(const struct cfg80211_chan_def *c)
{
	int width;

	switch (c->width) {
	case NL80211_CHAN_WIDTH_20:
	case NL80211_CHAN_WIDTH_20_NOHT:
		width = 20;
		break;
	case NL80211_CHAN_WIDTH_40:
		width = 40;
		break;
	case NL80211_CHAN_WIDTH_80P80:
	case NL80211_CHAN_WIDTH_80:
		width = 80;
		break;
	case NL80211_CHAN_WIDTH_160:
		width = 160;
		break;
	default:
		WARN_ON_ONCE(1);
		return -1;
	}
	return width;
}

static inline int cfg80211_get_chans_dfs_required(struct wiphy *wiphy,
						  u32 center_freq,
						  u32 bandwidth)
{
	struct ieee80211_channel *c;
	u32 freq, start_freq, end_freq;

	if (bandwidth <= 20) {
		start_freq = center_freq;
		end_freq = center_freq;
	} else {
		start_freq = center_freq - bandwidth/2 + 10;
		end_freq = center_freq + bandwidth/2 - 10;
	}

	for (freq = start_freq; freq <= end_freq; freq += 20) {
		c = ieee80211_get_channel(wiphy, freq);
		if (!c)
			return -EINVAL;

		if (c->flags & IEEE80211_CHAN_RADAR)
			return 1;
	}
	return 0;
}

#define cfg80211_radar_event(...) do { } while (0)
static inline bool
cfg80211_ibss_userspace_handles_dfs(struct cfg80211_ibss_params *params)
{
	return false;
}
#else
static inline bool
cfg80211_ibss_userspace_handles_dfs(struct cfg80211_ibss_params *params)
{
	return params->userspace_handles_dfs;
}
#endif /* CFG80211_VERSION < KERNEL_VERSION(3,13,0) */

#if CFG80211_VERSION < KERNEL_VERSION(3,14,0)
struct cfg80211_mgmt_tx_params {
	struct ieee80211_channel *chan;
	bool offchan;
	unsigned int wait;
	const u8 *buf;
	size_t len;
	bool no_cck;
	bool dont_wait_for_ack;
};

#define regulatory_flags flags

#define REGULATORY_CUSTOM_REG WIPHY_FLAG_CUSTOM_REGULATORY
#define REGULATORY_DISABLE_BEACON_HINTS WIPHY_FLAG_DISABLE_BEACON_HINTS

#define IEEE80211_CHAN_NO_IR (IEEE80211_CHAN_PASSIVE_SCAN |\
			      IEEE80211_CHAN_NO_IBSS)

struct cfg80211_qos_map {
	u8 unused_dummy;
};

static inline unsigned int
bp_cfg80211_classify8021d(struct sk_buff *skb,
			  struct cfg80211_qos_map *qos_map)
{
	return cfg80211_classify8021d(skb);
}
#define cfg80211_classify8021d bp_cfg80211_classify8021d
#endif /* CFG80211_VERSION < KERNEL_VERSION(3,14,0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,15,0)
#define IEEE80211_RADIOTAP_CODING_LDPC_USER0	0x1
#endif

#if CFG80211_VERSION < KERNEL_VERSION(3,15,0)
#define cfg80211_ibss_joined(dev, bssid, chan, gfp) \
	cfg80211_ibss_joined(dev, bssid, gfp)

static inline bool wdev_cac_started(struct wireless_dev *wdev)
{
	return false;
}

static inline void
cfg80211_cac_event(struct net_device *netdev,
		   const struct cfg80211_chan_def *chandef,
		   enum nl80211_radar_event event, gfp_t gfp)
{
}
#else
static inline bool wdev_cac_started(struct wireless_dev *wdev)
{
	return wdev->cac_started;
}
#endif /* CFG80211_VERSION < KERNEL_VERSION(3,15,0) */

#if CFG80211_VERSION < KERNEL_VERSION(3,16,0)
#include <linux/utsname.h>

static inline void
cfg80211_get_drvinfo(struct net_device *dev, struct ethtool_drvinfo *info)
{
	struct wireless_dev *wdev = dev->ieee80211_ptr;

	strlcpy(info->driver, wiphy_dev(wdev->wiphy)->driver->name,
		sizeof(info->driver));

	strlcpy(info->version, init_utsname()->release, sizeof(info->version));

	if (wdev->wiphy->fw_version[0])
		strlcpy(info->fw_version, wdev->wiphy->fw_version,
			sizeof(info->fw_version));
	else
		strlcpy(info->fw_version, "N/A", sizeof(info->fw_version));

	strlcpy(info->bus_info, dev_name(wiphy_dev(wdev->wiphy)),
		sizeof(info->bus_info));
}

#define REGULATORY_ENABLE_RELAX_NO_IR 0

#define cfg80211_reg_can_beacon(wiphy, chandef, iftype) \
	cfg80211_reg_can_beacon(wiphy, chandef)

static inline int
cfg80211_chandef_dfs_required(struct wiphy *wiphy,
			      const struct cfg80211_chan_def *chandef,
			      enum nl80211_iftype iftype)
{
	int width;
	int ret;

	if (WARN_ON(!cfg80211_chandef_valid(chandef)))
		return -EINVAL;

	switch (iftype) {
	case NL80211_IFTYPE_ADHOC:
	case NL80211_IFTYPE_AP:
	case NL80211_IFTYPE_P2P_GO:
	case NL80211_IFTYPE_MESH_POINT:
		width = cfg80211_chandef_get_width(chandef);
		if (width < 0)
			return -EINVAL;

		ret = cfg80211_get_chans_dfs_required(wiphy,
						      chandef->center_freq1,
						      width);
		if (ret < 0)
			return ret;
		else if (ret > 0)
			return BIT(chandef->width);

		if (!chandef->center_freq2)
			return 0;

		ret = cfg80211_get_chans_dfs_required(wiphy,
						      chandef->center_freq2,
						      width);
		if (ret < 0)
			return ret;
		else if (ret > 0)
			return BIT(chandef->width);

		break;
	case NL80211_IFTYPE_STATION:
	case NL80211_IFTYPE_P2P_CLIENT:
	case NL80211_IFTYPE_MONITOR:
	case NL80211_IFTYPE_AP_VLAN:
	case NL80211_IFTYPE_WDS:
	case NL80211_IFTYPE_P2P_DEVICE:
		break;
	case NL80211_IFTYPE_UNSPECIFIED:
	case NUM_NL80211_IFTYPES:
		WARN_ON(1);
	}

	return 0;
}

static inline int
cfg80211_iter_combinations(struct wiphy *wiphy,
			   const int num_different_channels,
			   const u8 radar_detect,
			   const int iftype_num[NUM_NL80211_IFTYPES],
			   void (*iter)(const struct ieee80211_iface_combination *c,
					void *data),
			   void *data)
{
	int i, j, iftype;
	int num_interfaces = 0;
	u32 used_iftypes = 0;

	for (iftype = 0; iftype < NUM_NL80211_IFTYPES; iftype++) {
		num_interfaces += iftype_num[iftype];
		if (iftype_num[iftype] > 0 &&
		    !(wiphy->software_iftypes & BIT(iftype)))
			used_iftypes |= BIT(iftype);
	}

	for (i = 0; i < wiphy->n_iface_combinations; i++) {
		const struct ieee80211_iface_combination *c;
		struct ieee80211_iface_limit *limits;
		u32 all_iftypes = 0;

		c = &wiphy->iface_combinations[i];

		if (num_interfaces > c->max_interfaces)
			continue;
		if (num_different_channels > c->num_different_channels)
			continue;

		limits = kmemdup(c->limits, sizeof(limits[0]) * c->n_limits,
				 GFP_KERNEL);
		if (!limits)
			return -ENOMEM;

		for (iftype = 0; iftype < NUM_NL80211_IFTYPES; iftype++) {
			if (wiphy->software_iftypes & BIT(iftype))
				continue;
			for (j = 0; j < c->n_limits; j++) {
				all_iftypes |= limits[j].types;
				if (!(limits[j].types & BIT(iftype)))
					continue;
				if (limits[j].max < iftype_num[iftype])
					goto cont;
				limits[j].max -= iftype_num[iftype];
			}
		}

		if (radar_detect)
			goto cont;

		/* Finally check that all iftypes that we're currently
		 * using are actually part of this combination. If they
		 * aren't then we can't use this combination and have
		 * to continue to the next.
		 */
		if ((all_iftypes & used_iftypes) != used_iftypes)
			goto cont;

		/* This combination covered all interface types and
		 * supported the requested numbers, so we're good.
		 */

		(*iter)(c, data);
 cont:
		kfree(limits);
	}

	return 0;
}

static void
cfg80211_iter_sum_ifcombs(const struct ieee80211_iface_combination *c,
			  void *data)
{
	int *num = data;
	(*num)++;
}

static inline int
cfg80211_check_combinations(struct wiphy *wiphy,
			    const int num_different_channels,
			    const u8 radar_detect,
			    const int iftype_num[NUM_NL80211_IFTYPES])
{
	int err, num = 0;

	err = cfg80211_iter_combinations(wiphy, num_different_channels,
					 radar_detect, iftype_num,
					 cfg80211_iter_sum_ifcombs, &num);
	if (err)
		return err;
	if (num == 0)
		return -EBUSY;

	return 0;
}

#define const_since_3_16

static inline unsigned int
csa_n_counter_offsets_beacon(struct cfg80211_csa_settings *s)
{
	return 0;
}

static inline unsigned int
csa_n_counter_offsets_presp(struct cfg80211_csa_settings *s)
{
	return 0;
}

static inline const u16 *
csa_counter_offsets_beacon(struct cfg80211_csa_settings *s)
{
	return NULL;
}

static inline const u16 *
csa_counter_offsets_presp(struct cfg80211_csa_settings *s)
{
	return NULL;
}
#else
#define const_since_3_16 const

static inline unsigned int
csa_n_counter_offsets_beacon(struct cfg80211_csa_settings *s)
{
	return s->n_counter_offsets_beacon;
}

static inline unsigned int
csa_n_counter_offsets_presp(struct cfg80211_csa_settings *s)
{
	return s->n_counter_offsets_presp;
}

static inline const u16 *
csa_counter_offsets_beacon(struct cfg80211_csa_settings *s)
{
	return s->counter_offsets_beacon;
}

static inline const u16 *
csa_counter_offsets_presp(struct cfg80211_csa_settings *s)
{
	return s->counter_offsets_presp;
}
#endif /* CFG80211_VERSION < KERNEL_VERSION(3,16,0) */

#if CFG80211_VERSION < KERNEL_VERSION(3,18,0)
#define NL80211_FEATURE_QUIET 0
#define NL80211_FEATURE_TX_POWER_INSERTION 0
#define NL80211_FEATURE_DS_PARAM_SET_IE_IN_PROBES 0
#define NL80211_FEATURE_WFA_TPC_IE_IN_PROBES 0
#define ASSOC_REQ_USE_RRM 0
#define cfg80211_ap_settings_smps_mode(params) NL80211_SMPS_OFF
#else
#define cfg80211_ap_settings_smps_mode(params) ((params)->smps_mode)
#endif

#if CFG80211_VERSION <= KERNEL_VERSION(9,9,9)
#define IEEE80211_CHAN_NO_HE 0
#define NL80211_RRF_NO_HE 0
#endif

#if CFG80211_VERSION < KERNEL_VERSION(3,19,0)
#define NL80211_FEATURE_MAC_ON_CREATE 0 /* cannot be used */

struct ocb_setup {
	struct cfg80211_chan_def chandef;
};

static inline bool ieee80211_viftype_ocb(unsigned int iftype)
{
	return false;
}

static inline struct wiphy *
wiphy_new_nm(const struct cfg80211_ops *ops, int sizeof_priv,
	     const char *requested_name)
{
	/* drop the requested name since it's not supported */
	return wiphy_new(ops, sizeof_priv);
}

static inline void
cfg80211_cqm_beacon_loss_notify(struct net_device *dev, gfp_t gfp)
{
	/* don't do anything - this event is unused by the supplicant here */
}

/* LAR related functionality privately backported into mac80211 */
int regulatory_set_wiphy_regd(struct wiphy *wiphy,
			      struct ieee80211_regdomain *rd);
int regulatory_set_wiphy_regd_sync_rtnl(struct wiphy *wiphy,
					struct ieee80211_regdomain *rd);

#define REGULATORY_COUNTRY_IE_IGNORE 0
#define REGULATORY_WIPHY_SELF_MANAGED WIPHY_FLAG_SELF_MANAGED_REG

#define IEEE80211_CHAN_INDOOR_ONLY 0
#define IEEE80211_CHAN_IR_CONCURRENT 0

static inline void
cfg80211_ch_switch_started_notify(struct net_device *dev,
				  struct cfg80211_chan_def *chandef,
				  u8 count)
{
}
#else
static inline bool ieee80211_viftype_ocb(unsigned int iftype)
{
	return iftype == NL80211_IFTYPE_OCB;
}
#endif /* 3.19 */

#if CFG80211_VERSION < KERNEL_VERSION(4,0,0)
struct cfg80211_tid_stats {
	u32 filled;
	u64 rx_msdu;
	u64 tx_msdu;
	u64 tx_msdu_retries;
	u64 tx_msdu_failed;
};

static inline void
cfg80211_del_sta_sinfo(struct net_device *dev, const u8 *mac_addr,
		       struct station_info *sinfo, gfp_t gfp)
{
	cfg80211_del_sta(dev, mac_addr, gfp);
}

enum rate_info_bw {
	RATE_INFO_BW_20 = 0,
	RATE_INFO_BW_5,
	RATE_INFO_BW_10,
	RATE_INFO_BW_40,
	RATE_INFO_BW_80,
	RATE_INFO_BW_160,
};

enum ieee80211_bss_type {
	IEEE80211_BSS_TYPE_ESS,
	IEEE80211_BSS_TYPE_PBSS,
	IEEE80211_BSS_TYPE_IBSS,
	IEEE80211_BSS_TYPE_MBSS,
	IEEE80211_BSS_TYPE_ANY
};

enum ieee80211_privacy {
	IEEE80211_PRIVACY_ON,
	IEEE80211_PRIVACY_OFF,
	IEEE80211_PRIVACY_ANY
};

#define IEEE80211_PRIVACY(x)	\
	((x) ? IEEE80211_PRIVACY_ON : IEEE80211_PRIVACY_OFF)

static inline struct cfg80211_bss *
iwl7000_cfg80211_get_bss(struct wiphy *wiphy,
			 struct ieee80211_channel *channel,
			 const u8 *bssid,
			 const u8 *ssid, size_t ssid_len,
			 enum ieee80211_bss_type bss_type,
			 enum ieee80211_privacy privacy)
{
	u16 capa_val = 0;
	u16 capa_msk = 0;

	switch (bss_type) {
	case IEEE80211_BSS_TYPE_ESS:
		capa_val |= WLAN_CAPABILITY_ESS;
		capa_msk |= WLAN_CAPABILITY_ESS;
		break;
	case IEEE80211_BSS_TYPE_PBSS:
	case IEEE80211_BSS_TYPE_MBSS:
		WARN_ON(1);
		break;
	case IEEE80211_BSS_TYPE_IBSS:
		capa_val |= WLAN_CAPABILITY_IBSS;
		capa_msk |= WLAN_CAPABILITY_IBSS;
		break;
	case IEEE80211_BSS_TYPE_ANY:
		break;
	}

	switch (privacy) {
	case IEEE80211_PRIVACY_ON:
		capa_val |= WLAN_CAPABILITY_PRIVACY;
		capa_msk |= WLAN_CAPABILITY_PRIVACY;
		break;
	case IEEE80211_PRIVACY_OFF:
		capa_msk |= WLAN_CAPABILITY_PRIVACY;
		break;
	case IEEE80211_PRIVACY_ANY:
		break;
	}

	return cfg80211_get_bss(wiphy, channel, bssid, ssid, ssid_len,
				capa_msk, capa_val);
}
#define cfg80211_get_bss iwl7000_cfg80211_get_bss

static inline bool
ieee80211_chandef_to_operating_class(struct cfg80211_chan_def *chandef,
				     u8 *op_class)
{
	u8 vht_opclass;
	u16 freq = chandef->center_freq1;

	if (freq >= 2412 && freq <= 2472) {
		if (chandef->width > NL80211_CHAN_WIDTH_40)
			return false;

		/* 2.407 GHz, channels 1..13 */
		if (chandef->width == NL80211_CHAN_WIDTH_40) {
			if (freq > chandef->chan->center_freq)
				*op_class = 83; /* HT40+ */
			else
				*op_class = 84; /* HT40- */
		} else {
			*op_class = 81;
		}

		return true;
	}

	if (freq == 2484) {
		if (chandef->width > NL80211_CHAN_WIDTH_40)
			return false;

		*op_class = 82; /* channel 14 */
		return true;
	}

	switch (chandef->width) {
	case NL80211_CHAN_WIDTH_80:
		vht_opclass = 128;
		break;
	case NL80211_CHAN_WIDTH_160:
		vht_opclass = 129;
		break;
	case NL80211_CHAN_WIDTH_80P80:
		vht_opclass = 130;
		break;
	case NL80211_CHAN_WIDTH_10:
	case NL80211_CHAN_WIDTH_5:
		return false; /* unsupported for now */
	default:
		vht_opclass = 0;
		break;
	}

	/* 5 GHz, channels 36..48 */
	if (freq >= 5180 && freq <= 5240) {
		if (vht_opclass) {
			*op_class = vht_opclass;
		} else if (chandef->width == NL80211_CHAN_WIDTH_40) {
			if (freq > chandef->chan->center_freq)
				*op_class = 116;
			else
				*op_class = 117;
		} else {
			*op_class = 115;
		}

		return true;
	}

	/* 5 GHz, channels 52..64 */
	if (freq >= 5260 && freq <= 5320) {
		if (vht_opclass) {
			*op_class = vht_opclass;
		} else if (chandef->width == NL80211_CHAN_WIDTH_40) {
			if (freq > chandef->chan->center_freq)
				*op_class = 119;
			else
				*op_class = 120;
		} else {
			*op_class = 118;
		}

		return true;
	}

	/* 5 GHz, channels 100..144 */
	if (freq >= 5500 && freq <= 5720) {
		if (vht_opclass) {
			*op_class = vht_opclass;
		} else if (chandef->width == NL80211_CHAN_WIDTH_40) {
			if (freq > chandef->chan->center_freq)
				*op_class = 122;
			else
				*op_class = 123;
		} else {
			*op_class = 121;
		}

		return true;
	}

	/* 5 GHz, channels 149..169 */
	if (freq >= 5745 && freq <= 5845) {
		if (vht_opclass) {
			*op_class = vht_opclass;
		} else if (chandef->width == NL80211_CHAN_WIDTH_40) {
			if (freq > chandef->chan->center_freq)
				*op_class = 126;
			else
				*op_class = 127;
		} else if (freq <= 5805) {
			*op_class = 124;
		} else {
			*op_class = 125;
		}

		return true;
	}

	/* 56.16 GHz, channel 1..4 */
	if (freq >= 56160 + 2160 * 1 && freq <= 56160 + 2160 * 4) {
		if (chandef->width >= NL80211_CHAN_WIDTH_40)
			return false;

		*op_class = 180;
		return true;
	}

	/* not supported yet */
	return false;
}

#ifndef U16_MAX
#define U16_MAX         ((u16)~0U)
#endif
#endif

/* backport wiphy_ext_feature_set/_isset
 *
 * To do so, define our own versions thereof that check for a negative
 * feature index and in that case ignore it entirely. That allows us to
 * define the ones that the cfg80211 version doesn't support to -1.
 */
static inline void iwl7000_wiphy_ext_feature_set(struct wiphy *wiphy, int ftidx)
{
	if (ftidx < 0)
		return;
#if CFG80211_VERSION >= KERNEL_VERSION(4,0,0)
	wiphy_ext_feature_set(wiphy, ftidx);
#endif
}

static inline bool iwl7000_wiphy_ext_feature_isset(struct wiphy *wiphy,
						   int ftidx)
{
	if (ftidx < 0)
		return false;
#if CFG80211_VERSION >= KERNEL_VERSION(4,0,0)
	return wiphy_ext_feature_isset(wiphy, ftidx);
#endif
	return false;
}
#define wiphy_ext_feature_set iwl7000_wiphy_ext_feature_set
#define wiphy_ext_feature_isset iwl7000_wiphy_ext_feature_isset

static inline
size_t iwl7000_get_auth_data_len(struct cfg80211_auth_request *req)
{
#if CFG80211_VERSION < KERNEL_VERSION(4,10,0)
	return req->sae_data_len;
#else
	return req->auth_data_len;
#endif
}

static inline
const u8 *iwl7000_get_auth_data(struct cfg80211_auth_request *req)
{
#if CFG80211_VERSION < KERNEL_VERSION(4,10,0)
	return req->sae_data;
#else
	return req->auth_data;
#endif
}

static inline
size_t iwl7000_get_fils_kek_len(struct cfg80211_assoc_request *req)
{
#if CFG80211_VERSION < KERNEL_VERSION(4,10,0)
	return 0;
#else
	return req->fils_kek_len;
#endif
}

static inline
const u8 *iwl7000_get_fils_kek(struct cfg80211_assoc_request *req)
{
#if CFG80211_VERSION < KERNEL_VERSION(4,10,0)
	return NULL;
#else
	return req->fils_kek;
#endif
}

static inline
const u8 *iwl7000_get_fils_nonces(struct cfg80211_assoc_request *req)
{
#if CFG80211_VERSION < KERNEL_VERSION(4,10,0)
	return NULL;
#else
	return req->fils_nonces;
#endif
}

#if CFG80211_VERSION < KERNEL_VERSION(4,1,0)
size_t ieee80211_ie_split_ric(const u8 *ies, size_t ielen,
			      const u8 *ids, int n_ids,
			      const u8 *after_ric, int n_after_ric,
			      size_t offset);
size_t ieee80211_ie_split(const u8 *ies, size_t ielen,
			  const u8 *ids, int n_ids, size_t offset);
#define NL80211_EXT_FEATURE_VHT_IBSS -1
#endif

#if CFG80211_VERSION < KERNEL_VERSION(4,3,0)
#define cfg80211_reg_can_beacon_relax(wiphy, chandef, iftype) \
	cfg80211_reg_can_beacon(wiphy, chandef, iftype)
#endif

#if CFG80211_VERSION < KERNEL_VERSION(4,4,0)
#define CFG80211_STA_AP_CLIENT_UNASSOC CFG80211_STA_AP_CLIENT
#define NL80211_FEATURE_FULL_AP_CLIENT_STATE 0

struct cfg80211_sched_scan_plan {
	u32 interval;
	u32 iterations;
};
#endif

struct backport_sinfo {
	u32 filled;
	u32 connected_time;
	u32 inactive_time;
	u64 rx_bytes;
	u64 tx_bytes;
	u16 llid;
	u16 plid;
	u8 plink_state;
	s8 signal;
	s8 signal_avg;

	u8 chains;
	s8 chain_signal[IEEE80211_MAX_CHAINS];
	s8 chain_signal_avg[IEEE80211_MAX_CHAINS];

	struct rate_info txrate;
	struct rate_info rxrate;
	u32 rx_packets;
	u32 tx_packets;
	u32 tx_retries;
	u32 tx_failed;
	u32 rx_dropped_misc;
	struct sta_bss_parameters bss_param;
	struct nl80211_sta_flag_update sta_flags;

	int generation;

	const u8 *assoc_req_ies;
	size_t assoc_req_ies_len;

	u32 beacon_loss_count;
	s64 t_offset;
	enum nl80211_mesh_power_mode local_pm;
	enum nl80211_mesh_power_mode peer_pm;
	enum nl80211_mesh_power_mode nonpeer_pm;

	u32 expected_throughput;

	u64 tx_duration;
	u64 rx_duration;
	u64 rx_beacon;
	u8 rx_beacon_signal_avg;
#if CFG80211_VERSION < KERNEL_VERSION(4,18,0)
	/*
	 * With < 4.18 we use an array here, like before, so we don't
	 * need to alloc/free it
	 */
	struct cfg80211_tid_stats pertid[IEEE80211_NUM_TIDS + 1];
#else
	struct cfg80211_tid_stats *pertid;
#endif
	s8 ack_signal;
	s8 avg_ack_signal;

	u16 airtime_weight;

	u32 rx_mpdu_count;
	u32 fcs_err_count;

	u32 airtime_link_metric;
};

/* these are constants in nl80211.h, so it's
 * harmless to define them unconditionally
 */
#define NL80211_STA_INFO_RX_DROP_MISC		28
#define NL80211_STA_INFO_BEACON_RX		29
#define NL80211_STA_INFO_BEACON_SIGNAL_AVG	30
#define NL80211_STA_INFO_TID_STATS		31
#define NL80211_TID_STATS_RX_MSDU		1
#define NL80211_TID_STATS_TX_MSDU		2
#define NL80211_TID_STATS_TX_MSDU_RETRIES	3
#define NL80211_TID_STATS_TX_MSDU_FAILED	4

static inline void iwl7000_convert_sinfo(struct backport_sinfo *bpsinfo,
					 struct station_info *sinfo)
{
	memset(sinfo, 0, sizeof(*sinfo));
#define COPY(x)	sinfo->x = bpsinfo->x
#define MCPY(x)	memcpy(&sinfo->x, &bpsinfo->x, sizeof(sinfo->x))
	COPY(connected_time);
	COPY(inactive_time);
	COPY(rx_bytes);
	COPY(tx_bytes);
	COPY(llid);
	COPY(plid);
	COPY(plink_state);
	COPY(signal);
	COPY(signal_avg);
#if CFG80211_VERSION >= KERNEL_VERSION(3,11,0)
	COPY(chains);
	MCPY(chain_signal);
	MCPY(chain_signal_avg);
#endif
	COPY(txrate);
	COPY(rxrate);
	COPY(rx_packets);
	COPY(tx_packets);
	COPY(tx_retries);
	COPY(tx_failed);
	COPY(rx_dropped_misc);
	COPY(bss_param);
	COPY(sta_flags);
	COPY(generation);
	COPY(assoc_req_ies);
	COPY(assoc_req_ies_len);
	COPY(beacon_loss_count);
	COPY(t_offset);
#if CFG80211_VERSION >= KERNEL_VERSION(3,9,0)
	COPY(local_pm);
	COPY(peer_pm);
	COPY(nonpeer_pm);
#endif
#if CFG80211_VERSION >= KERNEL_VERSION(3,16,0)
	COPY(expected_throughput);
#endif
#if CFG80211_VERSION >= KERNEL_VERSION(4,18,0)
	COPY(ack_signal);
	COPY(avg_ack_signal);
#if CFG80211_VERSION >= KERNEL_VERSION(4,10,0)
	COPY(rx_duration);
#endif
#if CFG80211_VERSION >= KERNEL_VERSION(4,20,0)
	COPY(rx_mpdu_count);
	COPY(fcs_err_count);
#endif
#endif
#if CFG80211_VERSION >= KERNEL_VERSION(5,1,0)
	COPY(tx_duration);
	COPY(airtime_weight);
#endif
#if CFG80211_VERSION >= KERNEL_VERSION(5,2,0)
	COPY(airtime_link_metric);
#endif
#if CFG80211_VERSION >= KERNEL_VERSION(4,0,0)
	COPY(rx_beacon);
	COPY(rx_beacon_signal_avg);
	MCPY(pertid);
	COPY(filled);
#else
#define RENAMED_FLAG(n, o)						\
	do {								\
		if (bpsinfo->filled & BIT(NL80211_STA_INFO_ ## n))	\
			sinfo->filled |= STATION_INFO_ ## o;		\
	} while (0)
#define FLAG(flg)	RENAMED_FLAG(flg, flg)
	FLAG(INACTIVE_TIME);
	FLAG(RX_BYTES);
	FLAG(TX_BYTES);
	FLAG(LLID);
	FLAG(PLID);
	FLAG(PLINK_STATE);
	FLAG(SIGNAL);
	FLAG(TX_BITRATE);
	FLAG(RX_PACKETS);
	FLAG(TX_PACKETS);
	FLAG(TX_RETRIES);
	FLAG(TX_FAILED);
	FLAG(RX_DROP_MISC);
	FLAG(SIGNAL_AVG);
	FLAG(RX_BITRATE);
	FLAG(BSS_PARAM);
	FLAG(CONNECTED_TIME);
	if (bpsinfo->assoc_req_ies_len)
		sinfo->filled |= STATION_INFO_ASSOC_REQ_IES;
	FLAG(STA_FLAGS);
	RENAMED_FLAG(BEACON_LOSS, BEACON_LOSS_COUNT);
	FLAG(T_OFFSET);
#if CFG80211_VERSION >= KERNEL_VERSION(3,9,0)
	FLAG(LOCAL_PM);
	FLAG(PEER_PM);
	FLAG(NONPEER_PM);
	FLAG(RX_BYTES64);
	FLAG(TX_BYTES64);
#else
	RENAMED_FLAG(RX_BYTES64, RX_BYTES);
	RENAMED_FLAG(TX_BYTES64, RX_BYTES);
#endif
#if CFG80211_VERSION >= KERNEL_VERSION(3,11,0)
	FLAG(CHAIN_SIGNAL);
	FLAG(CHAIN_SIGNAL_AVG);
#endif
#if CFG80211_VERSION >= KERNEL_VERSION(3,16,0)
	FLAG(EXPECTED_THROUGHPUT);
#endif
#undef RENAMED_FLAG
#undef FLAG
#endif
#undef COPY
}
typedef struct station_info cfg_station_info_t;
#define station_info backport_sinfo

static inline void
backport_cfg80211_new_sta(struct net_device *dev, const u8 *mac_addr,
			  struct station_info *sinfo, gfp_t gfp)
{
	cfg_station_info_t cfg_info;

	iwl7000_convert_sinfo(sinfo, &cfg_info);
	cfg80211_new_sta(dev, mac_addr, &cfg_info, gfp);
}
#define cfg80211_new_sta backport_cfg80211_new_sta

static inline void
backport_cfg80211_del_sta_sinfo(struct net_device *dev, const u8 *mac_addr,
				struct station_info *sinfo, gfp_t gfp)
{
	cfg_station_info_t cfg_info;

	iwl7000_convert_sinfo(sinfo, &cfg_info);
	cfg80211_del_sta_sinfo(dev, mac_addr, &cfg_info, gfp);
}
#define cfg80211_del_sta_sinfo backport_cfg80211_del_sta_sinfo

typedef struct survey_info cfg_survey_info_t;
#if CFG80211_VERSION < KERNEL_VERSION(4,0,0)
#define survey_info bp_survey_info
struct survey_info {
	struct ieee80211_channel *channel;
	u64 time;
	u64 time_busy;
	u64 time_ext_busy;
	u64 time_rx;
	u64 time_tx;
	u64 time_scan;
	u32 filled;
	s8 noise;
};
#define SURVEY_INFO_TIME SURVEY_INFO_CHANNEL_TIME
#define SURVEY_INFO_TIME_BUSY SURVEY_INFO_CHANNEL_TIME_BUSY
#define SURVEY_INFO_TIME_EXT_BUSY SURVEY_INFO_CHANNEL_TIME_EXT_BUSY
#define SURVEY_INFO_TIME_RX SURVEY_INFO_CHANNEL_TIME_RX
#define SURVEY_INFO_TIME_TX SURVEY_INFO_CHANNEL_TIME_TX
#define SURVEY_INFO_TIME_SCAN 0
static inline void iwl7000_convert_survey_info(struct survey_info *survey,
					       cfg_survey_info_t *cfg)
{
	cfg->channel = survey->channel;
	cfg->channel_time = survey->time;
	cfg->channel_time_busy = survey->time_busy;
	cfg->channel_time_ext_busy = survey->time_ext_busy;
	cfg->channel_time_rx = survey->time_rx;
	cfg->channel_time_tx = survey->time_tx;
	cfg->noise = survey->noise;
	cfg->filled = survey->filled;
}
#else
static inline void iwl7000_convert_survey_info(struct survey_info *survey,
					       cfg_survey_info_t *cfg)
{
	memcpy(cfg, survey, sizeof(*cfg));
}
#endif

#if CFG80211_VERSION < KERNEL_VERSION(4,4,0)
struct cfg80211_inform_bss {
	struct ieee80211_channel *chan;
	enum nl80211_bss_scan_width scan_width;
	s32 signal;
	u64 boottime_ns;
};

static inline struct cfg80211_bss *
cfg80211_inform_bss_frame_data(struct wiphy *wiphy,
			       struct cfg80211_inform_bss *data,
			       struct ieee80211_mgmt *mgmt, size_t len,
			       gfp_t gfp)

{
	return cfg80211_inform_bss_width_frame(wiphy, data->chan,
					       data->scan_width, mgmt,
					       len, data->signal, gfp);
}
#endif /* CFG80211_VERSION < KERNEL_VERSION(4,4,0) */

#if CFG80211_VERSION < KERNEL_VERSION(4,12,0)
#define mon_opts_flags(p)	flags
#define mon_opts_params(p)	(!flags ? 0 :				    \
				 *flags | 1 << __NL80211_MNTR_FLAG_INVALID),\
				p

#define vif_params_vht_mumimo_groups(p) NULL
#define vif_params_vht_mumimo_follow_addr(p) NULL

#define cfg80211_sched_scan_results(wiphy, reqid)	\
	cfg80211_sched_scan_results(wiphy)
#define cfg80211_sched_scan_stopped(wiphy, reqid)	\
	cfg80211_sched_scan_stopped(wiphy)

#ifndef cfg80211_sched_scan_stopped_rtnl
#define cfg80211_sched_scan_stopped_rtnl(wiphy, reqid)	\
	cfg80211_sched_scan_stopped_rtnl(wiphy)
#endif

#else  /* CFG80211_VERSION < KERNEL_VERSION(4,12,0) */

#define mon_opts_flags(p)	((p)->flags)
#define mon_opts_params(p)	p

#define vif_params_vht_mumimo_groups(p) ((p)->vht_mumimo_groups)
#define vif_params_vht_mumimo_follow_addr(p) ((p)->vht_mumimo_follow_addr)

#endif /* CFG80211_VERSION < KERNEL_VERSION(4,12,0) */

#if CFG80211_VERSION < KERNEL_VERSION(4,9,0)
static inline bool ieee80211_viftype_nan(unsigned int iftype)
{
	return false;
}

static inline bool ieee80211_has_nan_iftype(unsigned int iftype)
{
	return false;
}

#if CFG80211_VERSION < KERNEL_VERSION(4,4,0)
struct cfg80211_nan_conf {
	u8 master_pref;
	u8 bands;
	u8 cdw_2g;
	u8 cdw_5g;
};

enum nl80211_nan_function_type {
	NL80211_NAN_FUNC_PUBLISH,
	NL80211_NAN_FUNC_SUBSCRIBE,
	NL80211_NAN_FUNC_FOLLOW_UP,

	/* keep last */
	__NL80211_NAN_FUNC_TYPE_AFTER_LAST,
	NL80211_NAN_FUNC_MAX_TYPE = __NL80211_NAN_FUNC_TYPE_AFTER_LAST - 1,
};

struct cfg80211_nan_func_filter {
	const u8 *filter;
	u8 len;
};

enum nl80211_nan_func_term_reason {
	NL80211_NAN_FUNC_TERM_REASON_USER_REQUEST,
	NL80211_NAN_FUNC_TERM_REASON_TTL_EXPIRED,
	NL80211_NAN_FUNC_TERM_REASON_ERROR,
};

#define NL80211_NAN_FUNC_SERVICE_ID_LEN 6

struct cfg80211_nan_func {
	enum nl80211_nan_function_type type;
	u8 service_id[NL80211_NAN_FUNC_SERVICE_ID_LEN];
	u8 publish_type;
	bool close_range;
	bool publish_bcast;
	bool subscribe_active;
	u8 followup_id;
	u8 followup_reqid;
	struct mac_address followup_dest;
	u32 ttl;
	const u8 *serv_spec_info;
	u8 serv_spec_info_len;
	bool srf_include;
	const u8 *srf_bf;
	u8 srf_bf_len;
	u8 srf_bf_idx;
	struct mac_address *srf_macs;
	int srf_num_macs;
	struct cfg80211_nan_func_filter *rx_filters;
	struct cfg80211_nan_func_filter *tx_filters;
	u8 num_tx_filters;
	u8 num_rx_filters;
	u8 instance_id;
	u64 cookie;
};

static inline void cfg80211_free_nan_func(struct cfg80211_nan_func *f)
{
}

struct cfg80211_nan_match_params {
	enum nl80211_nan_function_type type;
	u8 inst_id;
	u8 peer_inst_id;
	const u8 *addr;
	u8 info_len;
	const u8 *info;
	u64 cookie;
};

static inline bool cfg80211_nan_started(struct wireless_dev *wdev)
{
	return false;
}

enum nl80211_nan_publish_type {
	NL80211_NAN_SOLICITED_PUBLISH = 1 << 0,
	NL80211_NAN_UNSOLICITED_PUBLISH = 1 << 1,
};
#endif /* CFG80211_VERSION < KERNEL_VERSION(4,4,0) */
#else
static inline bool ieee80211_viftype_nan(unsigned int iftype)
{
	return iftype == NL80211_IFTYPE_NAN;
}

static inline
bool ieee80211_has_nan_iftype(unsigned int iftype)
{
	return iftype & BIT(NL80211_IFTYPE_NAN);
}
#endif /* CFG80211_VERSION < KERNEL_VERSION(4,9,0) */

#if CFG80211_VERSION < KERNEL_VERSION(99,0,0)
#define nan_conf_cdw_2g(conf) 1
#define nan_conf_cdw_5g(conf) 1
#else
#define nan_conf_cdw_2g(conf) ((conf)->cdw_2g)
#define nan_conf_cdw_5g(conf) ((conf)->cdw_5g)
#endif

#if CFG80211_VERSION < KERNEL_VERSION(4,20,0)
#define beacon_ftm_len(beacon, m) 0
#else
#define beacon_ftm_len(beacon, m) ((beacon)->m)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,19,0)
static inline long ktime_get_seconds(void)
{
	struct timespec uptime;

	ktime_get_ts(&uptime);
	return uptime.tv_sec;
}
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3,19,0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
#define thermal_notify_framework notify_thermal_framework
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0) */

#ifndef S16_MAX
#define S16_MAX		((s16)(U16_MAX>>1))
#endif
#ifndef S16_MIN
#define S16_MIN		((s16)(-S16_MAX - 1))
#endif

#if CFG80211_VERSION < KERNEL_VERSION(4,6,0)
#define NL80211_EXT_FEATURE_RRM -1
#endif

static inline int
cfg80211_sta_support_p2p_ps(struct station_parameters *params, bool p2p_go)
{
#if CFG80211_VERSION >= KERNEL_VERSION(4,7,0)
	return params->support_p2p_ps;
#endif
	return p2p_go;
}

#if LINUX_VERSION_IS_LESS(4,4,0)
int match_string(const char * const *array, size_t n, const char *string);
#endif /* LINUX_VERSION_IS_LESS(4,4,0) */

/* this was added in v3.2.79, v3.18.30, v4.1.21, v4.4.6 and 4.5 */
#if !(LINUX_VERSION_IS_GEQ(4,4,6) || \
      (LINUX_VERSION_IS_GEQ(4,1,21) && \
       LINUX_VERSION_IS_LESS(4,2,0)) || \
      (LINUX_VERSION_IS_GEQ(3,18,30) && \
       LINUX_VERSION_IS_LESS(3,19,0)) || \
      (LINUX_VERSION_IS_GEQ(3,2,79) && \
       LINUX_VERSION_IS_LESS(3,3,0)))
/* we don't have wext */
static inline void wireless_nlevent_flush(void) {}
#endif

static inline u8*
cfg80211_scan_req_bssid(struct cfg80211_scan_request *scan_req)
{
#if CFG80211_VERSION >= KERNEL_VERSION(4,7,0)
	return scan_req->bssid;
#endif
	return NULL;
}

#if CFG80211_VERSION < KERNEL_VERSION(4,7,0)
/* this was originally in 3.10, but causes nasty warnings
 * due to the enum ieee80211_band removal - use the inline
 * to avoid that.
 */
#define ieee80211_operating_class_to_band iwl7000_ieee80211_operating_class_to_band
static inline bool
ieee80211_operating_class_to_band(u8 operating_class,
				  enum nl80211_band *band)
{
	switch (operating_class) {
	case 112:
	case 115 ... 127:
	case 128 ... 130:
		*band = NL80211_BAND_5GHZ;
		return true;
	case 81:
	case 82:
	case 83:
	case 84:
		*band = NL80211_BAND_2GHZ;
		return true;
	case 180:
		*band = NL80211_BAND_60GHZ;
		return true;
	}

	/* stupid compiler */
	*band = NL80211_BAND_2GHZ;

	return false;
}

#define NUM_NL80211_BANDS ((enum nl80211_band)IEEE80211_NUM_BANDS)
#endif

#if CFG80211_VERSION < KERNEL_VERSION(4,4,0)
struct cfg80211_scan_info {
	u64 scan_start_tsf;
	u8 tsf_bssid[ETH_ALEN] __aligned(2);
	bool aborted;
};

static inline void
backport_cfg80211_scan_done(struct cfg80211_scan_request *request,
			    struct cfg80211_scan_info *info)
{
	cfg80211_scan_done(request, info->aborted);
}
#define cfg80211_scan_done backport_cfg80211_scan_done

#define NL80211_EXT_FEATURE_SCAN_START_TIME -1
#define NL80211_EXT_FEATURE_BSS_PARENT_TSF -1
#define NL80211_EXT_FEATURE_SET_SCAN_DWELL -1
#endif /* CFG80211_VERSION < KERNEL_VERSION(4,4,0) */

#if CFG80211_VERSION < KERNEL_VERSION(4,9,0)
static inline
const u8 *bp_cfg80211_find_ie_match(u8 eid, const u8 *ies, int len,
				    const u8 *match, int match_len,
				    int match_offset)
{
	/* match_offset can't be smaller than 2, unless match_len is
	 * zero, in which case match_offset must be zero as well.
	 */
	if (WARN_ON((match_len && match_offset < 2) ||
		    (!match_len && match_offset)))
		return NULL;

	while (len >= 2 && len >= ies[1] + 2) {
		if ((ies[0] == eid) &&
		    (ies[1] + 2 >= match_offset + match_len) &&
		    !memcmp(ies + match_offset, match, match_len))
			return ies;

		len -= ies[1] + 2;
		ies += ies[1] + 2;
	}

	return NULL;
}

#define cfg80211_find_ie_match bp_cfg80211_find_ie_match

#define NL80211_EXT_FEATURE_MU_MIMO_AIR_SNIFFER -1

int ieee80211_data_to_8023_exthdr(struct sk_buff *skb, struct ethhdr *ehdr,
				  const u8 *addr, enum nl80211_iftype iftype);
/* manually renamed to avoid symbol issues with cfg80211 */
#define ieee80211_amsdu_to_8023s iwl7000_ieee80211_amsdu_to_8023s
void ieee80211_amsdu_to_8023s(struct sk_buff *skb, struct sk_buff_head *list,
			      const u8 *addr, enum nl80211_iftype iftype,
			      const unsigned int extra_headroom,
			      const u8 *check_da, const u8 *check_sa);
#endif /* CFG80211_VERSION < KERNEL_VERSION(4,9,0) */

#ifndef IEEE80211_RADIOTAP_TIMESTAMP_UNIT_MASK
#define IEEE80211_RADIOTAP_TIMESTAMP 22
#define IEEE80211_RADIOTAP_TIMESTAMP_UNIT_MASK			0x000F
#define IEEE80211_RADIOTAP_TIMESTAMP_UNIT_MS			0x0000
#define IEEE80211_RADIOTAP_TIMESTAMP_UNIT_US			0x0001
#define IEEE80211_RADIOTAP_TIMESTAMP_UNIT_NS			0x0003
#define IEEE80211_RADIOTAP_TIMESTAMP_SPOS_MASK			0x00F0
#define IEEE80211_RADIOTAP_TIMESTAMP_SPOS_BEGIN_MDPU		0x0000
#define IEEE80211_RADIOTAP_TIMESTAMP_SPOS_PLCP_SIG_ACQ		0x0010
#define IEEE80211_RADIOTAP_TIMESTAMP_SPOS_EO_PPDU		0x0020
#define IEEE80211_RADIOTAP_TIMESTAMP_SPOS_EO_MPDU		0x0030
#define IEEE80211_RADIOTAP_TIMESTAMP_SPOS_UNKNOWN		0x00F0
#define IEEE80211_RADIOTAP_TIMESTAMP_FLAG_64BIT			0x00
#define IEEE80211_RADIOTAP_TIMESTAMP_FLAG_32BIT			0x01
#define IEEE80211_RADIOTAP_TIMESTAMP_FLAG_ACCURACY		0x02
#endif /* IEEE80211_RADIOTAP_TIMESTAMP_UNIT_MASK */

#if CFG80211_VERSION < KERNEL_VERSION(4,4,0)
/*
 * NB: upstream this only landed in 4.10, but it was backported
 * to almost every kernel, including 4.4 (at least in ChromeOS)
 * If you see a compilation failure on this function you should
 * backport the fix:
 * e6f462df9acd ("cfg80211/mac80211: fix BSS leaks when abandoning assoc attempts")
 */
static inline void cfg80211_abandon_assoc(struct net_device *dev,
					  struct cfg80211_bss *bss)
{
	/*
	 * We can't really do anything better - we used to leak in
	 * this scenario forever, and we can't backport the cfg80211
	 * function since it needs access to the *internal* BSS to
	 * remove the pinning (internal_bss->hold).
	 * Just warn, and hope that ChromeOS will pick up the fix
	 * from upstream at which point we can remove this inline.
	 */
	WARN_ONCE(1, "BSS entry for %pM leaked\n", bss->bssid);
}
#endif /* CFG80211_VERSION < KERNEL_VERSION(4,4,0) */

#if CFG80211_VERSION < KERNEL_VERSION(4,10,0)
#define NL80211_EXT_FEATURE_FILS_STA -1

static inline bool wdev_running(struct wireless_dev *wdev)
{
	if (wdev->netdev)
		return netif_running(wdev->netdev);
	return wdev->p2p_started;
}

static inline const u8 *cfg80211_find_ext_ie(u8 ext_eid, const u8 *ies, int len)
{
	return cfg80211_find_ie_match(WLAN_EID_EXTENSION, ies, len,
				      &ext_eid, 1, 2);
}
#endif /* CFG80211_VERSION < KERNEL_VERSION(4,10,0) */

#if CFG80211_VERSION < KERNEL_VERSION(4,4,0)
struct iface_combination_params {
	int num_different_channels;
	u8 radar_detect;
	int iftype_num[NUM_NL80211_IFTYPES];
	u32 new_beacon_int;
};

static inline
int iwl7000_check_combinations(struct wiphy *wiphy,
			       struct iface_combination_params *params)
{
	return cfg80211_check_combinations(wiphy,
					   params->num_different_channels,
					   params->radar_detect,
					   params->iftype_num);
}

#define cfg80211_check_combinations iwl7000_check_combinations

static inline
int iwl7000_iter_combinations(struct wiphy *wiphy,
			      struct iface_combination_params *params,
			      void (*iter)(const struct ieee80211_iface_combination *c,
					   void *data),
			      void *data)
{
	return cfg80211_iter_combinations(wiphy, params->num_different_channels,
					  params->radar_detect,
					  params->iftype_num,
					  iter, data);
}

#define cfg80211_iter_combinations iwl7000_iter_combinations
#endif /* CFG80211_VERSION < KERNEL_VERSION(4,4,0) */

#if CFG80211_VERSION >= KERNEL_VERSION(4,11,0) || \
     CFG80211_VERSION < KERNEL_VERSION(4,9,0)
static inline
bool ieee80211_nan_has_band(struct cfg80211_nan_conf *conf, u8 band)
{
	return conf->bands & BIT(band);
}

static inline
void ieee80211_nan_set_band(struct cfg80211_nan_conf *conf, u8 band)
{
	conf->bands |= BIT(band);
}

static inline u8 ieee80211_nan_bands(struct cfg80211_nan_conf *conf)
{
	return conf->bands;
}
#else
static inline
bool ieee80211_nan_has_band(struct cfg80211_nan_conf *conf, u8 band)
{
	return (band == NL80211_BAND_2GHZ) ||
		(band == NL80211_BAND_5GHZ && conf->dual);
}

static inline
void ieee80211_nan_set_band(struct cfg80211_nan_conf *conf, u8 band)
{
	if (band == NL80211_BAND_2GHZ)
		return;

	conf->dual = (band == NL80211_BAND_5GHZ);
}

static inline u8 ieee80211_nan_bands(struct cfg80211_nan_conf *conf)
{
	return BIT(NL80211_BAND_2GHZ) |
		(conf->dual ? BIT(NL80211_BAND_5GHZ) : 0);
}

#define CFG80211_NAN_CONF_CHANGED_BANDS CFG80211_NAN_CONF_CHANGED_DUAL
#endif

#if CFG80211_VERSION < KERNEL_VERSION(4,11,0)
static inline
void iwl7000_cqm_rssi_notify(struct net_device *dev,
			     enum nl80211_cqm_rssi_threshold_event rssi_event,
			     s32 rssi_level, gfp_t gfp)
{
	cfg80211_cqm_rssi_notify(dev, rssi_event, gfp);
}

#define cfg80211_cqm_rssi_notify iwl7000_cqm_rssi_notify
#endif

#if CFG80211_VERSION < KERNEL_VERSION(4,19,0)
#define IEEE80211_HE_PPE_THRES_MAX_LEN		25

/**
 * enum nl80211_he_gi - HE guard interval
 * @NL80211_RATE_INFO_HE_GI_0_8: 0.8 usec
 * @NL80211_RATE_INFO_HE_GI_1_6: 1.6 usec
 * @NL80211_RATE_INFO_HE_GI_3_2: 3.2 usec
 */
enum nl80211_he_gi {
	NL80211_RATE_INFO_HE_GI_0_8,
	NL80211_RATE_INFO_HE_GI_1_6,
	NL80211_RATE_INFO_HE_GI_3_2,
};

/**
 * @enum nl80211_he_ru_alloc - HE RU allocation values
 * @NL80211_RATE_INFO_HE_RU_ALLOC_26: 26-tone RU allocation
 * @NL80211_RATE_INFO_HE_RU_ALLOC_52: 52-tone RU allocation
 * @NL80211_RATE_INFO_HE_RU_ALLOC_106: 106-tone RU allocation
 * @NL80211_RATE_INFO_HE_RU_ALLOC_242: 242-tone RU allocation
 * @NL80211_RATE_INFO_HE_RU_ALLOC_484: 484-tone RU allocation
 * @NL80211_RATE_INFO_HE_RU_ALLOC_996: 996-tone RU allocation
 * @NL80211_RATE_INFO_HE_RU_ALLOC_2x996: 2x996-tone RU allocation
 */
enum nl80211_he_ru_alloc {
	NL80211_RATE_INFO_HE_RU_ALLOC_26,
	NL80211_RATE_INFO_HE_RU_ALLOC_52,
	NL80211_RATE_INFO_HE_RU_ALLOC_106,
	NL80211_RATE_INFO_HE_RU_ALLOC_242,
	NL80211_RATE_INFO_HE_RU_ALLOC_484,
	NL80211_RATE_INFO_HE_RU_ALLOC_996,
	NL80211_RATE_INFO_HE_RU_ALLOC_2x996,
};

#define RATE_INFO_BW_HE_RU	(RATE_INFO_BW_160 + 1)

/**
 * struct ieee80211_sta_he_cap - STA's HE capabilities
 *
 * This structure describes most essential parameters needed
 * to describe 802.11ax HE capabilities for a STA.
 *
 * @has_he: true iff HE data is valid.
 * @he_cap_elem: Fixed portion of the HE capabilities element.
 * @he_mcs_nss_supp: The supported NSS/MCS combinations.
 * @ppe_thres: Holds the PPE Thresholds data.
 */
struct ieee80211_sta_he_cap {
	bool has_he;
	struct ieee80211_he_cap_elem he_cap_elem;
	struct ieee80211_he_mcs_nss_supp he_mcs_nss_supp;
	u8 ppe_thres[IEEE80211_HE_PPE_THRES_MAX_LEN];
};

/**
 * struct ieee80211_sband_iftype_data
 *
 * This structure encapsulates sband data that is relevant for the interface
 * types defined in %types
 *
 * @types_mask: interface types (bits)
 * @he_cap: holds the HE capabilities
 */
struct ieee80211_sband_iftype_data {
	u16 types_mask;
	struct ieee80211_sta_he_cap he_cap;
};

/**
 * ieee80211_get_he_sta_cap - return HE capabilities for an sband's STA
 * @sband: the sband to search for the STA on
 *
 * Return: pointer to the struct ieee80211_sta_he_cap, or NULL is none found
 *	Currently, not supported
 */
static inline const struct ieee80211_sta_he_cap *
ieee80211_get_he_sta_cap(const struct ieee80211_supported_band *sband)
{
	return NULL;
}
#endif

#ifndef SHASH_DESC_ON_STACK
#define SHASH_DESC_ON_STACK(shash, ctx)				 \
	char __##shash##_desc[sizeof(struct shash_desc) +	 \
	       crypto_shash_descsize(ctx)] CRYPTO_MINALIGN_ATTR; \
	struct shash_desc *shash = (struct shash_desc *)__##shash##_desc

#endif

#if LINUX_VERSION_IS_LESS(4,11,0)
static inline void *backport_idr_remove(struct idr *idr, int id)
{
	void *item = idr_find(idr, id);
	idr_remove(idr, id);
	return item;
}
#define idr_remove     backport_idr_remove
#endif

#ifndef setup_deferrable_timer
#define setup_deferrable_timer(timer, fn, data)                         \
        __setup_timer((timer), (fn), (data), TIMER_DEFERRABLE)
#endif

#if LINUX_VERSION_IS_LESS(4,1,0)
typedef struct {
#ifdef CONFIG_NET_NS
	struct net *net;
#endif
} possible_net_t;

static inline void possible_write_pnet(possible_net_t *pnet, struct net *net)
{
#ifdef CONFIG_NET_NS
	pnet->net = net;
#endif
}

static inline struct net *possible_read_pnet(const possible_net_t *pnet)
{
#ifdef CONFIG_NET_NS
	return pnet->net;
#else
	return &init_net;
#endif
}
#else
#define possible_write_pnet(pnet, net) write_pnet(pnet, net)
#define possible_read_pnet(pnet) read_pnet(pnet)
#endif /* LINUX_VERSION_IS_LESS(4,1,0) */

#if LINUX_VERSION_IS_LESS(4,12,0) &&		\
	!LINUX_VERSION_IN_RANGE(4,11,9, 4,12,0)
#define netdev_set_priv_destructor(_dev, _destructor) \
	(_dev)->destructor = __ ## _destructor
#define netdev_set_def_destructor(_dev) \
	(_dev)->destructor = free_netdev
#else
#define netdev_set_priv_destructor(_dev, _destructor) \
	(_dev)->needs_free_netdev = true; \
	(_dev)->priv_destructor = (_destructor);
#define netdev_set_def_destructor(_dev) \
	(_dev)->needs_free_netdev = true;
#endif

/*
 * ChromeOS cherry-picked this in 3.8, 3.14 and 3.18.  And it is also
 * in 4.4 (inherited from stable).  Let's keep the check for 4.8 check
 * still here for the unlikely case that they branch out before
 * 4.8.13 and don't cherry-pick it.
 */
#if LINUX_VERSION_IS_LESS(4,9,0) &&			\
	!LINUX_VERSION_IN_RANGE(3,8,0, 3,9,0) &&	\
	!LINUX_VERSION_IN_RANGE(3,14,0, 3,15,0) &&	\
	!LINUX_VERSION_IN_RANGE(3,18,0, 3,19,0) &&	\
	!LINUX_VERSION_IN_RANGE(4,4,37, 4,5,0) &&	\
	!LINUX_VERSION_IN_RANGE(4,8,13, 4,9,0)

#define pcie_find_root_port iwl7000_pcie_find_root_port
static inline struct pci_dev *pcie_find_root_port(struct pci_dev *dev)
{
	while (1) {
		if (!pci_is_pcie(dev))
			break;
		if (pci_pcie_type(dev) == PCI_EXP_TYPE_ROOT_PORT)
			return dev;
		if (!dev->bus->self)
			break;
		dev = dev->bus->self;
	}
	return NULL;
}
#endif

#ifndef from_timer
#define TIMER_DATA_TYPE          unsigned long
#define TIMER_FUNC_TYPE          void (*)(TIMER_DATA_TYPE)

static inline void timer_setup(struct timer_list *timer,
			       void (*callback) (struct timer_list *),
			       unsigned int flags)
{
	__setup_timer(timer, (TIMER_FUNC_TYPE) callback,
		      (TIMER_DATA_TYPE) timer, flags);
}

#define from_timer(var, callback_timer, timer_fieldname) \
	container_of(callback_timer, typeof(*var), timer_fieldname)
#endif

#if CFG80211_VERSION < KERNEL_VERSION(4,0,0)
/* must be after the (potential) definition of RATE_INFO_BW_HE_RU */
static inline void set_rate_info_bw(struct rate_info *ri, int bw)
{
	switch (bw) {
	case RATE_INFO_BW_20:
		break;
	case RATE_INFO_BW_5:
	case RATE_INFO_BW_10:
	case RATE_INFO_BW_HE_RU:
		WARN_ONCE(1, "Unsupported bandwidth (%d) on this cfg80211 version\n",
			  bw);
		break;
	case RATE_INFO_BW_40:
		ri->flags |= RATE_INFO_FLAGS_40_MHZ_WIDTH;
		break;
	case RATE_INFO_BW_80:
		ri->flags |= RATE_INFO_FLAGS_80_MHZ_WIDTH;
		break;
	case RATE_INFO_BW_160:
		ri->flags |= RATE_INFO_FLAGS_160_MHZ_WIDTH;
		break;
	}
}
#else
static inline void set_rate_info_bw(struct rate_info *ri, int bw)
{
	ri->bw = bw;
}
#endif /* CFG80211_VERSION < KERNEL_VERSION(4,0,0) */

#if LINUX_VERSION_IS_LESS(4,11,0)
static inline u32 get_random_u32(void)
{
#if LINUX_VERSION_IS_LESS(3,10,0)
	unsigned int r;
	get_random_bytes(&r, sizeof(r));

	return r;
#else
	return get_random_int();
#endif
}
#endif

#if LINUX_VERSION_IS_LESS(4,13,0)
static inline void *backport_skb_put(struct sk_buff *skb, unsigned int len)
{
	return skb_put(skb, len);
}
#define skb_put LINUX_BACKPORT(skb_put)

static inline void *backport_skb_push(struct sk_buff *skb, unsigned int len)
{
	return skb_push(skb, len);
}
#define skb_push LINUX_BACKPORT(skb_push)

static inline void *backport___skb_push(struct sk_buff *skb, unsigned int len)
{
	return __skb_push(skb, len);
}
#define __skb_push LINUX_BACKPORT(__skb_push)

static inline void *skb_put_zero(struct sk_buff *skb, unsigned int len)
{
	void *tmp = skb_put(skb, len);

	memset(tmp, 0, len);

	return tmp;
}

static inline void *skb_put_data(struct sk_buff *skb, const void *data,
				 unsigned int len)
{
	void *tmp = skb_put(skb, len);

	memcpy(tmp, data, len);

	return tmp;
}

static inline void skb_put_u8(struct sk_buff *skb, u8 val)
{
	*(u8 *)skb_put(skb, 1) = val;
}
#endif

#if LINUX_VERSION_IS_LESS(4,20,0)
static inline struct sk_buff *__skb_peek(const struct sk_buff_head *list_)
{
	return list_->next;
}
#endif

#if LINUX_VERSION_IS_LESS(3, 10, 0)
static inline void kfree_skb_list(struct sk_buff *segs)
{
	while (segs) {
		struct sk_buff *next = segs->next;

		kfree_skb(segs);
		segs = next;
	}
}
#endif

#if LINUX_VERSION_IS_LESS(3,18,0)
static inline void __percpu *__alloc_gfp_warn(void)
{
	WARN(1, "Cannot backport alloc_percpu_gfp");
	return NULL;
}

#define alloc_percpu_gfp(type, gfp) \
	({ (gfp == GFP_KERNEL) ? alloc_percpu(type) : __alloc_gfp_warn(); })
#endif

#if LINUX_VERSION_IS_LESS(4,15,0)
#define NL80211_EXT_FEATURE_FILS_MAX_CHANNEL_TIME -1
#define NL80211_EXT_FEATURE_ACCEPT_BCAST_PROBE_RESP -1
#define NL80211_EXT_FEATURE_OCE_PROBE_REQ_HIGH_TX_RATE -1
#define NL80211_EXT_FEATURE_OCE_PROBE_REQ_DEFERRAL_SUPPRESSION -1
#define NL80211_SCAN_FLAG_FILS_MAX_CHANNEL_TIME BIT(4)
#define NL80211_SCAN_FLAG_ACCEPT_BCAST_PROBE_RESP BIT(5)
#define NL80211_SCAN_FLAG_OCE_PROBE_REQ_HIGH_TX_RATE BIT(6)
#define NL80211_SCAN_FLAG_OCE_PROBE_REQ_DEFERRAL_SUPPRESSION BIT(7)
#endif

#if CFG80211_VERSION < KERNEL_VERSION(4,4,0) ||		\
	(CFG80211_VERSION >= KERNEL_VERSION(4,5,0) &&	\
	 CFG80211_VERSION < KERNEL_VERSION(4,17,0))
struct ieee80211_wmm_ac {
	u16 cw_min;
	u16 cw_max;
	u16 cot;
	u8 aifsn;
};

struct ieee80211_wmm_rule {
	struct ieee80211_wmm_ac client[IEEE80211_NUM_ACS];
	struct ieee80211_wmm_ac ap[IEEE80211_NUM_ACS];
};

static inline int
reg_query_regdb_wmm(char *alpha2, int freq, u32 *ptr,
		    struct ieee80211_wmm_rule *rule)
{
	pr_debug_once(KERN_DEBUG
		      "iwl7000: ETSI WMM data not implemented yet!\n");
	return -ENODATA;
}
#endif /* < 4.4.0 || (>= 4.5.0 && < 4.17.0) */

#if CFG80211_VERSION < KERNEL_VERSION(99,0,0)
/* not yet upstream */
static inline bool ieee80211_viftype_nan_data(unsigned int iftype)
{
	return false;
}

static inline bool ieee80211_has_nan_data_iftype(unsigned int iftype)
{
	return false;
}
#endif

#if CFG80211_VERSION < KERNEL_VERSION(99,0,0)
/* not yet upstream */
static inline int
cfg80211_crypto_n_ciphers_group(struct cfg80211_crypto_settings *crypto)
{
	return 1;
}

static inline u32
cfg80211_crypto_ciphers_group(struct cfg80211_crypto_settings *crypto,
			      int idx)
{
	WARN_ON(idx != 0);
	return crypto->cipher_group;
}
#else
static inline int
cfg80211_crypto_n_ciphers_group(struct cfg80211_crypto_settings *crypto)
{
	return crypto->n_ciphers_group;
}

static inline u32
cfg80211_crypto_ciphers_group(struct cfg80211_crypto_settings *crypto,
			      int idx)
{
	return crypto->cipher_groups[idx];
}
#endif

#ifndef VHT_MUMIMO_GROUPS_DATA_LEN
#define VHT_MUMIMO_GROUPS_DATA_LEN (WLAN_MEMBERSHIP_LEN +\
				    WLAN_USER_POSITION_LEN)
#endif

#if CFG80211_VERSION >= KERNEL_VERSION(4,20,0)
#define cfg_he_cap(params) params->he_cap
#else
#define cfg_he_cap(params) NULL

/* Layer 2 Update frame (802.2 Type 1 LLC XID Update response) */
struct iapp_layer2_update {
	u8 da[ETH_ALEN];	/* broadcast */
	u8 sa[ETH_ALEN];	/* STA addr */
	__be16 len;		/* 6 */
	u8 dsap;		/* 0 */
	u8 ssap;		/* 0 */
	u8 control;
	u8 xid_info[3];
} __packed;

static inline
void cfg80211_send_layer2_update(struct net_device *dev, const u8 *addr)
{
	struct iapp_layer2_update *msg;
	struct sk_buff *skb;

	/* Send Level 2 Update Frame to update forwarding tables in layer 2
	 * bridge devices */

	skb = dev_alloc_skb(sizeof(*msg));
	if (!skb)
		return;
	msg = skb_put(skb, sizeof(*msg));

	/* 802.2 Type 1 Logical Link Control (LLC) Exchange Identifier (XID)
	 * Update response frame; IEEE Std 802.2-1998, 5.4.1.2.1 */

	eth_broadcast_addr(msg->da);
	ether_addr_copy(msg->sa, addr);
	msg->len = htons(6);
	msg->dsap = 0;
	msg->ssap = 0x01;	/* NULL LSAP, CR Bit: Response */
	msg->control = 0xaf;	/* XID response lsb.1111F101.
				 * F=0 (no poll command; unsolicited frame) */
	msg->xid_info[0] = 0x81;	/* XID format identifier */
	msg->xid_info[1] = 1;	/* LLC types/classes: Type 1 LLC */
	msg->xid_info[2] = 0;	/* XID sender's receive window size (RW) */

	skb->dev = dev;
	skb->protocol = eth_type_trans(skb, dev);
	memset(skb->cb, 0, sizeof(skb->cb));
	netif_rx_ni(skb);
}

#define NL80211_EXT_FEATURE_CAN_REPLACE_PTK0 -1

int ieee80211_get_vht_max_nss(struct ieee80211_vht_cap *cap,
			      enum ieee80211_vht_chanwidth bw,
			      int mcs, bool ext_nss_bw_capable);
#endif /* >= 4.20 */

/*
 * Upstream this is on 4.16+, but it was backported to chromeos 4.4
 * and 4.14.
 */
#if LINUX_VERSION_IS_LESS(4,4,0)
static inline void sk_pacing_shift_update(struct sock *sk, int val)
{
#if LINUX_VERSION_IS_GEQ(4,4,0)
	if (!sk || !sk_fullsock(sk) || sk->sk_pacing_shift == val)
		return;
	sk->sk_pacing_shift = val;
#endif /* >= 4.4 */
}
#endif /* < 4.4 */

#if CFG80211_VERSION < KERNEL_VERSION(4,19,0)
#define NL80211_EXT_FEATURE_SCAN_RANDOM_SN		-1
#define NL80211_EXT_FEATURE_SCAN_MIN_PREQ_CONTENT	-1
#endif

#if CFG80211_VERSION < KERNEL_VERSION(4,20,0)
#define NL80211_EXT_FEATURE_ENABLE_FTM_RESPONDER	-1
#endif

#if CFG80211_VERSION < KERNEL_VERSION(4,17,0)
#define NL80211_EXT_FEATURE_CONTROL_PORT_OVER_NL80211	-1

/* define it here so we can set the values in mac80211... */
struct sta_opmode_info {
	u32 changed;
	enum nl80211_smps_mode smps_mode;
	enum nl80211_chan_width bw;
	u8 rx_nss;
};

#define STA_OPMODE_MAX_BW_CHANGED	0
#define STA_OPMODE_SMPS_MODE_CHANGED	0
#define STA_OPMODE_N_SS_CHANGED		0

/* ...but make the user an empty function, since we don't have it in cfg80211 */
#define cfg80211_sta_opmode_change_notify(...)  do { } while (0)

/*
 * we should never call this function since we force
 * cfg_control_port_over_nl80211 to be 0.
 */
#define cfg80211_rx_control_port(...) do { } while (0)

#define cfg_control_port_over_nl80211(params) 0
#else
#if CFG80211_VERSION >= KERNEL_VERSION(4,17,0) && \
	CFG80211_VERSION < KERNEL_VERSION(4,18,0)
static inline bool iwl7000_cfg80211_rx_control_port(struct net_device *dev,
				    struct sk_buff *skb, bool unencrypted)
{
	struct ethhdr *ehdr;

	/*
	 * Try to linearize the skb, because in 4.17
	 * cfg80211_rx_control_port() is broken and needs it to be
	 * linear.  If it fails, too bad, we fail too.
	 */
	if (skb_linearize(skb))
		return false;

	ehdr = eth_hdr(skb);

	return cfg80211_rx_control_port(dev, skb->data, skb->len,
				ehdr->h_source,
				be16_to_cpu(skb->protocol), unencrypted);
}
#define cfg80211_rx_control_port iwl7000_cfg80211_rx_control_port
#endif
#define cfg_control_port_over_nl80211(params) (params)->control_port_over_nl80211
#endif

#if CFG80211_VERSION < KERNEL_VERSION(4,18,0)
#define NL80211_EXT_FEATURE_TXQS -1

/*
 * This function just allocates tidstats and returns 0 if it
 * succeeded.  Since pre-4.18 tidstats is pre-allocated as part of
 * sinfo, we can simply return 0 because it's already allocated.
 */
#define cfg80211_sinfo_alloc_tid_stats(...) 0

#define WIPHY_PARAM_TXQ_LIMIT		0
#define WIPHY_PARAM_TXQ_MEMORY_LIMIT	0
#define WIPHY_PARAM_TXQ_QUANTUM		0

#define ieee80211_data_to_8023_exthdr iwl7000_ieee80211_data_to_8023_exthdr
int ieee80211_data_to_8023_exthdr(struct sk_buff *skb, struct ethhdr *ehdr,
				  const u8 *addr, enum nl80211_iftype iftype,
				  u8 data_offset);
#else
static inline int
backport_cfg80211_sinfo_alloc_tid_stats(struct station_info *sinfo, gfp_t gfp)
{
	int ret;
	cfg_station_info_t cfg_info = {};

	ret = cfg80211_sinfo_alloc_tid_stats(&cfg_info, gfp);
	if (ret)
		return ret;

	sinfo->pertid = cfg_info.pertid;

	return 0;
}
#define cfg80211_sinfo_alloc_tid_stats backport_cfg80211_sinfo_alloc_tid_stats
#endif

#if CFG80211_VERSION < KERNEL_VERSION(4,19,0)
#define NL80211_SCAN_FLAG_RANDOM_SN		0
#define NL80211_SCAN_FLAG_MIN_PREQ_CONTENT	0
#endif /* CFG80211_VERSION < KERNEL_VERSION(4,19,0) */

#if CFG80211_VERSION < KERNEL_VERSION(4,20,0)
enum nl80211_ftm_responder_stats {
	__NL80211_FTM_STATS_INVALID,
	NL80211_FTM_STATS_SUCCESS_NUM,
	NL80211_FTM_STATS_PARTIAL_NUM,
	NL80211_FTM_STATS_FAILED_NUM,
	NL80211_FTM_STATS_ASAP_NUM,
	NL80211_FTM_STATS_NON_ASAP_NUM,
	NL80211_FTM_STATS_TOTAL_DURATION_MSEC,
	NL80211_FTM_STATS_UNKNOWN_TRIGGERS_NUM,
	NL80211_FTM_STATS_RESCHEDULE_REQUESTS_NUM,
	NL80211_FTM_STATS_OUT_OF_WINDOW_TRIGGERS_NUM,
	NL80211_FTM_STATS_PAD,

	/* keep last */
	__NL80211_FTM_STATS_AFTER_LAST,
	NL80211_FTM_STATS_MAX = __NL80211_FTM_STATS_AFTER_LAST - 1
};

struct cfg80211_ftm_responder_stats {
	u32 filled;
	u32 success_num;
	u32 partial_num;
	u32 failed_num;
	u32 asap_num;
	u32 non_asap_num;
	u64 total_duration_ms;
	u32 unknown_triggers_num;
	u32 reschedule_requests_num;
	u32 out_of_window_triggers_num;
};
#endif /* CFG80211_VERSION < KERNEL_VERSION(4,20,0) */

#ifndef ETH_P_PREAUTH
#define ETH_P_PREAUTH  0x88C7	/* 802.11 Preauthentication */
#endif

/* This was actually added in 4.12, but chromeos backported to 3.18 */
#if LINUX_VERSION_IS_LESS(3,18,0)
#include <net/flow_keys.h>
#include <linux/jhash.h>

static inline u32 skb_get_hash_perturb(struct sk_buff *skb, u32 key)
{
	struct flow_keys keys;

	skb_flow_dissect(skb, &keys);
	return jhash_3words((__force u32)keys.dst,
			    (__force u32)keys.src ^ keys.ip_proto,
			    (__force u32)keys.ports, key);
}
#endif /* LINUX_VERSION_IS_LESS(4,2,0) */


#if CFG80211_VERSION < KERNEL_VERSION(4,21,0)
enum nl80211_preamble {
	NL80211_PREAMBLE_LEGACY,
	NL80211_PREAMBLE_HT,
	NL80211_PREAMBLE_VHT,
	NL80211_PREAMBLE_DMG,
};

enum nl80211_peer_measurement_status {
	NL80211_PMSR_STATUS_SUCCESS,
	NL80211_PMSR_STATUS_REFUSED,
	NL80211_PMSR_STATUS_TIMEOUT,
	NL80211_PMSR_STATUS_FAILURE,
};

enum nl80211_peer_measurement_type {
	NL80211_PMSR_TYPE_INVALID,

	NL80211_PMSR_TYPE_FTM,

	NUM_NL80211_PMSR_TYPES,
	NL80211_PMSR_TYPE_MAX = NUM_NL80211_PMSR_TYPES - 1
};

enum nl80211_peer_measurement_ftm_failure_reasons {
	NL80211_PMSR_FTM_FAILURE_UNSPECIFIED,
	NL80211_PMSR_FTM_FAILURE_NO_RESPONSE,
	NL80211_PMSR_FTM_FAILURE_REJECTED,
	NL80211_PMSR_FTM_FAILURE_WRONG_CHANNEL,
	NL80211_PMSR_FTM_FAILURE_PEER_NOT_CAPABLE,
	NL80211_PMSR_FTM_FAILURE_INVALID_TIMESTAMP,
	NL80211_PMSR_FTM_FAILURE_PEER_BUSY,
	NL80211_PMSR_FTM_FAILURE_BAD_CHANGED_PARAMS,
};

struct cfg80211_pmsr_ftm_result {
	const u8 *lci;
	const u8 *civicloc;
	unsigned int lci_len;
	unsigned int civicloc_len;
	enum nl80211_peer_measurement_ftm_failure_reasons failure_reason;
	u32 num_ftmr_attempts, num_ftmr_successes;
	s16 burst_index;
	u8 busy_retry_time;
	u8 num_bursts_exp;
	u8 burst_duration;
	u8 ftms_per_burst;
	s32 rssi_avg;
	s32 rssi_spread;
	struct rate_info tx_rate, rx_rate;
	s64 rtt_avg;
	s64 rtt_variance;
	s64 rtt_spread;
	s64 dist_avg;
	s64 dist_variance;
	s64 dist_spread;

	u16 num_ftmr_attempts_valid:1,
	    num_ftmr_successes_valid:1,
	    rssi_avg_valid:1,
	    rssi_spread_valid:1,
	    tx_rate_valid:1,
	    rx_rate_valid:1,
	    rtt_avg_valid:1,
	    rtt_variance_valid:1,
	    rtt_spread_valid:1,
	    dist_avg_valid:1,
	    dist_variance_valid:1,
	    dist_spread_valid:1;
};

struct cfg80211_pmsr_result {
	u64 host_time, ap_tsf;
	enum nl80211_peer_measurement_status status;

	u8 addr[ETH_ALEN];

	u8 final:1,
	   ap_tsf_valid:1;

	enum nl80211_peer_measurement_type type;

	union {
		struct cfg80211_pmsr_ftm_result ftm;
	};
};

struct cfg80211_pmsr_ftm_request_peer {
	enum nl80211_preamble preamble;
	u16 burst_period;
	u8 requested:1,
	   asap:1,
	   request_lci:1,
	   request_civicloc:1;
	u8 num_bursts_exp;
	u8 burst_duration;
	u8 ftms_per_burst;
	u8 ftmr_retries;
};

struct cfg80211_pmsr_request_peer {
	u8 addr[ETH_ALEN];
	struct cfg80211_chan_def chandef;
	u8 report_ap_tsf:1;
	struct cfg80211_pmsr_ftm_request_peer ftm;
};

struct cfg80211_pmsr_request {
	u64 cookie;
	void *drv_data;
	u32 n_peers;
	u32 nl_portid;

	u32 timeout;

	u8 mac_addr[ETH_ALEN] __aligned(2);
	u8 mac_addr_mask[ETH_ALEN] __aligned(2);

	struct list_head list;

	struct cfg80211_pmsr_request_peer peers[];
};

static inline void cfg80211_pmsr_report(struct wireless_dev *wdev,
					struct cfg80211_pmsr_request *req,
					struct cfg80211_pmsr_result *result,
					gfp_t gfp)
{
	/* nothing */
}

static inline void cfg80211_pmsr_complete(struct wireless_dev *wdev,
					  struct cfg80211_pmsr_request *req,
					  gfp_t gfp)
{
	kfree(req);
}

#endif /* CFG80211_VERSION < KERNEL_VERSION(4,21,0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0)
static inline u64 ether_addr_to_u64(const u8 *addr)
{
	u64 u = 0;
	int i;

	for (i = 0; i < ETH_ALEN; i++)
		u = u << 8 | addr[i];

	return u;
}

static inline void u64_to_ether_addr(u64 u, u8 *addr)
{
	int i;

	for (i = ETH_ALEN - 1; i >= 0; i--) {
		addr[i] = u & 0xff;
		u = u >> 8;
	}
}
#endif /* < 4,11,0 */

#if CFG80211_VERSION < KERNEL_VERSION(4, 19, 0)
static inline void
ieee80211_sband_set_num_iftypes_data(struct ieee80211_supported_band *sband,
				     u16 n)
{
}

static inline u16
ieee80211_sband_get_num_iftypes_data(struct ieee80211_supported_band *sband)
{
	return 0;
}

static inline void
ieee80211_sband_set_iftypes_data(struct ieee80211_supported_band *sband,
				 const struct ieee80211_sband_iftype_data *data)
{
}

static inline struct ieee80211_sband_iftype_data *
ieee80211_sband_get_iftypes_data(struct ieee80211_supported_band *sband)
{
	return NULL;
}

static inline struct ieee80211_sband_iftype_data *
ieee80211_sband_get_iftypes_data_entry(struct ieee80211_supported_band *sband,
				       u16 i)
{
	WARN_ONCE(1,
		  "Tried to use unsupported sband iftype data\n");
	return NULL;
}
#else  /* CFG80211_VERSION < KERNEL_VERSION(4,19,0) */
static inline void
ieee80211_sband_set_num_iftypes_data(struct ieee80211_supported_band *sband,
				     u16 n)
{
	sband->n_iftype_data = n;
}

static inline u16
ieee80211_sband_get_num_iftypes_data(struct ieee80211_supported_band *sband)
{
	return sband->n_iftype_data;
}

static inline void
ieee80211_sband_set_iftypes_data(struct ieee80211_supported_band *sband,
				 const struct ieee80211_sband_iftype_data *data)
{
	sband->iftype_data = data;
}

static inline const struct ieee80211_sband_iftype_data *
ieee80211_sband_get_iftypes_data(struct ieee80211_supported_band *sband)
{
	return sband->iftype_data;
}

static inline const struct ieee80211_sband_iftype_data *
ieee80211_sband_get_iftypes_data_entry(struct ieee80211_supported_band *sband,
				       u16 i)
{
	return &sband->iftype_data[i];
}
#endif /* CFG80211_VERSION < KERNEL_VERSION(4,19,0) */

#if CFG80211_VERSION < KERNEL_VERSION(5,1,0)
static inline int cfg80211_vendor_cmd_get_sender(struct wiphy *wiphy)
{
	/* cfg80211 doesn't really let us backport this */
	return 0;
}

static inline struct sk_buff *
cfg80211_vendor_event_alloc_ucast(struct wiphy *wiphy,
				  struct wireless_dev *wdev,
				  unsigned int portid, int approxlen,
				  int event_idx, gfp_t gfp)
{
	/*
	 * We might be able to fake backporting this, but not the
	 * associated changes to __cfg80211_send_event_skb(), at
	 * least not without duplicating all that code.
	 * And in any case, we cannot backport the get_sender()
	 * function above properly, so we might as well ignore
	 * this all.
	 */
	return NULL;
}

static inline const struct element *
cfg80211_find_elem(u8 eid, const u8 *ies, int len)
{
	return (void *)cfg80211_find_ie(eid, ies, len);
}

static inline const struct element *
cfg80211_find_ext_elem(u8 ext_eid, const u8 *ies, int len)
{
	return (void *)cfg80211_find_ext_ie(ext_eid, ies, len);
}

#define IEEE80211_DEFAULT_AIRTIME_WEIGHT       256

#endif /* CFG80211_VERSION < KERNEL_VERSION(5,1,0) */

#if CFG80211_VERSION < KERNEL_VERSION(5,2,0)
#define NL80211_EXT_FEATURE_EXT_KEY_ID -1
#define NL80211_EXT_FEATURE_AIRTIME_FAIRNESS -1
#endif /* CFG80211_VERSION < KERNEL_VERSION(5,2,0) */

#if CFG80211_VERSION < KERNEL_VERSION(5,3,0)
static inline void cfg80211_bss_iter(struct wiphy *wiphy,
				     struct cfg80211_chan_def *chandef,
				     void (*iter)(struct wiphy *wiphy,
						  struct cfg80211_bss *bss,
						  void *data),
				     void *iter_data)
{
	/*
	 * It might be possible to backport this function, but that would
	 * require duplicating large portions of data structure/code, so
	 * leave it empty for now.
	 */
}
#endif /* CFG80211_VERSION < KERNEL_VERSION(5,3,0) */

#if CFG80211_VERSION < KERNEL_VERSION(5,4,0)
static inline bool nl80211_is_6ghz(enum nl80211_band band)
{
	return false;
}
#else
static inline bool nl80211_is_6ghz(enum nl80211_band band)
{
	return band == NL80211_BAND_6GHZ;
}
#endif /* CFG80211_VERSION < KERNEL_VERSION(5,4,0) */

#if CFG80211_VERSION < KERNEL_VERSION(9,9,9)
#define ieee80211_preamble_he() 0
#define ftm_non_trigger_based(peer)	0
#define ftm_trigger_based(peer)	0
#else
#define ftm_non_trigger_based(peer)	((peer)->ftm.non_trigger_based)
#define ftm_trigger_based(peer)	((peer)->ftm.trigger_based)
#define ieee80211_preamble_he() BIT(NL80211_PREAMBLE_HE)
#endif
