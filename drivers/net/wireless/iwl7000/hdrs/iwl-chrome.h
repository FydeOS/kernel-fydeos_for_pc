#ifndef __IWL_CHROME
#define __IWL_CHROME
/* This file is pre-included from the Makefile (cc command line)
 *
 * ChromeOS backport definitions
 * Copyright (C) 2016-2017 Intel Deutschland GmbH
 * Copyright (C) 2018-2019 Intel Corporation
 */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/idr.h>
#include <linux/vmalloc.h>

/* get the CPTCFG_* preprocessor symbols */
#include <hdrs/config.h>

#include <hdrs/mac80211-exp.h>

#define LINUX_VERSION_IS_LESS(x1,x2,x3) (LINUX_VERSION_CODE < KERNEL_VERSION(x1,x2,x3))
#define LINUX_VERSION_IS_GEQ(x1,x2,x3)  (LINUX_VERSION_CODE >= KERNEL_VERSION(x1,x2,x3))
#define LINUX_VERSION_IN_RANGE(x1,x2,x3, y1,y2,y3) \
        (LINUX_VERSION_IS_GEQ(x1,x2,x3) && LINUX_VERSION_IS_LESS(y1,y2,y3))
#define LINUX_BACKPORT(sym) backport_ ## sym

/* this must be before including rhashtable.h */
#if LINUX_VERSION_IS_LESS(4,15,0)
#ifndef CONFIG_LOCKDEP
struct lockdep_map { };
#endif /* CONFIG_LOCKDEP */
#endif /* LINUX_VERSION_IS_LESS(4,15,0) */

/* also this... */
#if LINUX_VERSION_IS_LESS(3,12,0)
#ifdef CONFIG_PROVE_LOCKING
 #define lock_acquire_exclusive(l, s, t, n, i)         lock_acquire(l, s, t, 0, 2, n, i)
 #define lock_acquire_shared(l, s, t, n, i)            lock_acquire(l, s, t, 1, 2, n, i)
 #define lock_acquire_shared_recursive(l, s, t, n, i)  lock_acquire(l, s, t, 2, 2, n, i)
#else
# define spin_acquire(l, s, t, i)              do { } while (0)
# define spin_release(l, n, i)                 do { } while (0)
 #define lock_acquire_exclusive(l, s, t, n, i)         lock_acquire(l, s, t, 0, 1, n, i)
 #define lock_acquire_shared(l, s, t, n, i)            lock_acquire(l, s, t, 1, 1, n, i)
 #define lock_acquire_shared_recursive(l, s, t, n, i)  lock_acquire(l, s, t, 2, 1, n, i)
#endif
#endif

/* include rhashtable this way to get our copy if another exists */
#include <linux/list_nulls.h>
#ifndef NULLS_MARKER
#define NULLS_MARKER(value) (1UL | (((long)value) << 1))
#endif
#include "linux/rhashtable.h"

#include <net/genetlink.h>
#include <linux/crypto.h>
#include <linux/moduleparam.h>
#include <linux/debugfs.h>
#include <linux/hrtimer.h>
#include <crypto/algapi.h>
#include <linux/pci.h>
#include <linux/if_vlan.h>
#include <linux/overflow.h>
#include "net/fq.h"

#if LINUX_VERSION_IS_LESS(3,20,0)
#define get_net_ns_by_fd LINUX_BACKPORT(get_net_ns_by_fd)
static inline struct net *get_net_ns_by_fd(int fd)
{
	return ERR_PTR(-EINVAL);
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,17,0)
static inline u64 ktime_get_ns(void)
{
	return ktime_to_ns(ktime_get());
}

static inline u64 ktime_get_real_ns(void)
{
	return ktime_to_ns(ktime_get_real());
}
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3,17,0) */

/*
 * Need to include these here, otherwise we get the regular kernel ones
 * pre-including them makes it work, even though later the kernel ones
 * are included again, but they (hopefully) have the same include guard
 * ifdef/define so the second time around nothing happens
 *
 * We still keep them in the correct directory so if they don't exist in
 * the kernel (e.g. bitfield.h won't) the preprocessor can find them.
 */
#include <hdrs/linux/ieee80211.h>
#include <hdrs/linux/average.h>
#include <hdrs/linux/bitfield.h>
#include <hdrs/net/ieee80211_radiotap.h>
#define IEEE80211RADIOTAP_H 1 /* older kernels used this include protection */

/* mac80211 & backport - order matters, need this inbetween */
#include <hdrs/mac80211-bp.h>

#include <hdrs/net/codel.h>
#include <hdrs/net/mac80211.h>

