/****************************************************************************
 * Driver for Solarflare Solarstorm network controllers and boards
 * Copyright 2005-2006 Fen Systems Ltd.
 * Copyright 2005-2008 Solarflare Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation, incorporated herein by reference.
 */

/* Common definitions for all Efx net driver code */

#ifndef EFX_NET_DRIVER_H
#define EFX_NET_DRIVER_H

#include <linux/version.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/if_vlan.h>
#include <linux/timer.h>
#include <linux/mii.h>
#include <linux/list.h>
#include <linux/pci.h>
#include <linux/device.h>
#include <linux/highmem.h>
#include <linux/workqueue.h>
#include <linux/inet_lro.h>
#include <linux/i2c.h>

#include "enum.h"
#include "bitfield.h"

#define EFX_MAX_LRO_DESCRIPTORS 8
#define EFX_MAX_LRO_AGGR MAX_SKB_FRAGS

/**************************************************************************
 *
 * Build definitions
 *
 **************************************************************************/
#ifndef EFX_DRIVER_NAME
#define EFX_DRIVER_NAME	"sfc"
#endif
#define EFX_DRIVER_VERSION	"2.2"

#ifdef EFX_ENABLE_DEBUG
#define EFX_BUG_ON_PARANOID(x) BUG_ON(x)
#define EFX_WARN_ON_PARANOID(x) WARN_ON(x)
#else
#define EFX_BUG_ON_PARANOID(x) do {} while (0)
#define EFX_WARN_ON_PARANOID(x) do {} while (0)
#endif

