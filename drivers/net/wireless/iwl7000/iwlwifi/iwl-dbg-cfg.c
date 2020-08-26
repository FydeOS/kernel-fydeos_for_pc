/******************************************************************************
 *
 * This file is provided under a dual BSD/GPLv2 license.  When using or
 * redistributing this file, you may do so under either license.
 *
 * GPL LICENSE SUMMARY
 *
 * Copyright(c) 2013 - 2015, 2019 Intel Corporation. All rights reserved.
 * Copyright(c) 2013 - 2015 Intel Mobile Communications GmbH
 * Copyright(c) 2016        Intel Deutschland GmbH
 * Copyright (C) 2019 Intel Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * The full GNU General Public License is included in this distribution
 * in the file called COPYING.
 *
 * Contact Information:
 *  Intel Linux Wireless <linuxwifi@intel.com>
 * Intel Corporation, 5200 N.E. Elam Young Parkway, Hillsboro, OR 97124-6497
 *
 * BSD LICENSE
 *
 * Copyright(c) 2013 - 2015, 2019  Intel Corporation. All rights reserved.
 * Copyright(c) 2013 - 2015 Intel Mobile Communications GmbH
 * Copyright(c) 2016        Intel Deutschland GmbH
 * Copyright (C) 2019 Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name Intel Corporation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/
#include <linux/types.h>
#include <linux/export.h>
#include <linux/slab.h>
#include <linux/firmware.h>
#include "iwl-dbg-cfg.h"
#include "iwl-modparams.h"

/* grab default values */
#undef CPTCFG_IWLWIFI_SUPPORT_DEBUG_OVERRIDES
#include "fw/runtime.h"
#if IS_ENABLED(CPTCFG_IWLXVT)
#include "xvt/constants.h"
#endif
#if IS_ENABLED(CPTCFG_IWLMVM)
#include "mvm/constants.h"
#endif

struct iwl_dbg_cfg current_dbg_config = {
#define DBG_CFG_REINCLUDE
#define IWL_DBG_CFG(type, name) \
	.name = IWL_ ## name,
#define IWL_DBG_CFG_STR(name) /* no default */
#define IWL_DBG_CFG_NODEF(type, name) /* no default */
#define IWL_DBG_CFG_BIN(name) /* nothing, default empty */
#define IWL_DBG_CFG_BINA(name, max) /* nothing, default empty */
#define IWL_MOD_PARAM(type, name) /* nothing, default empty */
#define IWL_MVM_MOD_PARAM(type, name) /* nothing, default empty */
#define IWL_DBG_CFG_RANGE(type, name, min, max)	\
	.name = IWL_ ## name,
#include "iwl-dbg-cfg.h"
#undef IWL_DBG_CFG
#undef IWL_DBG_CFG_STR
#undef IWL_DBG_CFG_NODEF
#undef IWL_DBG_CFG_BIN
#undef IWL_DBG_CFG_BINA
#undef IWL_DBG_CFG_RANGE
#undef IWL_MOD_PARAM
#undef IWL_MVM_MOD_PARAM
};

static const char dbg_cfg_magic[] = "[IWL DEBUG CONFIG DATA]";

#define DBG_CFG_LOADER(_type)							\
static void __maybe_unused							\
dbg_cfg_load_ ## _type(const char *name, const char *val,			\
		       void *out, s64 min, s64 max)				\
{										\
	_type r;								\
										\
	if (kstrto ## _type(val, 0, &r)) {					\
		printk(KERN_INFO "iwlwifi debug config: Invalid data for %s: %s\n",\
		       name, val);						\
		return;								\
	}									\
										\
	if (min && max && (r < min || r > max)) {				\
		printk(KERN_INFO "iwlwifi debug config: value %u for %s out of range [%lld,%lld]\n",\
		       r, name, min, max);					\
		return;								\
	}									\
										\
	*(_type *)out = r;							\
	printk(KERN_INFO "iwlwifi debug config: %s=%d\n", name, *(_type *)out);	\
}

DBG_CFG_LOADER(u8)
DBG_CFG_LOADER(u16)
DBG_CFG_LOADER(u32)
DBG_CFG_LOADER(int)
DBG_CFG_LOADER(uint)

static void __maybe_unused
dbg_cfg_load_bool(const char *name, const char *val,
		  void *out, s64 min, s64 max)
{
	u8 v;

	if (kstrtou8(val, 0, &v)) {
		printk(KERN_INFO "iwlwifi debug config: Invalid data for %s: %s\n",
		       name, val);
	} else {
		*(bool *)out = v;
		printk(KERN_INFO "iwlwifi debug config: %s=%d\n",
		       name, *(bool *)out);
	}
}