/* artifacts of backports - never in upstream */
#define genl_info_snd_portid(__genl_info) (__genl_info->snd_portid)
#define NETLINK_CB_PORTID(__skb) NETLINK_CB(cb->skb).portid
#define netlink_notify_portid(__notify) __notify->portid

static inline struct netlink_ext_ack *genl_info_extack(struct genl_info *info)
{
#if LINUX_VERSION_IS_GEQ(4,12,0)
	return info->extack;
#else
	return NULL;
#endif
}

/* things that may or may not be upstream depending on the version */
#ifndef ETH_P_802_3_MIN
#define ETH_P_802_3_MIN 0x0600
#endif

#ifndef U32_MAX
#define U32_MAX		((u32)~0U)
#endif

#ifndef U8_MAX
#define U8_MAX		((u8)~0U)
#endif

#ifndef S8_MAX
#define S8_MAX		((s8)(U8_MAX>>1))
#endif

#ifndef S8_MIN
#define S8_MIN		((s8)(-S8_MAX - 1))
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,9,0)
/* backport IDR APIs */
static inline void iwl7000_idr_destroy(struct idr *idp)
{
	idr_remove_all(idp);
	idr_destroy(idp);
}
#define idr_destroy(idp) iwl7000_idr_destroy(idp)

static inline int idr_alloc(struct idr *idr, void *ptr, int start, int end,
			    gfp_t gfp_mask)
{
	int id, ret;

	do {
		if (!idr_pre_get(idr, gfp_mask))
			return -ENOMEM;
		ret = idr_get_new_above(idr, ptr, start, &id);
		if (!ret && id > end) {
			idr_remove(idr, id);
			ret = -ENOSPC;
		}
	} while (ret == -EAGAIN);

	return ret ? ret : id;
}

static inline void idr_preload(gfp_t gfp_mask)
{
}

static inline void idr_preload_end(void)
{
}

#ifdef CONFIG_PM
static inline bool pm_runtime_active(struct device *dev)
{
	return dev->power.runtime_status == RPM_ACTIVE ||
		dev->power.disable_depth;
}
#else
static inline bool pm_runtime_active(struct device *dev) { return true; }
#endif /* CONFIG_PM */

#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3,9,0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,11,0)
#define netdev_notifier_info_to_dev(ndev)	ndev

size_t sg_pcopy_from_buffer(struct scatterlist *sgl, unsigned int nents,
			    const void *buf, size_t buflen, off_t skip);
size_t sg_pcopy_to_buffer(struct scatterlist *sgl, unsigned int nents,
			  void *buf, size_t buflen, off_t skip);
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3,11,0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,12,0)
/* PCIe device capabilities flags have been renamed in (upstream)
 * commit d2ab1fa68c61f01b28ab0859a972c892d81f5d32 (PCI: Rename PCIe
 * capability definitions to follow convention).  This was just a
 * clean rename, without any functional changes.  We use one of the
 * renamed flags, so define it to the old one.
 */
#define PCI_EXP_DEVCTL2_LTR_EN PCI_EXP_LTR_EN

#define PTR_ERR_OR_ZERO(p) PTR_RET(p)

#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3,12,0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,13,0)
#if !defined(CONFIG_PROVE_LOCKING)
static inline bool lockdep_rtnl_is_held(void)
{
	return true;
}
#endif /* !defined(CONFIG_PROVE_LOCKING) */

#define __genl_const
static inline int
_genl_register_family_with_ops_grps(struct genl_family *family,
				    struct genl_ops *ops, size_t n_ops,
				    struct genl_multicast_group *mcgrps,
				    size_t n_mcgrps)
{
	int ret, i;

	ret = genl_register_family_with_ops(family, ops, n_ops);
	if (ret)
		return ret;
	for (i = 0; i < n_mcgrps; i++) {
		ret = genl_register_mc_group(family, &mcgrps[i]);
		if (ret) {
			genl_unregister_family(family);
			return ret;
		}
	}

	return 0;
}
#else
#define __genl_const const
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,14,0)
#define ether_addr_equal_unaligned __iwl7000_ether_addr_equal_unaligned
static inline bool ether_addr_equal_unaligned(const u8 *addr1, const u8 *addr2)
{
	return memcmp(addr1, addr2, ETH_ALEN) == 0;
}
#endif

#if LINUX_VERSION_IS_GEQ(5,3,0)
/*
 * In v5.3, this function was renamed, so rename it here for v5.3+.
 * When we merge v5.3 back from upstream, the opposite should be done
 * (i.e. we will have _boottime_ and need to rename to _boot_ in <
 * v5.3 instead).
*/
#define ktime_get_boot_ns ktime_get_boottime_ns
#endif /* > 5.3.0 */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,17,0)
#define kvfree __iwl7000_kvfree
static inline void kvfree(const void *addr)
{
	if (is_vmalloc_addr(addr))
		vfree(addr);
	else
		kfree(addr);
}

