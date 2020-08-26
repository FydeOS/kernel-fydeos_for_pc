#ifndef __MAC80211_EXP_H
#define __MAC80211_EXP_H
#define ieee80211_stop_rx_ba_session __iwl7000_ieee80211_stop_rx_ba_session
#define ieee80211_manage_rx_ba_offl __iwl7000_ieee80211_manage_rx_ba_offl
#define ieee80211_rx_ba_timer_expired __iwl7000_ieee80211_rx_ba_timer_expired
#define ieee80211_send_bar __iwl7000_ieee80211_send_bar
#define ieee80211_start_tx_ba_session __iwl7000_ieee80211_start_tx_ba_session
#define ieee80211_start_tx_ba_cb_irqsafe __iwl7000_ieee80211_start_tx_ba_cb_irqsafe
#define ieee80211_stop_tx_ba_session __iwl7000_ieee80211_stop_tx_ba_session
#define ieee80211_stop_tx_ba_cb_irqsafe __iwl7000_ieee80211_stop_tx_ba_cb_irqsafe
#define arc4_setkey __iwl7000_arc4_setkey
#define arc4_crypt __iwl7000_arc4_crypt
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,7,0)
#define dev_coredumpsg __iwl7000_dev_coredumpsg
#endif /* < 4.7.0 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
#define kstrtobool __iwl7000_kstrtobool
#define kstrtobool_from_user __iwl7000_kstrtobool_from_user
#endif /* < 4.6.0 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0)
#ifdef CONFIG_DEBUG_FS
#define iwl_debugfs_create_bool __iwl7000_iwl_debugfs_create_bool
#endif /* CONFIG_DEBUG_FS */
#define tso_count_descs __iwl7000_tso_count_descs
#define tso_build_hdr __iwl7000_tso_build_hdr
#define tso_build_data __iwl7000_tso_build_data
#define tso_start __iwl7000_tso_start
#define match_string __iwl7000_match_string
#endif /* < 4.4.0 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 19, 0)
#define netdev_rss_key_fill __iwl7000_netdev_rss_key_fill
#endif /* < 3.19.0 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 17, 0)
#define devm_kvasprintf __iwl7000_devm_kvasprintf
#define devm_kasprintf __iwl7000_devm_kasprintf
#endif /* < 3.17 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 13, 0)
#define __backport_genl_register_family __iwl7000___backport_genl_register_family
#define backport_genl_unregister_family __iwl7000_backport_genl_unregister_family
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 11, 0)
#define sg_pcopy_from_buffer __iwl7000_sg_pcopy_from_buffer
#define sg_pcopy_to_buffer __iwl7000_sg_pcopy_to_buffer
#endif /* < 3.11 */
#define __alloc_bucket_spinlocks __iwl7000___alloc_bucket_spinlocks
#define free_bucket_spinlocks __iwl7000_free_bucket_spinlocks
#if CFG80211_VERSION < KERNEL_VERSION(4,1,0)
#define ieee80211_ie_split_ric __iwl7000_ieee80211_ie_split_ric
#define ieee80211_ie_split __iwl7000_ieee80211_ie_split
#endif
#if CFG80211_VERSION < KERNEL_VERSION(4,20,0)
#define ieee80211_get_vht_max_nss __iwl7000_ieee80211_get_vht_max_nss
#endif
#define ieee80211_csa_finish __iwl7000_ieee80211_csa_finish
#define ieee80211_nan_func_terminated __iwl7000_ieee80211_nan_func_terminated
#define ieee80211_nan_func_match __iwl7000_ieee80211_nan_func_match
#define ieee80211_iter_chan_contexts_atomic __iwl7000_ieee80211_iter_chan_contexts_atomic
#define ieee80211_request_smps __iwl7000_ieee80211_request_smps
#define ieee80211_iter_keys __iwl7000_ieee80211_iter_keys
#define ieee80211_iter_keys_rcu __iwl7000_ieee80211_iter_keys_rcu
#define ieee80211_gtk_rekey_notify __iwl7000_ieee80211_gtk_rekey_notify
#define ieee80211_get_key_rx_seq __iwl7000_ieee80211_get_key_rx_seq
#define ieee80211_set_key_rx_seq __iwl7000_ieee80211_set_key_rx_seq
#define ieee80211_remove_key __iwl7000_ieee80211_remove_key
#define ieee80211_gtk_rekey_add __iwl7000_ieee80211_gtk_rekey_add
#define __ieee80211_get_radio_led_name __iwl7000___ieee80211_get_radio_led_name
#define __ieee80211_get_assoc_led_name __iwl7000___ieee80211_get_assoc_led_name
#define __ieee80211_get_tx_led_name __iwl7000___ieee80211_get_tx_led_name
#define __ieee80211_get_rx_led_name __iwl7000___ieee80211_get_rx_led_name
#define __ieee80211_create_tpt_led_trigger __iwl7000___ieee80211_create_tpt_led_trigger
#define ieee80211_restart_hw __iwl7000_ieee80211_restart_hw
#define ieee80211_alloc_hw_nm __iwl7000_ieee80211_alloc_hw_nm
#define ieee80211_register_hw __iwl7000_ieee80211_register_hw
#define ieee80211_unregister_hw __iwl7000_ieee80211_unregister_hw
#define ieee80211_free_hw __iwl7000_ieee80211_free_hw
#define ieee80211_chswitch_done __iwl7000_ieee80211_chswitch_done
#define ieee80211_ap_probereq_get __iwl7000_ieee80211_ap_probereq_get
#define ieee80211_beacon_loss __iwl7000_ieee80211_beacon_loss
#define ieee80211_connection_loss __iwl7000_ieee80211_connection_loss
#define ieee80211_cqm_rssi_notify __iwl7000_ieee80211_cqm_rssi_notify
#define ieee80211_cqm_beacon_loss_notify __iwl7000_ieee80211_cqm_beacon_loss_notify
#define ieee80211_ready_on_channel __iwl7000_ieee80211_ready_on_channel
#define ieee80211_remain_on_channel_expired __iwl7000_ieee80211_remain_on_channel_expired
#define ieee80211_report_wowlan_wakeup __iwl7000_ieee80211_report_wowlan_wakeup
#define ieee80211_rate_control_register __iwl7000_ieee80211_rate_control_register
#define ieee80211_rate_control_unregister __iwl7000_ieee80211_rate_control_unregister
#define ieee80211_get_tx_rates __iwl7000_ieee80211_get_tx_rates
#define rate_control_set_rates __iwl7000_rate_control_set_rates
#if CFG80211_VERSION < KERNEL_VERSION(4,0,0)
#define regulatory_set_wiphy_regd __iwl7000_regulatory_set_wiphy_regd
#define regulatory_set_wiphy_regd_sync_rtnl __iwl7000_regulatory_set_wiphy_regd_sync_rtnl
#endif /* CFG80211_VERSION < KERNEL_VERSION(4,0,0) */
#ifdef CONFIG_PROVE_LOCKING
#define lockdep_rht_mutex_is_held __iwl7000_lockdep_rht_mutex_is_held
#define lockdep_rht_bucket_is_held __iwl7000_lockdep_rht_bucket_is_held
#endif
#define rhashtable_insert_slow __iwl7000_rhashtable_insert_slow
#define rhashtable_walk_enter __iwl7000_rhashtable_walk_enter
#define rhashtable_walk_exit __iwl7000_rhashtable_walk_exit
#define rhashtable_walk_start_check __iwl7000_rhashtable_walk_start_check
#define rhashtable_walk_next __iwl7000_rhashtable_walk_next
#define rhashtable_walk_peek __iwl7000_rhashtable_walk_peek
#define rhashtable_walk_stop __iwl7000_rhashtable_walk_stop
#define rhashtable_init __iwl7000_rhashtable_init
#define rhltable_init __iwl7000_rhltable_init
#define rhashtable_free_and_destroy __iwl7000_rhashtable_free_and_destroy
#define rhashtable_destroy __iwl7000_rhashtable_destroy
#define __rht_bucket_nested __iwl7000___rht_bucket_nested
#define rht_bucket_nested __iwl7000_rht_bucket_nested
#define rht_bucket_nested_insert __iwl7000_rht_bucket_nested_insert
#define ieee80211_sta_ps_transition __iwl7000_ieee80211_sta_ps_transition
#define ieee80211_sta_pspoll __iwl7000_ieee80211_sta_pspoll
#define ieee80211_sta_uapsd_trigger __iwl7000_ieee80211_sta_uapsd_trigger
#define ieee80211_mark_rx_ba_filtered_frames __iwl7000_ieee80211_mark_rx_ba_filtered_frames
#define ieee80211_rx_napi __iwl7000_ieee80211_rx_napi
#define ieee80211_rx_irqsafe __iwl7000_ieee80211_rx_irqsafe
#define ieee80211_scan_completed __iwl7000_ieee80211_scan_completed
#define ieee80211_sched_scan_results __iwl7000_ieee80211_sched_scan_results
#define ieee80211_sched_scan_stopped __iwl7000_ieee80211_sched_scan_stopped
#define ieee80211_find_sta_by_ifaddr __iwl7000_ieee80211_find_sta_by_ifaddr
#define ieee80211_find_sta __iwl7000_ieee80211_find_sta
#define ieee80211_sta_block_awake __iwl7000_ieee80211_sta_block_awake
#define ieee80211_sta_eosp __iwl7000_ieee80211_sta_eosp
#define ieee80211_send_eosp_nullfunc __iwl7000_ieee80211_send_eosp_nullfunc
#define ieee80211_sta_set_buffered __iwl7000_ieee80211_sta_set_buffered
#define ieee80211_sta_register_airtime __iwl7000_ieee80211_sta_register_airtime
#define ieee80211_tx_status_irqsafe __iwl7000_ieee80211_tx_status_irqsafe
#define ieee80211_tx_status __iwl7000_ieee80211_tx_status
#define ieee80211_tx_status_ext __iwl7000_ieee80211_tx_status_ext
#define ieee80211_tx_rate_update __iwl7000_ieee80211_tx_rate_update
#define ieee80211_report_low_ack __iwl7000_ieee80211_report_low_ack
#define ieee80211_free_txskb __iwl7000_ieee80211_free_txskb
#define ieee80211_tdls_oper_request __iwl7000_ieee80211_tdls_oper_request
#define ieee80211_tkip_add_iv __iwl7000_ieee80211_tkip_add_iv
#define ieee80211_get_tkip_p1k_iv __iwl7000_ieee80211_get_tkip_p1k_iv
#define ieee80211_get_tkip_rx_p1k __iwl7000_ieee80211_get_tkip_rx_p1k
#define ieee80211_get_tkip_p2k __iwl7000_ieee80211_get_tkip_p2k
#define ieee80211_tx_prepare_skb __iwl7000_ieee80211_tx_prepare_skb
#define ieee80211_tx_dequeue __iwl7000_ieee80211_tx_dequeue
#define ieee80211_next_txq __iwl7000_ieee80211_next_txq
#define __ieee80211_schedule_txq __iwl7000___ieee80211_schedule_txq
#define ieee80211_txq_may_transmit __iwl7000_ieee80211_txq_may_transmit
#define ieee80211_txq_schedule_start __iwl7000_ieee80211_txq_schedule_start
#define ieee80211_csa_update_counter __iwl7000_ieee80211_csa_update_counter
#define ieee80211_csa_set_counter __iwl7000_ieee80211_csa_set_counter
#define ieee80211_csa_is_complete __iwl7000_ieee80211_csa_is_complete
#define ieee80211_beacon_get_template __iwl7000_ieee80211_beacon_get_template
#define ieee80211_beacon_get_tim __iwl7000_ieee80211_beacon_get_tim
#define ieee80211_proberesp_get __iwl7000_ieee80211_proberesp_get
#define ieee80211_pspoll_get __iwl7000_ieee80211_pspoll_get
#define ieee80211_nullfunc_get __iwl7000_ieee80211_nullfunc_get
#define ieee80211_probereq_get __iwl7000_ieee80211_probereq_get
#define ieee80211_rts_get __iwl7000_ieee80211_rts_get
#define ieee80211_ctstoself_get __iwl7000_ieee80211_ctstoself_get
#define ieee80211_get_buffered_bc __iwl7000_ieee80211_get_buffered_bc
#define ieee80211_reserve_tid __iwl7000_ieee80211_reserve_tid
#define ieee80211_unreserve_tid __iwl7000_ieee80211_unreserve_tid
#define wiphy_to_ieee80211_hw __iwl7000_wiphy_to_ieee80211_hw
#define ieee80211_generic_frame_duration __iwl7000_ieee80211_generic_frame_duration
#define ieee80211_rts_duration __iwl7000_ieee80211_rts_duration
#define ieee80211_ctstoself_duration __iwl7000_ieee80211_ctstoself_duration
#define ieee80211_wake_queue __iwl7000_ieee80211_wake_queue
#define ieee80211_stop_queue __iwl7000_ieee80211_stop_queue
#define ieee80211_stop_queues __iwl7000_ieee80211_stop_queues
#define ieee80211_queue_stopped __iwl7000_ieee80211_queue_stopped
#define ieee80211_wake_queues __iwl7000_ieee80211_wake_queues
#define ieee80211_iterate_interfaces __iwl7000_ieee80211_iterate_interfaces
#define ieee80211_iterate_active_interfaces_atomic __iwl7000_ieee80211_iterate_active_interfaces_atomic
#define ieee80211_iterate_active_interfaces_rtnl __iwl7000_ieee80211_iterate_active_interfaces_rtnl
#define ieee80211_iterate_stations_atomic __iwl7000_ieee80211_iterate_stations_atomic
#define wdev_to_ieee80211_vif __iwl7000_wdev_to_ieee80211_vif
#define ieee80211_vif_to_wdev __iwl7000_ieee80211_vif_to_wdev
#define ieee80211_queue_work __iwl7000_ieee80211_queue_work
#define ieee80211_queue_delayed_work __iwl7000_ieee80211_queue_delayed_work
#define ieee80211_resume_disconnect __iwl7000_ieee80211_resume_disconnect
#define ieee80211_enable_rssi_reports __iwl7000_ieee80211_enable_rssi_reports
#define ieee80211_disable_rssi_reports __iwl7000_ieee80211_disable_rssi_reports
#define ieee80211_ave_rssi __iwl7000_ieee80211_ave_rssi
#define ieee80211_radar_detected __iwl7000_ieee80211_radar_detected
#define ieee80211_update_p2p_noa __iwl7000_ieee80211_update_p2p_noa
#define ieee80211_parse_p2p_noa __iwl7000_ieee80211_parse_p2p_noa
#define ieee80211_txq_get_depth __iwl7000_ieee80211_txq_get_depth
#define ieee80211_update_mu_groups __iwl7000_ieee80211_update_mu_groups
#endif