static int __maybe_unused
dbg_cfg_load_bin(const char *name, const char *val, struct iwl_dbg_cfg_bin *out)
{
	int len = strlen(val);
	u8 *data;

	if (len % 2)
		goto error;
	len /= 2;

	data = kzalloc(len, GFP_KERNEL);
	if (!data)
		return -ENOMEM;
	if (hex2bin(data, val, len)) {
		kfree(data);
		goto error;
	}
	out->data = data;
	out->len = len;
	printk(KERN_INFO "iwlwifi debug config: %d bytes for %s\n", len, name);
	return 0;
error:
	printk(KERN_INFO "iwlwifi debug config: Invalid data for %s\n", name);
	return -EINVAL;
}

static __maybe_unused void
dbg_cfg_load_str(const char *name, const char *val, void *out, s64 min, s64 max)
{
	if (strlen(val) == 0) {
		printk(KERN_INFO "iwlwifi debug config: Invalid data for %s\n",
		       name);
	} else {
		*(char **)out = kstrdup(val, GFP_KERNEL);
		printk(KERN_INFO "iwlwifi debug config: %s=%s\n",
		       name, *(char **)out);
	}
}

void iwl_dbg_cfg_free(struct iwl_dbg_cfg *dbgcfg)
{
#define IWL_DBG_CFG(t, n) /* nothing */
#define IWL_DBG_CFG_STR(n)				\
	kfree(dbgcfg->n);
#define IWL_DBG_CFG_NODEF(t, n) /* nothing */
#define IWL_DBG_CFG_BIN(n)				\
	do {						\
		kfree(dbgcfg->n.data);			\
		dbgcfg->n.data = NULL;			\
		dbgcfg->n.len = 0;			\
	} while (0);
#define IWL_DBG_CFG_BINA(n, max)			\
	do {						\
		int i;					\
							\
		for (i = 0; i < max; i++) {		\
			kfree(dbgcfg->n[i].data);	\
			dbgcfg->n[i].data = NULL;	\
			dbgcfg->n[i].len = 0;		\
		}					\
		dbgcfg->n_ ## n = 0;			\
	} while (0);
#define IWL_DBG_CFG_RANGE(t, n, min, max) /* nothing */
#define IWL_MOD_PARAM(t, n) /* nothing */
#define IWL_MVM_MOD_PARAM(t, n) /* nothing */
#include "iwl-dbg-cfg.h"
#undef IWL_DBG_CFG
#undef IWL_DBG_CFG_STR
#undef IWL_DBG_CFG_NODEF
#undef IWL_DBG_CFG_BIN
#undef IWL_DBG_CFG_BINA
#undef IWL_DBG_CFG_RANGE
#undef IWL_MOD_PARAM
#undef IWL_MVM_MOD_PARAM
}

struct iwl_dbg_cfg_loader {
	const char *name;
	s64 min, max;
	void (*loader)(const char *name, const char *val,
		       void *out, s64 min, s64 max);
	u32 offset;
};

static const struct iwl_dbg_cfg_loader iwl_dbg_cfg_loaders[] = {
#define IWL_DBG_CFG(t, n)					\
	{							\
		.name = #n,					\
		.offset = offsetof(struct iwl_dbg_cfg, n),	\
		.loader = dbg_cfg_load_##t,			\
	},
#define IWL_DBG_CFG_STR(n)					\
	{							\
		.name = #n,					\
		.offset = offsetof(struct iwl_dbg_cfg, n),	\
		.loader = dbg_cfg_load_str,			\
	},
#define IWL_DBG_CFG_NODEF(t, n) IWL_DBG_CFG(t, n)
#define IWL_DBG_CFG_BIN(n) /* not using this */
#define IWL_DBG_CFG_BINA(n, max) /* not using this */
#define IWL_DBG_CFG_RANGE(t, n, _min, _max)			\
	{							\
		.name = #n,					\
		.offset = offsetof(struct iwl_dbg_cfg, n),	\
		.min = _min,					\
		.max = _max,					\
		.loader = dbg_cfg_load_##t,			\
	},
#define IWL_MOD_PARAM(t, n) /* no using this */
#define IWL_MVM_MOD_PARAM(t, n) /* no using this */
#include "iwl-dbg-cfg.h"
#undef IWL_DBG_CFG
#undef IWL_DBG_CFG_STR
#undef IWL_DBG_CFG_NODEF
#undef IWL_DBG_CFG_BIN
#undef IWL_DBG_CFG_BINA
#undef IWL_DBG_CFG_RANGE
#undef IWL_MOD_PARAM
#undef IWL_MVM_MOD_PARAM
};