static inline u64 ktime_get_boot_ns(void)
{
	return ktime_to_ns(ktime_get_boottime());
}

/* interface name assignment types (sysfs name_assign_type attribute) */
#define NET_NAME_UNKNOWN	0	/* unknown origin (not exposed to userspace) */
#define NET_NAME_ENUM		1	/* enumerated by kernel */
#define NET_NAME_PREDICTABLE	2	/* predictably named by the kernel */
#define NET_NAME_USER		3	/* provided by user-space */
#define NET_NAME_RENAMED	4	/* renamed by user-space */

static inline struct net_device *
backport_alloc_netdev_mqs(int sizeof_priv, const char *name,
			  unsigned char name_assign_type,
			  void (*setup)(struct net_device *),
			  unsigned int txqs, unsigned int rxqs)
{
	return alloc_netdev_mqs(sizeof_priv, name, setup, txqs, rxqs);
}

#define alloc_netdev_mqs backport_alloc_netdev_mqs

#undef alloc_netdev
static inline struct net_device *
backport_alloc_netdev(int sizeof_priv, const char *name,
		      unsigned char name_assign_type,
		      void (*setup)(struct net_device *))
{
	return backport_alloc_netdev_mqs(sizeof_priv, name, name_assign_type,
					 setup, 1, 1);
}
#define alloc_netdev backport_alloc_netdev

char *devm_kvasprintf(struct device *dev, gfp_t gfp, const char *fmt,
		      va_list ap);
char *devm_kasprintf(struct device *dev, gfp_t gfp, const char *fmt, ...);
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3,17,0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,2,0)
#include <crypto/scatterwalk.h>
#include <crypto/aead.h>

static inline struct scatterlist *scatterwalk_ffwd(struct scatterlist dst[2],
					    struct scatterlist *src,
					    unsigned int len)
{
	for (;;) {
		if (!len)
			return src;

		if (src->length > len)
			break;

		len -= src->length;
		src = sg_next(src);
	}

	sg_init_table(dst, 2);
	sg_set_page(dst, sg_page(src), src->length - len, src->offset + len);
	scatterwalk_crypto_chain(dst, sg_next(src), 0, 2);

	return dst;
}



struct aead_old_request {
	struct scatterlist srcbuf[2];
	struct scatterlist dstbuf[2];
	struct aead_request subreq;
};

static inline unsigned int iwl7000_crypto_aead_reqsize(struct crypto_aead *tfm)
{
	return crypto_aead_crt(tfm)->reqsize + sizeof(struct aead_old_request);
}
#define crypto_aead_reqsize iwl7000_crypto_aead_reqsize

static inline struct aead_request *
crypto_backport_convert(struct aead_request *req)
{
	struct aead_old_request *nreq = aead_request_ctx(req);
	struct crypto_aead *aead = crypto_aead_reqtfm(req);
	struct scatterlist *src, *dst;

	src = scatterwalk_ffwd(nreq->srcbuf, req->src, req->assoclen);
	dst = req->src == req->dst ?
	      src : scatterwalk_ffwd(nreq->dstbuf, req->dst, req->assoclen);

	aead_request_set_tfm(&nreq->subreq, aead);
	aead_request_set_callback(&nreq->subreq, aead_request_flags(req),
				  req->base.complete, req->base.data);
	aead_request_set_crypt(&nreq->subreq, src, dst, req->cryptlen,
			       req->iv);
	aead_request_set_assoc(&nreq->subreq, req->src, req->assoclen);

	return &nreq->subreq;
}

static inline int iwl7000_crypto_aead_encrypt(struct aead_request *req)
{
	return crypto_aead_encrypt(crypto_backport_convert(req));
}
#define crypto_aead_encrypt iwl7000_crypto_aead_encrypt

static inline int iwl7000_crypto_aead_decrypt(struct aead_request *req)
{
	return crypto_aead_decrypt(crypto_backport_convert(req));
}
#define crypto_aead_decrypt iwl7000_crypto_aead_decrypt

#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(4,2,0) */

/* Note: this stuff is included in in chromeos-3.14 and 3.18.
 * Additionally, we check for <4.2, since that's when it was
 * added upstream.
 */
#if (LINUX_VERSION_CODE != KERNEL_VERSION(3,14,0)) &&	\
    (LINUX_VERSION_CODE != KERNEL_VERSION(3,18,0)) &&	\
    (LINUX_VERSION_CODE < KERNEL_VERSION(4,2,0))