/* Un-rate-limited logging */
#define EFX_ERR(efx, fmt, args...) \
dev_err(&((efx)->pci_dev->dev), "ERR: %s " fmt, efx_dev_name(efx), ##args)

#define EFX_INFO(efx, fmt, args...) \
dev_info(&((efx)->pci_dev->dev), "INFO: %s " fmt, efx_dev_name(efx), ##args)

#ifdef EFX_ENABLE_DEBUG
#define EFX_LOG(efx, fmt, args...) \
dev_info(&((efx)->pci_dev->dev), "DBG: %s " fmt, efx_dev_name(efx), ##args)
#else
#define EFX_LOG(efx, fmt, args...) \
dev_dbg(&((efx)->pci_dev->dev), "DBG: %s " fmt, efx_dev_name(efx), ##args)
#endif

#define EFX_TRACE(efx, fmt, args...) do {} while (0)

#define EFX_REGDUMP(efx, fmt, args...) do {} while (0)

/* Rate-limited logging */
#define EFX_ERR_RL(efx, fmt, args...) \
do {if (net_ratelimit()) EFX_ERR(efx, fmt, ##args); } while (0)

#define EFX_INFO_RL(efx, fmt, args...) \
do {if (net_ratelimit()) EFX_INFO(efx, fmt, ##args); } while (0)

#define EFX_LOG_RL(efx, fmt, args...) \
do {if (net_ratelimit()) EFX_LOG(efx, fmt, ##args); } while (0)

/**************************************************************************
 *
 * Efx data structures
 *
 **************************************************************************/

#define EFX_MAX_CHANNELS 32
#define EFX_MAX_TX_QUEUES 1
#define EFX_MAX_RX_QUEUES EFX_MAX_CHANNELS

/**
 * struct efx_special_buffer - An Efx special buffer
 * @addr: CPU base address of the buffer
 * @dma_addr: DMA base address of the buffer
 * @len: Buffer length, in bytes
 * @index: Buffer index within controller;s buffer table
 * @entries: Number of buffer table entries
 *
 * Special buffers are used for the event queues and the TX and RX
 * descriptor queues for each channel.  They are *not* used for the
 * actual transmit and receive buffers.
 *
 * Note that for Falcon, TX and RX descriptor queues live in host memory.
 * Allocation and freeing procedures must take this into account.
 */
struct efx_special_buffer {
	void *addr;
	dma_addr_t dma_addr;
	unsigned int len;
	int index;
	int entries;
};

/**
 * struct efx_tx_buffer - An Efx TX buffer
 * @skb: The associated socket buffer.
 *	Set only on the final fragment of a packet; %NULL for all other
 *	fragments.  When this fragment completes, then we can free this
 *	skb.
 * @tsoh: The associated TSO header structure, or %NULL if this
 *	buffer is not a TSO header.
 * @dma_addr: DMA address of the fragment.
 * @len: Length of this fragment.
 *	This field is zero when the queue slot is empty.
 * @continuation: True if this fragment is not the end of a packet.
 * @unmap_single: True if pci_unmap_single should be used.
 * @unmap_addr: DMA address to unmap
 * @unmap_len: Length of this fragment to unmap
 */
struct efx_tx_buffer {
	const struct sk_buff *skb;
	struct efx_tso_header *tsoh;
	dma_addr_t dma_addr;
	unsigned short len;
	unsigned char continuation;
	unsigned char unmap_single;
	dma_addr_t unmap_addr;
	unsigned short unmap_len;
};

/**
 * struct efx_tx_queue - An Efx TX queue
 *
 * This is a ring buffer of TX fragments.
 * Since the TX completion path always executes on the same
 * CPU and the xmit path can operate on different CPUs,
 * performance is increased by ensuring that the completion
 * path and the xmit path operate on different cache lines.
 * This is particularly important if the xmit path is always
 * executing on one CPU which is different from the completion
 * path.  There is also a cache line for members which are
 * read but not written on the fast path.
 *
 * @efx: The associated Efx NIC
 * @queue: DMA queue number
 * @used: Queue is used by net driver
 * @channel: The associated channel
 * @buffer: The software buffer ring
 * @txd: The hardware descriptor ring
 * @read_count: Current read pointer.
 *	This is the number of buffers that have been removed from both rings.
 * @stopped: Stopped flag.
 *	Set if this TX queue is currently stopping its port.
 * @insert_count: Current insert pointer
 *	This is the number of buffers that have been added to the
 *	software ring.
 * @write_count: Current write pointer
 *	This is the number of buffers that have been added to the
 *	hardware ring.
 * @old_read_count: The value of read_count when last checked.
 *	This is here for performance reasons.  The xmit path will
 *	only get the up-to-date value of read_count if this
 *	variable indicates that the queue is full.  This is to
 *	avoid cache-line ping-pong between the xmit path and the
 *	completion path.
 * @tso_headers_free: A list of TSO headers allocated for this TX queue
 *	that are not in use, and so available for new TSO sends. The list
 *	is protected by the TX queue lock.
 * @tso_bursts: Number of times TSO xmit invoked by kernel
 * @tso_long_headers: Number of packets with headers too long for standard
 *	blocks
 * @tso_packets: Number of packets via the TSO xmit path
 */
struct efx_tx_queue {
	/* Members which don't change on the fast path */
	struct efx_nic *efx ____cacheline_aligned_in_smp;
	int queue;
	int used;
	struct efx_channel *channel;
	struct efx_nic *nic;
	struct efx_tx_buffer *buffer;
	struct efx_special_buffer txd;

	/* Members used mainly on the completion path */
	unsigned int read_count ____cacheline_aligned_in_smp;
	int stopped;

	/* Members used only on the xmit path */
	unsigned int insert_count ____cacheline_aligned_in_smp;
	unsigned int write_count;
	unsigned int old_read_count;
	struct efx_tso_header *tso_headers_free;
	unsigned int tso_bursts;
	unsigned int tso_long_headers;
	unsigned int tso_packets;
};

/**
 * struct efx_rx_buffer - An Efx RX data buffer
 * @dma_addr: DMA base address of the buffer
 * @skb: The associated socket buffer, if any.
 *	If both this and page are %NULL, the buffer slot is currently free.
 * @page: The associated page buffer, if any.
 *	If both this and skb are %NULL, the buffer slot is currently free.
 * @data: Pointer to ethernet header
 * @len: Buffer length, in bytes.
 * @unmap_addr: DMA address to unmap
 */
struct efx_rx_buffer {
	dma_addr_t dma_addr;
	struct sk_buff *skb;
	struct page *page;
	char *data;
	unsigned int len;
	dma_addr_t unmap_addr;
};

/**
 * struct efx_rx_queue - An Efx RX queue
 * @efx: The associated Efx NIC
 * @queue: DMA queue number
 * @used: Queue is used by net driver
 * @channel: The associated channel
 * @buffer: The software buffer ring
 * @rxd: The hardware descriptor ring
 * @added_count: Number of buffers added to the receive queue.
 * @notified_count: Number of buffers given to NIC (<= @added_count).
 * @removed_count: Number of buffers removed from the receive queue.
 * @add_lock: Receive queue descriptor add spin lock.
 *	This lock must be held in order to add buffers to the RX
 *	descriptor ring (rxd and buffer) and to update added_count (but
 *	not removed_count).
 * @max_fill: RX descriptor maximum fill level (<= ring size)
 * @fast_fill_trigger: RX descriptor fill level that will trigger a fast fill
 *	(<= @max_fill)
 * @fast_fill_limit: The level to which a fast fill will fill
 *	(@fast_fill_trigger <= @fast_fill_limit <= @max_fill)
 * @min_fill: RX descriptor minimum non-zero fill level.
 *	This records the minimum fill level observed when a ring
 *	refill was triggered.
 * @min_overfill: RX descriptor minimum overflow fill level.
 *	This records the minimum fill level at which RX queue
 *	overflow was observed.  It should never be set.
 * @alloc_page_count: RX allocation strategy counter.
 * @alloc_skb_count: RX allocation strategy counter.
 * @work: Descriptor push work thread
 * @buf_page: Page for next RX buffer.
 *	We can use a single page for multiple RX buffers. This tracks
 *	the remaining space in the allocation.
 * @buf_dma_addr: Page's DMA address.
 * @buf_data: Page's host address.
 */
struct efx_rx_queue {
	struct efx_nic *efx;
	int queue;
	int used;
	struct efx_channel *channel;
	struct efx_rx_buffer *buffer;
	struct efx_special_buffer rxd;

	int added_count;
	int notified_count;
	int removed_count;
	spinlock_t add_lock;
	unsigned int max_fill;
	unsigned int fast_fill_trigger;
	unsigned int fast_fill_limit;
	unsigned int min_fill;
	unsigned int min_overfill;
	unsigned int alloc_page_count;
	unsigned int alloc_skb_count;
	struct delayed_work work;
	unsigned int slow_fill_count;

	struct page *buf_page;
	dma_addr_t buf_dma_addr;
	char *buf_data;
};

/**
 * struct efx_buffer - An Efx general-purpose buffer
 * @addr: host base address of the buffer
 * @dma_addr: DMA base address of the buffer
 * @len: Buffer length, in bytes
 *
 * Falcon uses these buffers for its interrupt status registers and
 * MAC stats dumps.
 */
struct efx_buffer {
	void *addr;
	dma_addr_t dma_addr;
	unsigned int len;
};


/* Flags for channel->used_flags */
#define EFX_USED_BY_RX 1
#define EFX_USED_BY_TX 2
#define EFX_USED_BY_RX_TX (EFX_USED_BY_RX | EFX_USED_BY_TX)

enum efx_rx_alloc_method {
	RX_ALLOC_METHOD_AUTO = 0,
	RX_ALLOC_METHOD_SKB = 1,
	RX_ALLOC_METHOD_PAGE = 2,
};

/**
 * struct efx_channel - An Efx channel
 *
 * A channel comprises an event queue, at least one TX queue, at least
 * one RX queue, and an associated tasklet for processing the event
 * queue.
 *
 * @efx: Associated Efx NIC
 * @evqnum: Event queue number
 * @channel: Channel instance number
 * @used_flags: Channel is used by net driver
 * @enabled: Channel enabled indicator
 * @irq: IRQ number (MSI and MSI-X only)
 * @has_interrupt: Channel has an interrupt
 * @irq_moderation: IRQ moderation value (in us)
 * @napi_dev: Net device used with NAPI
 * @napi_str: NAPI control structure
 * @reset_work: Scheduled reset work thread
 * @work_pending: Is work pending via NAPI?
 * @eventq: Event queue buffer
 * @eventq_read_ptr: Event queue read pointer
 * @last_eventq_read_ptr: Last event queue read pointer value.
 * @eventq_magic: Event queue magic value for driver-generated test events
 * @lro_mgr: LRO state
 * @rx_alloc_level: Watermark based heuristic counter for pushing descriptors
 *	and diagnostic counters
 * @rx_alloc_push_pages: RX allocation method currently in use for pushing
 *	descriptors
 * @rx_alloc_pop_pages: RX allocation method currently in use for popping
 *	descriptors
 * @n_rx_tobe_disc: Count of RX_TOBE_DISC errors
 * @n_rx_ip_frag_err: Count of RX IP fragment errors
 * @n_rx_ip_hdr_chksum_err: Count of RX IP header checksum errors
 * @n_rx_tcp_udp_chksum_err: Count of RX TCP and UDP checksum errors
 * @n_rx_frm_trunc: Count of RX_FRM_TRUNC errors
 * @n_rx_overlength: Count of RX_OVERLENGTH errors
 * @n_skbuff_leaks: Count of skbuffs leaked due to RX overrun
 */
struct efx_channel {
	struct efx_nic *efx;
	int evqnum;
	int channel;
	int used_flags;
	int enabled;
	int irq;
	unsigned int has_interrupt;
	unsigned int irq_moderation;
	struct net_device *napi_dev;
	struct napi_struct napi_str;
	struct work_struct reset_work;
	int work_pending;
	struct efx_special_buffer eventq;
	unsigned int eventq_read_ptr;
	unsigned int last_eventq_read_ptr;
	unsigned int eventq_magic;

	struct net_lro_mgr lro_mgr;
	int rx_alloc_level;
	int rx_alloc_push_pages;
	int rx_alloc_pop_pages;

	unsigned n_rx_tobe_disc;
	unsigned n_rx_ip_frag_err;
	unsigned n_rx_ip_hdr_chksum_err;
	unsigned n_rx_tcp_udp_chksum_err;
	unsigned n_rx_frm_trunc;
	unsigned n_rx_overlength;
	unsigned n_skbuff_leaks;

	/* Used to pipeline received packets in order to optimise memory
	 * access with prefetches.
	 */
	struct efx_rx_buffer *rx_pkt;
	int rx_pkt_csummed;

};

/**
 * struct efx_blinker - S/W LED blinking context
 * @led_num: LED ID (board-specific meaning)
 * @state: Current state - on or off
 * @resubmit: Timer resubmission flag
 * @timer: Control timer for blinking
 */
struct efx_blinker {
	int led_num;
	int state;
	int resubmit;
	struct timer_list timer;
};


/**
 * struct efx_board - board information
 * @type: Board model type
 * @major: Major rev. ('A', 'B' ...)
 * @minor: Minor rev. (0, 1, ...)
 * @init: Initialisation function
 * @init_leds: Sets up board LEDs
 * @set_fault_led: Turns the fault LED on or off
 * @blink: Starts/stops blinking
 * @fini: Cleanup function
 * @blinker: used to blink LEDs in software
 * @hwmon_client: I2C client for hardware monitor
 * @ioexp_client: I2C client for power/port control
 */
struct efx_board {
	int type;
	int major;
	int minor;
	int (*init) (struct efx_nic *nic);
	/* As the LEDs are typically attached to the PHY, LEDs
	 * have a separate init callback that happens later than
	 * board init. */
	int (*init_leds)(struct efx_nic *efx);
	void (*set_fault_led) (struct efx_nic *efx, int state);
	void (*blink) (struct efx_nic *efx, int start);
	void (*fini) (struct efx_nic *nic);
	struct efx_blinker blinker;
	struct i2c_client *hwmon_client, *ioexp_client;
};

#define STRING_TABLE_LOOKUP(val, member)	\
	member ## _names[val]

enum efx_int_mode {
	/* Be careful if altering to correct macro below */
	EFX_INT_MODE_MSIX = 0,
	EFX_INT_MODE_MSI = 1,
	EFX_INT_MODE_LEGACY = 2,
	EFX_INT_MODE_MAX	/* Insert any new items before this */
};
#define EFX_INT_MODE_USE_MSI(x) (((x)->interrupt_mode) <= EFX_INT_MODE_MSI)

enum phy_type {
	PHY_TYPE_NONE = 0,
	PHY_TYPE_CX4_RTMR = 1,
	PHY_TYPE_1G_ALASKA = 2,
	PHY_TYPE_10XPRESS = 3,
	PHY_TYPE_XFP = 4,
	PHY_TYPE_PM8358 = 6,
	PHY_TYPE_MAX	/* Insert any new items before this */
};

#define PHY_ADDR_INVALID 0xff

enum nic_state {
	STATE_INIT = 0,
	STATE_RUNNING = 1,
	STATE_FINI = 2,
	STATE_RESETTING = 3, /* rtnl_lock always held */
	STATE_DISABLED = 4,
	STATE_MAX,
};

/*
 * Alignment of page-allocated RX buffers
 *
 * Controls the number of bytes inserted at the start of an RX buffer.
 * This is the equivalent of NET_IP_ALIGN [which controls the alignment
 * of the skb->head for hardware DMA].
 */
#if defined(__i386__) || defined(__x86_64__)
#define EFX_PAGE_IP_ALIGN 0
#else
#define EFX_PAGE_IP_ALIGN NET_IP_ALIGN
#endif

/*
 * Alignment of the skb->head which wraps a page-allocated RX buffer
 *
 * The skb allocated to wrap an rx_buffer can have this alignment. Since
 * the data is memcpy'd from the rx_buf, it does not need to be equal to
 * EFX_PAGE_IP_ALIGN.
 */
#define EFX_PAGE_SKB_ALIGN 2

/* Forward declaration */
struct efx_nic;

/* Pseudo bit-mask flow control field */
enum efx_fc_type {
	EFX_FC_RX = 1,
	EFX_FC_TX = 2,
	EFX_FC_AUTO = 4,
};

/**
 * struct efx_phy_operations - Efx PHY operations table
 * @init: Initialise PHY
 * @fini: Shut down PHY
 * @reconfigure: Reconfigure PHY (e.g. for new link parameters)
 * @clear_interrupt: Clear down interrupt
 * @blink: Blink LEDs
 * @check_hw: Check hardware
 * @reset_xaui: Reset XAUI side of PHY for (software sequenced reset)
 * @mmds: MMD presence mask
 * @loopbacks: Supported loopback modes mask
 */
struct efx_phy_operations {
	int (*init) (struct efx_nic *efx);
	void (*fini) (struct efx_nic *efx);
	void (*reconfigure) (struct efx_nic *efx);
	void (*clear_interrupt) (struct efx_nic *efx);
	int (*check_hw) (struct efx_nic *efx);
	void (*reset_xaui) (struct efx_nic *efx);
	int mmds;
	unsigned loopbacks;
};

/*
 * Efx extended statistics
 *
 * Not all statistics are provided by all supported MACs.  The purpose
 * is this structure is to contain the raw statistics provided by each
 * MAC.
 */
struct efx_mac_stats {
	u64 tx_bytes;
	u64 tx_good_bytes;
	u64 tx_bad_bytes;
	unsigned long tx_packets;
	unsigned long tx_bad;
	unsigned long tx_pause;
	unsigned long tx_control;
	unsigned long tx_unicast;
	unsigned long tx_multicast;
	unsigned long tx_broadcast;
	unsigned long tx_lt64;
	unsigned long tx_64;
	unsigned long tx_65_to_127;
	unsigned long tx_128_to_255;
	unsigned long tx_256_to_511;
	unsigned long tx_512_to_1023;
	unsigned long tx_1024_to_15xx;
	unsigned long tx_15xx_to_jumbo;
	unsigned long tx_gtjumbo;
	unsigned long tx_collision;
	unsigned long tx_single_collision;
	unsigned long tx_multiple_collision;
	unsigned long tx_excessive_collision;
	unsigned long tx_deferred;
	unsigned long tx_late_collision;
	unsigned long tx_excessive_deferred;
	unsigned long tx_non_tcpudp;
	unsigned long tx_mac_src_error;
	unsigned long tx_ip_src_error;
	u64 rx_bytes;
	u64 rx_good_bytes;
	u64 rx_bad_bytes;
	unsigned long rx_packets;
	unsigned long rx_good;
	unsigned long rx_bad;
	unsigned long rx_pause;
	unsigned long rx_control;
	unsigned long rx_unicast;
	unsigned long rx_multicast;
	unsigned long rx_broadcast;
	unsigned long rx_lt64;
	unsigned long rx_64;
	unsigned long rx_65_to_127;
	unsigned long rx_128_to_255;
	unsigned long rx_256_to_511;
	unsigned long rx_512_to_1023;
	unsigned long rx_1024_to_15xx;
	unsigned long rx_15xx_to_jumbo;
	unsigned long rx_gtjumbo;
	unsigned long rx_bad_lt64;
	unsigned long rx_bad_64_to_15xx;
	unsigned long rx_bad_15xx_to_jumbo;
	unsigned long rx_bad_gtjumbo;
	unsigned long rx_overflow;
	unsigned long rx_missed;
	unsigned long rx_false_carrier;
	unsigned long rx_symbol_error;
	unsigned long rx_align_error;
	unsigned long rx_length_error;
	unsigned long rx_internal_error;
	unsigned long rx_good_lt64;
};

/* Number of bits used in a multicast filter hash address */
#define EFX_MCAST_HASH_BITS 8

/* Number of (single-bit) entries in a multicast filter hash */
#define EFX_MCAST_HASH_ENTRIES (1 << EFX_MCAST_HASH_BITS)

/* An Efx multicast filter hash */
union efx_multicast_hash {
	u8 byte[EFX_MCAST_HASH_ENTRIES / 8];
	efx_oword_t oword[EFX_MCAST_HASH_ENTRIES / sizeof(efx_oword_t) / 8];
};

/**
 * struct efx_nic - an Efx NIC
 * @name: Device name (net device name or bus id before net device registered)
 * @pci_dev: The PCI device
 * @type: Controller type attributes
 * @legacy_irq: IRQ number
 * @workqueue: Workqueue for port reconfigures and the HW monitor.
 *	Work items do not hold and must not acquire RTNL.
 * @reset_workqueue: Workqueue for resets.  Work item will acquire RTNL.
 * @reset_work: Scheduled reset workitem
 * @monitor_work: Hardware monitor workitem
 * @membase_phys: Memory BAR value as physical address
 * @membase: Memory BAR value
 * @biu_lock: BIU (bus interface unit) lock
 * @interrupt_mode: Interrupt mode
 * @i2c_adap: I2C adapter
 * @board_info: Board-level information
 * @state: Device state flag. Serialised by the rtnl_lock.
 * @reset_pending: Pending reset method (normally RESET_TYPE_NONE)
 * @tx_queue: TX DMA queues
 * @rx_queue: RX DMA queues
 * @channel: Channels
 * @rss_queues: Number of RSS queues
 * @rx_buffer_len: RX buffer length
 * @rx_buffer_order: Order (log2) of number of pages for each RX buffer
 * @irq_status: Interrupt status buffer
 * @last_irq_cpu: Last CPU to handle interrupt.
 *	This register is written with the SMP processor ID whenever an
 *	interrupt is handled.  It is used by falcon_test_interrupt()
 *	to verify that an interrupt has occurred.
 * @n_rx_nodesc_drop_cnt: RX no descriptor drop count
 * @nic_data: Hardware dependant state
 * @mac_lock: MAC access lock. Protects @port_enabled, efx_monitor() and
 *	efx_reconfigure_port()
 * @port_enabled: Port enabled indicator.
 *	Serialises efx_stop_all(), efx_start_all() and efx_monitor() and
 *	efx_reconfigure_work with kernel interfaces. Safe to read under any
 *	one of the rtnl_lock, mac_lock, or netif_tx_lock, but all three must
 *	be held to modify it.
 * @port_initialized: Port initialized?
 * @net_dev: Operating system network device. Consider holding the rtnl lock
 * @rx_checksum_enabled: RX checksumming enabled
 * @netif_stop_count: Port stop count
 * @netif_stop_lock: Port stop lock
 * @mac_stats: MAC statistics. These include all statistics the MACs
 *	can provide.  Generic code converts these into a standard
 *	&struct net_device_stats.
 * @stats_buffer: DMA buffer for statistics
 * @stats_lock: Statistics update lock
 * @mac_address: Permanent MAC address
 * @phy_type: PHY type
 * @phy_lock: PHY access lock
 * @phy_op: PHY interface
 * @phy_data: PHY private data (including PHY-specific stats)
 * @mii: PHY interface
 * @tx_disabled: PHY transmitter turned off
 * @link_up: Link status
 * @link_options: Link options (MII/GMII format)
 * @n_link_state_changes: Number of times the link has changed state
 * @promiscuous: Promiscuous flag. Protected by netif_tx_lock.
 * @multicast_hash: Multicast hash table
 * @flow_control: Flow control flags - separate RX/TX so can't use link_options
 * @reconfigure_work: work item for dealing with PHY events
 * @loopback_mode: Loopback status
 * @loopback_modes: Supported loopback mode bitmask
 * @loopback_selftest: Offline self-test private state
 *
 * The @priv field of the corresponding &struct net_device points to
 * this.
 */
struct efx_nic {
	char name[IFNAMSIZ];
	struct pci_dev *pci_dev;
	const struct efx_nic_type *type;
	int legacy_irq;
	struct workqueue_struct *workqueue;
	struct workqueue_struct *reset_workqueue;
	struct work_struct reset_work;
	struct delayed_work monitor_work;
	resource_size_t membase_phys;
	void __iomem *membase;
	spinlock_t biu_lock;
	enum efx_int_mode interrupt_mode;

	struct i2c_adapter i2c_adap;
	struct efx_board board_info;

	enum nic_state state;
	enum reset_type reset_pending;

	struct efx_tx_queue tx_queue[EFX_MAX_TX_QUEUES];
	struct efx_rx_queue rx_queue[EFX_MAX_RX_QUEUES];
	struct efx_channel channel[EFX_MAX_CHANNELS];

	int rss_queues;
	unsigned int rx_buffer_len;
	unsigned int rx_buffer_order;

	struct efx_buffer irq_status;
	volatile signed int last_irq_cpu;

	unsigned n_rx_nodesc_drop_cnt;

	struct falcon_nic_data *nic_data;

	struct mutex mac_lock;
	int port_enabled;

	int port_initialized;
	struct net_device *net_dev;
	int rx_checksum_enabled;

	atomic_t netif_stop_count;
	spinlock_t netif_stop_lock;

	struct efx_mac_stats mac_stats;
	struct efx_buffer stats_buffer;
	spinlock_t stats_lock;

	unsigned char mac_address[ETH_ALEN];

	enum phy_type phy_type;
	spinlock_t phy_lock;
	struct efx_phy_operations *phy_op;
	void *phy_data;
	struct mii_if_info mii;
	unsigned tx_disabled;

	int link_up;
	unsigned int link_options;
	unsigned int n_link_state_changes;

	int promiscuous;
	union efx_multicast_hash multicast_hash;
	enum efx_fc_type flow_control;
	struct work_struct reconfigure_work;

	atomic_t rx_reset;
	enum efx_loopback_mode loopback_mode;
	unsigned int loopback_modes;

	void *loopback_selftest;
};

static inline int efx_dev_registered(struct efx_nic *efx)
{
	return efx->net_dev->reg_state == NETREG_REGISTERED;
}

/* Net device name, for inclusion in log messages if it has been registered.
 * Use efx->name not efx->net_dev->name so that races with (un)registration
 * are harmless.
 */
static inline const char *efx_dev_name(struct efx_nic *efx)
{
	return efx_dev_registered(efx) ? efx->name : "";
}

/**
 * struct efx_nic_type - Efx device type definition
 * @mem_bar: Memory BAR number
 * @mem_map_size: Memory BAR mapped size
 * @txd_ptr_tbl_base: TX descriptor ring base address
 * @rxd_ptr_tbl_base: RX descriptor ring base address
 * @buf_tbl_base: Buffer table base address
 * @evq_ptr_tbl_base: Event queue pointer table base address
 * @evq_rptr_tbl_base: Event queue read-pointer table base address
 * @txd_ring_mask: TX descriptor ring size - 1 (must be a power of two - 1)
 * @rxd_ring_mask: RX descriptor ring size - 1 (must be a power of two - 1)
 * @evq_size: Event queue size (must be a power of two)
 * @max_dma_mask: Maximum possible DMA mask
 * @tx_dma_mask: TX DMA mask
 * @bug5391_mask: Address mask for bug 5391 workaround
 * @rx_xoff_thresh: RX FIFO XOFF watermark (bytes)
 * @rx_xon_thresh: RX FIFO XON watermark (bytes)
 * @rx_buffer_padding: Padding added to each RX buffer
 * @max_interrupt_mode: Highest capability interrupt mode supported
 *	from &enum efx_init_mode.
 * @phys_addr_channels: Number of channels with physically addressed
 *	descriptors
 */
struct efx_nic_type {
	unsigned int mem_bar;
	unsigned int mem_map_size;
	unsigned int txd_ptr_tbl_base;
	unsigned int rxd_ptr_tbl_base;
	unsigned int buf_tbl_base;
	unsigned int evq_ptr_tbl_base;
	unsigned int evq_rptr_tbl_base;

	unsigned int txd_ring_mask;
	unsigned int rxd_ring_mask;
	unsigned int evq_size;
	u64 max_dma_mask;
	unsigned int tx_dma_mask;
	unsigned bug5391_mask;

	int rx_xoff_thresh;
	int rx_xon_thresh;
	unsigned int rx_buffer_padding;
	unsigned int max_interrupt_mode;
	unsigned int phys_addr_channels;
};

/**************************************************************************
 *
 * Prototypes and inline functions
 *
 *************************************************************************/

/* Iterate over all used channels */
#define efx_for_each_channel(_channel, _efx)				\
	for (_channel = &_efx->channel[0];				\
	     _channel < &_efx->channel[EFX_MAX_CHANNELS];		\
	     _channel++)						\
		if (!_channel->used_flags)				\
			continue;					\
		else

/* Iterate over all used channels with interrupts */
#define efx_for_each_channel_with_interrupt(_channel, _efx)		\
	for (_channel = &_efx->channel[0];				\
	     _channel < &_efx->channel[EFX_MAX_CHANNELS];		\
	     _channel++)						\
		if (!(_channel->used_flags && _channel->has_interrupt))	\
			continue;					\
		else

/* Iterate over all used TX queues */
#define efx_for_each_tx_queue(_tx_queue, _efx)				\
	for (_tx_queue = &_efx->tx_queue[0];				\
	     _tx_queue < &_efx->tx_queue[EFX_MAX_TX_QUEUES];		\
	     _tx_queue++)						\
		if (!_tx_queue->used)					\
			continue;					\
		else

/* Iterate over all TX queues belonging to a channel */
#define efx_for_each_channel_tx_queue(_tx_queue, _channel)		\
	for (_tx_queue = &_channel->efx->tx_queue[0];			\
	     _tx_queue < &_channel->efx->tx_queue[EFX_MAX_TX_QUEUES];	\
	     _tx_queue++)						\
		if ((!_tx_queue->used) ||				\
		    (_tx_queue->channel != _channel))			\
			continue;					\
		else

/* Iterate over all used RX queues */
#define efx_for_each_rx_queue(_rx_queue, _efx)				\
	for (_rx_queue = &_efx->rx_queue[0];				\
	     _rx_queue < &_efx->rx_queue[EFX_MAX_RX_QUEUES];		\
	     _rx_queue++)						\
		if (!_rx_queue->used)					\
			continue;					\
		else

/* Iterate over all RX queues belonging to a channel */
#define efx_for_each_channel_rx_queue(_rx_queue, _channel)		\
	for (_rx_queue = &_channel->efx->rx_queue[0];			\
	     _rx_queue < &_channel->efx->rx_queue[EFX_MAX_RX_QUEUES];	\
	     _rx_queue++)						\
		if ((!_rx_queue->used) ||				\
		    (_rx_queue->channel != _channel))			\
			continue;					\
		else

/* Returns a pointer to the specified receive buffer in the RX
 * descriptor queue.
 */
static inline struct efx_rx_buffer *efx_rx_buffer(struct efx_rx_queue *rx_queue,
						  unsigned int index)
{
	return (&rx_queue->buffer[index]);
}

/* Set bit in a little-endian bitfield */
static inline void set_bit_le(int nr, unsigned char *addr)
{
	addr[nr / 8] |= (1 << (nr % 8));
}

/* Clear bit in a little-endian bitfield */
static inline void clear_bit_le(int nr, unsigned char *addr)
{
	addr[nr / 8] &= ~(1 << (nr % 8));
}


/**
 * EFX_MAX_FRAME_LEN - calculate maximum frame length
 *
 * This calculates the maximum frame length that will be used for a
 * given MTU.  The frame length will be equal to the MTU plus a
 * constant amount of header space and padding.  This is the quantity
 * that the net driver will program into the MAC as the maximum frame
 * length.
 *
 * The 10G MAC used in Falcon requires 8-byte alignment on the frame
 * length, so we round up to the nearest 8.
 */
#define EFX_MAX_FRAME_LEN(mtu) \
	((((mtu) + ETH_HLEN + VLAN_HLEN + 4/* FCS */) + 7) & ~7)


#endif /* EFX_NET_DRIVER_H */