void iwl_dbg_cfg_load_ini(struct device *dev, struct iwl_dbg_cfg *dbgcfg)
{
	const struct firmware *fw;
	char *data, *end, *pos;
	int err;

	if (dbgcfg->loaded)
		return;

	/* TODO: maybe add a per-device file? */
	err = request_firmware(&fw, "iwl-dbg-cfg.ini", dev);
	if (err)
		return;

	/* must be ini file style with magic section header */
	if (fw->size < strlen(dbg_cfg_magic))
		goto release;
	if (memcmp(fw->data, dbg_cfg_magic, strlen(dbg_cfg_magic))) {
		printk(KERN_INFO "iwlwifi debug config: file is malformed\n");
		goto release;
	}

	/* +1 guarantees the last line gets NUL-terminated even without \n */
	data = kzalloc(fw->size - strlen(dbg_cfg_magic) + 1, GFP_KERNEL);
	if (!data)
		goto release;
	memcpy(data, fw->data + strlen(dbg_cfg_magic),
	       fw->size - strlen(dbg_cfg_magic));
	end = data + fw->size - strlen(dbg_cfg_magic);
	/* replace CR/LF with NULs to make parsing easier */
	for (pos = data; pos < end; pos++) {
		if (*pos == '\n' || *pos == '\r')
			*pos = '\0';
	}

	pos = data;
	while (pos < end) {
		const char *line = pos;
		bool loaded = false;
		int idx;

		/* skip to next line */
		while (pos < end && *pos)
			pos++;
		/* skip to start of next line, over empty ones if any */
		while (pos < end && !*pos)
			pos++;

		/* skip empty lines and comments */
		if (!*line || *line == '#')
			continue;

		for (idx = 0; idx < ARRAY_SIZE(iwl_dbg_cfg_loaders); idx++) {
			const struct iwl_dbg_cfg_loader *l;

			l = &iwl_dbg_cfg_loaders[idx];

			if (strncmp(l->name, line, strlen(l->name)) == 0 &&
			    line[strlen(l->name)] == '=') {
				l->loader(l->name, line + strlen(l->name) + 1,
					  (void *)((u8 *)dbgcfg + l->offset),
					  l->min, l->max);
				loaded = true;
			}
		}

		/*
		 * if it was loaded by the loaders, don't bother checking
		 * more or printing an error message below
		 */
		if (loaded)
			continue;

#define IWL_DBG_CFG(t, n) /* handled above */
#define IWL_DBG_CFG_NODEF(t, n) /* handled above */
#define IWL_DBG_CFG_BIN(n)						\
		if (strncmp(#n, line, strlen(#n)) == 0 &&		\
		    line[strlen(#n)] == '=') {				\
			dbg_cfg_load_bin(#n, line + strlen(#n) + 1,	\
					 &dbgcfg->n);			\
			continue;					\
		}
#define IWL_DBG_CFG_BINA(n, max)					\
		if (strncmp(#n, line, strlen(#n)) == 0 &&		\
		    line[strlen(#n)] == '=') {				\
			if (dbgcfg->n_##n >= max) {			\
				printk(KERN_INFO			\
				       "iwlwifi debug config: " #n " given too many times\n");\
				continue;				\
			}						\
			if (!dbg_cfg_load_bin(#n, line + strlen(#n) + 1,\
					      &dbgcfg->n[dbgcfg->n_##n]))\
				dbgcfg->n_##n++;			\
			continue;					\
		}
#define IWL_DBG_CFG_RANGE(t, n, min, max) /* handled above */
#define IWL_DBG_CFG_STR(n) /* handled above */
#define IWL_MOD_PARAM(t, n)						\
		if (strncmp(#n, line, strlen(#n)) == 0 &&		\
		    line[strlen(#n)] == '=') {				\
			dbg_cfg_load_##t(#n, line + strlen(#n) + 1,	\
					 &iwlwifi_mod_params.n, 0, 0);	\
			continue;					\
		}
#define IWL_MVM_MOD_PARAM(t, n)	{					\
		if (strncmp("mvm." #n, line, strlen("mvm." #n)) == 0 &&	\
		    line[strlen("mvm." #n)] == '=') {			\
			dbg_cfg_load_##t("mvm." #n,			\
					 line + strlen("mvm." #n) + 1,	\
					 &dbgcfg->mvm_##n, 0, 0);	\
			dbgcfg->__mvm_mod_param_##n = true;		\
			continue;					\
		}							\
	}
#include "iwl-dbg-cfg.h"
#undef IWL_DBG_CFG
#undef IWL_DBG_CFG_STR
#undef IWL_DBG_CFG_NODEF
#undef IWL_DBG_CFG_BIN
#undef IWL_DBG_CFG_BINA
#undef IWL_DBG_CFG_RANGE
#undef IWL_MOD_PARAM
#undef IWL_MVM_MOD_PARAM
		printk(KERN_INFO "iwlwifi debug config: failed to load line \"%s\"\n",
		       line);
	}

	kfree(data);
 release:
	release_firmware(fw);
	dbgcfg->loaded = true;
}