static inline void aead_request_set_ad(struct aead_request *req,
				       unsigned int assoclen)
{
	req->assoclen = assoclen;
}

static inline void kernel_param_lock(struct module *mod)
{
	__kernel_param_lock();
}

static inline void kernel_param_unlock(struct module *mod)
{
	__kernel_param_unlock();
}
#endif /* !3.14 && <4.2 */

#ifndef list_first_entry_or_null
#define list_first_entry_or_null(ptr, type, member) \
	(!list_empty(ptr) ? list_first_entry(ptr, type, member) : NULL)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0)
#ifdef CONFIG_DEBUG_FS
struct dentry *iwl_debugfs_create_bool(const char *name, umode_t mode,
				       struct dentry *parent, bool *value);
#else
static inline struct dentry *
iwl_debugfs_create_bool(const char *name, umode_t mode,
			struct dentry *parent, bool *value)
{
	return ERR_PTR(-ENODEV);
}
#endif /* CONFIG_DEBUG_FS */
#define debugfs_create_bool iwl_debugfs_create_bool

#define tso_t __iwl7000_tso_t
struct tso_t {
	int next_frag_idx;
	void *data;
	size_t size;
	u16 ip_id;
	bool ipv6;
	u32 tcp_seq;
};

int tso_count_descs(struct sk_buff *skb);
void tso_build_hdr(struct sk_buff *skb, char *hdr, struct tso_t *tso,
		   int size, bool is_last);
void tso_start(struct sk_buff *skb, struct tso_t *tso);
void tso_build_data(struct sk_buff *skb, struct tso_t *tso, int size);

#endif /* < 4.4.0 */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,19,0))
static inline int
skb_ensure_writable(struct sk_buff *skb, int write_len)
{
	if (!pskb_may_pull(skb, write_len))
		return -ENOMEM;

	if (!skb_cloned(skb) || skb_clone_writable(skb, write_len))
		return 0;

	return pskb_expand_head(skb, 0, 0, GFP_ATOMIC);
}
#endif

#ifndef NETIF_F_CSUM_MASK
#define NETIF_F_CSUM_MASK (NETIF_F_V4_CSUM | NETIF_F_V6_CSUM)
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,14,0))
static inline int
pci_enable_msix_range(struct pci_dev *dev, struct msix_entry *entries,
		      int minvec, int maxvec)
{
	return -EOPNOTSUPP;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 19, 0)
void netdev_rss_key_fill(void *buffer, size_t len);
#endif

#if CFG80211_VERSION < KERNEL_VERSION(4, 1, 0) &&	\
	CFG80211_VERSION >= KERNEL_VERSION(3, 14, 0)
static inline struct sk_buff *
iwl7000_cfg80211_vendor_event_alloc(struct wiphy *wiphy,
				    struct wireless_dev *wdev,
				    int approxlen, int event_idx, gfp_t gfp)
{
	return cfg80211_vendor_event_alloc(wiphy, approxlen, event_idx, gfp);
}

#define cfg80211_vendor_event_alloc iwl7000_cfg80211_vendor_event_alloc
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
static inline void page_ref_inc(struct page *page)
{
	atomic_inc(&page->_count);
}

int __must_check kstrtobool(const char *s, bool *res);
int __must_check kstrtobool_from_user(const char __user *s, size_t count, bool *res);
#endif /* < 4.6 */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,7,0)

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,5,0) ||	\
     LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0))
/* We don't really care much about alignment, since nl80211 isn't using
 * this for hot paths. So just implement it using nla_put_u64().
 */
static inline int nla_put_u64_64bit(struct sk_buff *skb, int attrtype,
				    u64 value, int padattr)
{
	return nla_put_u64(skb, attrtype, value);
}
#endif /* < 4.4 && > 4.5 */

#define nla_put_s64 iwl7000_nla_put_s64
static inline int nla_put_s64(struct sk_buff *skb, int attrtype, s64 value,
			      int padattr)
{
	return nla_put_u64(skb, attrtype, value);
}
void dev_coredumpsg(struct device *dev, struct scatterlist *table,
		    size_t datalen, gfp_t gfp);
#endif /* < 4.7 */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,10,0)
/* on earlier kernels, genl_unregister_family() modifies the struct */
#define __genl_ro_after_init
#else
#define __genl_ro_after_init __ro_after_init
#endif

#ifndef __BUILD_BUG_ON_NOT_POWER_OF_2
#define __BUILD_BUG_ON_NOT_POWER_OF_2(...)
#endif

#if LINUX_VERSION_IS_LESS(3,11,0)
#ifndef DEVICE_ATTR_RO
#define DEVICE_ATTR_RO(_name) \
struct device_attribute dev_attr_ ## _name = __ATTR_RO(_name);
#endif
#ifndef DEVICE_ATTR_RW
#define DEVICE_ATTR_RW(_name) \
struct device_attribute dev_attr_ ## _name = __ATTR_RW(_name)
#endif
#endif

#define ATTRIBUTE_GROUPS_BACKPORT(_name) \
static struct BP_ATTR_GRP_STRUCT _name##_dev_attrs[ARRAY_SIZE(_name##_attrs)];\
static void init_##_name##_attrs(void)				\
{									\
	int i;								\
	for (i = 0; _name##_attrs[i]; i++)				\
		_name##_dev_attrs[i] =				\
			*container_of(_name##_attrs[i],		\
				      struct BP_ATTR_GRP_STRUCT,	\
				      attr);				\
}

#ifndef __ATTRIBUTE_GROUPS
#define __ATTRIBUTE_GROUPS(_name)				\
static const struct attribute_group *_name##_groups[] = {	\
	&_name##_group,						\
	NULL,							\
}
#endif /* __ATTRIBUTE_GROUPS */

#undef ATTRIBUTE_GROUPS
#define ATTRIBUTE_GROUPS(_name)					\
static const struct attribute_group _name##_group = {		\
	.attrs = _name##_attrs,					\
};								\
static inline void init_##_name##_attrs(void) {}		\
__ATTRIBUTE_GROUPS(_name)

#ifndef ETH_P_80221
#define ETH_P_80221	0x8917	/* IEEE 802.21 Media Independent Handover Protocol */
#endif

#ifndef skb_vlan_tag_present
#define skb_vlan_tag_present(__skb)	((__skb)->vlan_tci & VLAN_TAG_PRESENT)
#endif

#ifndef skb_vlan_tag_get
#define skb_vlan_tag_get(__skb)		((__skb)->vlan_tci & ~VLAN_TAG_PRESENT)
#endif

#if LINUX_VERSION_IS_LESS(3,11,0)
/* power efficient workqueues were added in commit 0668106ca386. */
#define system_power_efficient_wq system_wq
#define system_freezable_power_efficient_wq system_freezable_wq
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,0,0)
#define __print_array(array, count, el_size) ""
#endif

#ifndef S32_MAX
#define S32_MAX		((s32)(U32_MAX>>1))
#endif

#ifndef S32_MIN
#define S32_MIN		((s32)(-S32_MAX - 1))
#endif

/* ChromeOS backported this to 3.14, 3.18, etc. - upstream only since 4.1 */
#if LINUX_VERSION_IS_LESS(3,14,0)
static inline int nla_put_in_addr(struct sk_buff *skb, int attrtype,
				  __be32 addr)
{
	return nla_put_be32(skb, attrtype, addr);
}

static inline __be32 nla_get_in_addr(const struct nlattr *nla)
{
	return *(__be32 *) nla_data(nla);
}
#endif /* < 4.1 */

#if LINUX_VERSION_IS_LESS(4,10,0)
static inline void *nla_memdup(const struct nlattr *src, gfp_t gfp)
{
	return kmemdup(nla_data(src), nla_len(src), gfp);
}
#endif

#ifndef GENLMSG_DEFAULT_SIZE
#define GENLMSG_DEFAULT_SIZE (NLMSG_DEFAULT_SIZE - GENL_HDRLEN)
#endif

#if LINUX_VERSION_IS_LESS(4,15,0)
static inline
void backport_genl_dump_check_consistent(struct netlink_callback *cb,
					 void *user_hdr)
{
	struct genl_family dummy_family = {
		.hdrsize = 0,
	};

	genl_dump_check_consistent(cb, user_hdr, &dummy_family);
}
#define genl_dump_check_consistent LINUX_BACKPORT(genl_dump_check_consistent)
#endif /* LINUX_VERSION_IS_LESS(4,15,0) */

#if LINUX_VERSION_IS_LESS(3,13,0)
static inline int __real_genl_register_family(struct genl_family *family)
{
	return genl_register_family(family);
}

/* Needed for the mcgrps pointer */
struct backport_genl_family {
	struct genl_family family;

	unsigned int id, hdrsize, version, maxattr;
	char name[GENL_NAMSIZ];
	bool netnsok;
	bool parallel_ops;

	struct nlattr **attrbuf;

	int (*pre_doit)(struct genl_ops *ops, struct sk_buff *skb,
			struct genl_info *info);

	void (*post_doit)(struct genl_ops *ops, struct sk_buff *skb,
			  struct genl_info *info);

	struct genl_multicast_group *mcgrps;
	struct genl_ops *ops;
	unsigned int n_mcgrps, n_ops;

	struct module *module;
};
#define genl_family LINUX_BACKPORT(genl_family)

int __backport_genl_register_family(struct genl_family *family);

#define genl_register_family LINUX_BACKPORT(genl_register_family)
static inline int
genl_register_family(struct genl_family *family)
{
	family->module = THIS_MODULE;
	return __backport_genl_register_family(family);
}

#define _genl_register_family_with_ops_grps \
	_backport_genl_register_family_with_ops_grps
static inline int
_genl_register_family_with_ops_grps(struct genl_family *family,
				    struct genl_ops *ops, size_t n_ops,
				    struct genl_multicast_group *mcgrps,
				    size_t n_mcgrps)
{
	family->ops = ops;
	family->n_ops = n_ops;
	family->mcgrps = mcgrps;
	family->n_mcgrps = n_mcgrps;
	return genl_register_family(family);
}

#define genl_register_family_with_ops(family, ops)			\
	_genl_register_family_with_ops_grps((family),			\
					    (ops), ARRAY_SIZE(ops),	\
					    NULL, 0)

#define genl_unregister_family backport_genl_unregister_family
int genl_unregister_family(struct genl_family *family);

#define genl_notify(_fam, _skb, _info, _group, _flags)			\
	genl_notify(_skb, genl_info_net(_info),				\
		    genl_info_snd_portid(_info),			\
		    (_fam)->mcgrps[_group].id, _info->nlhdr, _flags)
#define genlmsg_put(_skb, _pid, _seq, _fam, _flags, _cmd)		\
	genlmsg_put(_skb, _pid, _seq, &(_fam)->family, _flags, _cmd)
#ifndef genlmsg_put_reply /* might already be there from _info override above */
#define genlmsg_put_reply(_skb, _info, _fam, _flags, _cmd)		\
	genlmsg_put_reply(_skb, _info, &(_fam)->family, _flags, _cmd)
#endif
#define genlmsg_multicast_netns LINUX_BACKPORT(genlmsg_multicast_netns)
static inline int genlmsg_multicast_netns(struct genl_family *family,
					  struct net *net, struct sk_buff *skb,
					  u32 portid, unsigned int group,
					  gfp_t flags)
{
	if (WARN_ON_ONCE(group >= family->n_mcgrps))
		return -EINVAL;
	group = family->mcgrps[group].id;
	return nlmsg_multicast(
		net->genl_sock,
		skb, portid, group, flags);
}
#define genlmsg_multicast LINUX_BACKPORT(genlmsg_multicast)
static inline int genlmsg_multicast(struct genl_family *family,
				    struct sk_buff *skb, u32 portid,
				    unsigned int group, gfp_t flags)
{
	if (WARN_ON_ONCE(group >= family->n_mcgrps))
		return -EINVAL;
	group = family->mcgrps[group].id;
	return nlmsg_multicast(
		init_net.genl_sock,
		skb, portid, group, flags);
}
static inline int
backport_genlmsg_multicast_allns(struct genl_family *family,
				 struct sk_buff *skb, u32 portid,
				 unsigned int group, gfp_t flags)
{
	if (WARN_ON_ONCE(group >= family->n_mcgrps))
		return -EINVAL;
	group = family->mcgrps[group].id;
	return genlmsg_multicast_allns(skb, portid, group, flags);
}
#define genlmsg_multicast_allns LINUX_BACKPORT(genlmsg_multicast_allns)

#define __genl_const
#else /* < 3.13 */
#define __genl_const const
#if LINUX_VERSION_IS_LESS(4,4,0)
#define genl_notify(_fam, _skb, _info, _group, _flags)			\
	genl_notify(_fam, _skb, genl_info_net(_info),			\
		    genl_info_snd_portid(_info),			\
		    _group, _info->nlhdr, _flags)
#endif /* < 4.4 */
#endif /* < 3.13 */

#if LINUX_VERSION_IS_LESS(4,10,0)
/**
 * genl_family_attrbuf - return family's attrbuf
 * @family: the family
 *
 * Return the family's attrbuf, while validating that it's
 * actually valid to access it.
 *
 * You cannot use this function with a family that has parallel_ops
 * and you can only use it within (pre/post) doit/dumpit callbacks.
 */
#define genl_family_attrbuf LINUX_BACKPORT(genl_family_attrbuf)
static inline struct nlattr **genl_family_attrbuf(struct genl_family *family)
{
	WARN_ON(family->parallel_ops);

	return family->attrbuf;
}

#define __genl_ro_after_init
#else
#define __genl_ro_after_init __ro_after_init
#endif

#ifndef GENL_UNS_ADMIN_PERM
#define GENL_UNS_ADMIN_PERM GENL_ADMIN_PERM
#endif

#if LINUX_VERSION_IS_LESS(4, 1, 0)
#define dev_of_node LINUX_BACKPORT(dev_of_node)
static inline struct device_node *dev_of_node(struct device *dev)
{
#ifndef CONFIG_OF
	return NULL;
#else
	return dev->of_node;
#endif
}
#endif /* LINUX_VERSION_IS_LESS(4, 1, 0) */

#if LINUX_VERSION_IS_LESS(4,12,0)
#include "magic.h"

static inline int nla_validate5(const struct nlattr *head,
				int len, int maxtype,
				const struct nla_policy *policy,
				struct netlink_ext_ack *extack)
{
	return nla_validate(head, len, maxtype, policy);
}
#define nla_validate4 nla_validate
#define nla_validate(...) \
	macro_dispatcher(nla_validate, __VA_ARGS__)(__VA_ARGS__)

static inline int nla_parse6(struct nlattr **tb, int maxtype,
			     const struct nlattr *head,
			     int len, const struct nla_policy *policy,
			     struct netlink_ext_ack *extack)
{
	return nla_parse(tb, maxtype, head, len, policy);
}
#define nla_parse5(...) nla_parse(__VA_ARGS__)
#define nla_parse(...) \
	macro_dispatcher(nla_parse, __VA_ARGS__)(__VA_ARGS__)

static inline int nlmsg_parse6(const struct nlmsghdr *nlh, int hdrlen,
			       struct nlattr *tb[], int maxtype,
			       const struct nla_policy *policy,
			       struct netlink_ext_ack *extack)
{
	return nlmsg_parse(nlh, hdrlen, tb, maxtype, policy);
}
#define nlmsg_parse5 nlmsg_parse
#define nlmsg_parse(...) \
	macro_dispatcher(nlmsg_parse, __VA_ARGS__)(__VA_ARGS__)

static inline int nlmsg_validate5(const struct nlmsghdr *nlh,
				  int hdrlen, int maxtype,
				  const struct nla_policy *policy,
				  struct netlink_ext_ack *extack)
{
	return nlmsg_validate(nlh, hdrlen, maxtype, policy);
}
#define nlmsg_validate4 nlmsg_validate
#define nlmsg_validate(...) \
	macro_dispatcher(nlmsg_validate, __VA_ARGS__)(__VA_ARGS__)

static inline int nla_parse_nested5(struct nlattr *tb[], int maxtype,
				    const struct nlattr *nla,
				    const struct nla_policy *policy,
				    struct netlink_ext_ack *extack)
{
	return nla_parse_nested(tb, maxtype, nla, policy);
}
#define nla_parse_nested4 nla_parse_nested
#define nla_parse_nested(...) \
	macro_dispatcher(nla_parse_nested, __VA_ARGS__)(__VA_ARGS__)

static inline int nla_validate_nested4(const struct nlattr *start, int maxtype,
				       const struct nla_policy *policy,
				       struct netlink_ext_ack *extack)
{
	return nla_validate_nested(start, maxtype, policy);
}
#define nla_validate_nested3 nla_validate_nested
#define nla_validate_nested(...) \
	macro_dispatcher(nla_validate_nested, __VA_ARGS__)(__VA_ARGS__)

#define kvmalloc LINUX_BACKPORT(kvmalloc)
static inline void *kvmalloc(size_t size, gfp_t flags)
{
	gfp_t kmalloc_flags = flags;
	void *ret;

	if ((flags & GFP_KERNEL) != GFP_KERNEL)
		return kmalloc(size, flags);

	if (size > PAGE_SIZE)
		kmalloc_flags |= __GFP_NOWARN | __GFP_NORETRY;

	ret = kmalloc(size, flags);
	if (ret || size < PAGE_SIZE)
		return ret;

	return vmalloc(size);
}

#define kvmalloc_array LINUX_BACKPORT(kvmalloc_array)
static inline void *kvmalloc_array(size_t n, size_t size, gfp_t flags)
{
	size_t bytes;

	if (unlikely(check_mul_overflow(n, size, &bytes)))
		return NULL;

	return kvmalloc(bytes, flags);
}

#define kvzalloc LINUX_BACKPORT(kvzalloc)
static inline void *kvzalloc(size_t size, gfp_t flags)
{
	return kvmalloc(size, flags | __GFP_ZERO);
}

#endif /* LINUX_VERSION_IS_LESS(4,12,0) */

#if LINUX_VERSION_IS_LESS(4,14,0)
static inline void *kvcalloc(size_t n, size_t size, gfp_t flags)
{
	return kvmalloc_array(n, size, flags | __GFP_ZERO);
}
#endif /* LINUX_VERSION_IS_LESS(4,14,0) */

/* avoid conflicts with other headers */
#ifdef is_signed_type
#undef is_signed_type
#endif

#ifndef offsetofend
/**
 * offsetofend(TYPE, MEMBER)
 *
 * @TYPE: The type of the structure
 * @MEMBER: The member within the structure to get the end offset of
 */
#define offsetofend(TYPE, MEMBER) \
	(offsetof(TYPE, MEMBER)	+ sizeof(((TYPE *)0)->MEMBER))
#endif


int __alloc_bucket_spinlocks(spinlock_t **locks, unsigned int *lock_mask,
			     size_t max_size, unsigned int cpu_mult,
			     gfp_t gfp, const char *name,
			     struct lock_class_key *key);

#define alloc_bucket_spinlocks(locks, lock_mask, max_size, cpu_mult, gfp)    \
	({								     \
		static struct lock_class_key key;			     \
		int ret;						     \
									     \
		ret = __alloc_bucket_spinlocks(locks, lock_mask, max_size,   \
					       cpu_mult, gfp, #locks, &key); \
		ret;							\
	})
void free_bucket_spinlocks(spinlock_t *locks);

#ifndef READ_ONCE
#include <linux/types.h>

#define __READ_ONCE_SIZE						\
({									\
	switch (size) {							\
	case 1: *(__u8 *)res = *(volatile __u8 *)p; break;		\
	case 2: *(__u16 *)res = *(volatile __u16 *)p; break;		\
	case 4: *(__u32 *)res = *(volatile __u32 *)p; break;		\
	case 8: *(__u64 *)res = *(volatile __u64 *)p; break;		\
	default:							\
		barrier();						\
		__builtin_memcpy((void *)res, (const void *)p, size);	\
		barrier();						\
	}								\
})

static __always_inline
void __read_once_size(const volatile void *p, void *res, int size)
{
	__READ_ONCE_SIZE;
}

#define __READ_ONCE(x, check)						\
({									\
	union { typeof(x) __val; char __c[1]; } __u;			\
	__read_once_size(&(x), __u.__c, sizeof(x));			\
	__u.__val;							\
})

#define READ_ONCE(x) __READ_ONCE(x, 1)

static __always_inline void __write_once_size(volatile void *p, void *res, int size)
{
	switch (size) {
	case 1: *(volatile __u8 *)p = *(__u8 *)res; break;
	case 2: *(volatile __u16 *)p = *(__u16 *)res; break;
	case 4: *(volatile __u32 *)p = *(__u32 *)res; break;
	case 8: *(volatile __u64 *)p = *(__u64 *)res; break;
	default:
		barrier();
		__builtin_memcpy((void *)p, (const void *)res, size);
		barrier();
	}
}

#define WRITE_ONCE(x, val) \
({							\
	union { typeof(x) __val; char __c[1]; } __u =	\
		{ .__val = (__force typeof(x)) (val) }; \
	__write_once_size(&(x), __u.__c, sizeof(x));	\
	__u.__val;					\
})
#endif

#if LINUX_VERSION_IS_LESS(4,12,0)
#define GENL_SET_ERR_MSG(info, msg) do { } while (0)

static inline int genl_err_attr(struct genl_info *info, int err,
				struct nlattr *attr)
{
#if LINUX_VERSION_IS_GEQ(4,12,0)
	info->extack->bad_attr = attr;
#endif

	return err;
}
#endif

#if LINUX_VERSION_IS_LESS(4,19,0)
#ifndef atomic_fetch_add_unless
static inline int atomic_fetch_add_unless(atomic_t *v, int a, int u)
{
		return __atomic_add_unless(v, a, u);
}
#endif
#endif /* LINUX_VERSION_IS_LESS(4,19,0) */

#if LINUX_VERSION_IS_LESS(4,20,0)
static inline void rcu_head_init(struct rcu_head *rhp)
{
	rhp->func = (void *)~0L;
}

static inline bool
rcu_head_after_call_rcu(struct rcu_head *rhp, void *f)
{
	if (READ_ONCE(rhp->func) == f)
		return true;
	WARN_ON_ONCE(READ_ONCE(rhp->func) != (void *)~0L);
	return false;
}
#endif /* LINUX_VERSION_IS_LESS(4,20,0) */

#if LINUX_VERSION_IS_LESS(5,4,0)
#include <linux/pci-aspm.h>
#endif

#endif /* __IWL_CHROME */
