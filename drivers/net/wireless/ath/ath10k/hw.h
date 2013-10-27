/*
 * Copyright (c) 2005-2011 Atheros Communications Inc.
 * Copyright (c) 2011-2013 Qualcomm Atheros, Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _HW_H_
#define _HW_H_

#include "targaddrs.h"

/* Supported FW version */
#define SUPPORTED_FW_MAJOR	1
#define SUPPORTED_FW_MINOR	0
#define SUPPORTED_FW_RELEASE	0
#define SUPPORTED_FW_BUILD	636

/* QCA988X 1.0 definitions (unsupported) */
#define QCA988X_HW_1_0_CHIP_ID_REV	0x0

/* QCA988X 2.0 definitions */
#define QCA988X_HW_2_0_VERSION		0x4100016c
#define QCA988X_HW_2_0_CHIP_ID_REV	0x2
#define QCA988X_HW_2_0_FW_DIR		"ath10k/QCA988X/hw2.0"
#define QCA988X_HW_2_0_FW_FILE		"firmware.bin"
#define QCA988X_HW_2_0_OTP_FILE		"otp.bin"
#define QCA988X_HW_2_0_BOARD_DATA_FILE	"board.bin"
#define QCA988X_HW_2_0_PATCH_LOAD_ADDR	0x1234

/* Known pecularities:
 *  - current FW doesn't support raw rx mode (last tested v599)
 *  - current FW dumps upon raw tx mode (last tested v599)
 *  - raw appears in nwifi decap, raw and nwifi appear in ethernet decap
 *  - raw have FCS, nwifi doesn't
 *  - ethernet frames have 802.11 header decapped and parts (base hdr, cipher
 *    param, llc/snap) are aligned to 4byte boundaries each */
enum ath10k_hw_txrx_mode {
	ATH10K_HW_TXRX_RAW = 0,
	ATH10K_HW_TXRX_NATIVE_WIFI = 1,
	ATH10K_HW_TXRX_ETHERNET = 2,

	/* Valid for HTT >= 3.0. Used for management frames in TX_FRM. */
	ATH10K_HW_TXRX_MGMT = 3,
};

enum ath10k_mcast2ucast_mode {
	ATH10K_MCAST2UCAST_DISABLED = 0,
	ATH10K_MCAST2UCAST_ENABLED = 1,
};

#define TARGET_NUM_VDEVS			8
#define TARGET_NUM_PEER_AST			2
#define TARGET_NUM_WDS_ENTRIES			32
#define TARGET_DMA_BURST_SIZE			0
#define TARGET_MAC_AGGR_DELIM			0
#define TARGET_AST_SKID_LIMIT			16
#define TARGET_NUM_PEERS			16
#define TARGET_NUM_OFFLOAD_PEERS		0
#define TARGET_NUM_OFFLOAD_REORDER_BUFS         0
#define TARGET_NUM_PEER_KEYS			2
#define TARGET_NUM_TIDS		(2 * ((TARGET_NUM_PEERS) + (TARGET_NUM_VDEVS)))
#define TARGET_TX_CHAIN_MASK			(BIT(0) | BIT(1) | BIT(2))
#define TARGET_RX_CHAIN_MASK			(BIT(0) | BIT(1) | BIT(2))
#define TARGET_RX_TIMEOUT_LO_PRI		100
#define TARGET_RX_TIMEOUT_HI_PRI		40

/* Native Wifi decap mode is used to align IP frames to 4-byte boundaries and
 * avoid a very expensive re-alignment in mac80211. */
#define TARGET_RX_DECAP_MODE			ATH10K_HW_TXRX_NATIVE_WIFI

#define TARGET_SCAN_MAX_PENDING_REQS		4
#define TARGET_BMISS_OFFLOAD_MAX_VDEV		3
#define TARGET_ROAM_OFFLOAD_MAX_VDEV		3
#define TARGET_ROAM_OFFLOAD_MAX_AP_PROFILES	8
#define TARGET_GTK_OFFLOAD_MAX_VDEV		3
#define TARGET_NUM_MCAST_GROUPS			0
#define TARGET_NUM_MCAST_TABLE_ELEMS		0
#define TARGET_MCAST2UCAST_MODE			ATH10K_MCAST2UCAST_DISABLED
#define TARGET_TX_DBG_LOG_SIZE			1024
#define TARGET_RX_SKIP_DEFRAG_TIMEOUT_DUP_DETECTION_CHECK 0
#define TARGET_VOW_CONFIG			0
#define TARGET_NUM_MSDU_DESC			(1024 + 400)
#define TARGET_MAX_FRAG_ENTRIES			0


/* Number of Copy Engines supported */
#define CE_COUNT 8

/*
 * Total number of PCIe MSI interrupts requested for all interrupt sources.
 * PCIe standard forces this to be a power of 2.
 * Some Host OS's limit MSI requests that can be granted to 8
 * so for now we abide by this limit and avoid requesting more
 * than that.
 */
#define MSI_NUM_REQUEST_LOG2	3
#define MSI_NUM_REQUEST		(1<<MSI_NUM_REQUEST_LOG2)

/*
 * Granted MSIs are assigned as follows:
 * Firmware uses the first
 * Remaining MSIs, if any, are used by Copy Engines
 * This mapping is known to both Target firmware and Host software.
 * It may be changed as long as Host and Target are kept in sync.
 */
/* MSI for firmware (errors, etc.) */
#define MSI_ASSIGN_FW		0

/* MSIs for Copy Engines */
#define MSI_ASSIGN_CE_INITIAL	1
#define MSI_ASSIGN_CE_MAX	7

/* as of IP3.7.1 */
#define RTC_STATE_V_ON				3

#define RTC_STATE_COLD_RESET_MASK		0x00000400
#define RTC_STATE_V_LSB				0
#define RTC_STATE_V_MASK			0x00000007
#define RTC_STATE_ADDRESS			0x0000
#define PCIE_SOC_WAKE_V_MASK			0x00000001
#define PCIE_SOC_WAKE_ADDRESS			0x0004
#define PCIE_SOC_WAKE_RESET			0x00000000
#define SOC_GLOBAL_RESET_ADDRESS		0x0008

#define RTC_SOC_BASE_ADDRESS			0x00004000
#define RTC_WMAC_BASE_ADDRESS			0x00005000
#define MAC_COEX_BASE_ADDRESS			0x00006000
#define BT_COEX_BASE_ADDRESS			0x00007000
#define SOC_PCIE_BASE_ADDRESS			0x00008000
#define SOC_CORE_BASE_ADDRESS			0x00009000
#define WLAN_UART_BASE_ADDRESS			0x0000c000
#define WLAN_SI_BASE_ADDRESS			0x00010000
#define WLAN_GPIO_BASE_ADDRESS			0x00014000
#define WLAN_ANALOG_INTF_BASE_ADDRESS		0x0001c000
#define WLAN_MAC_BASE_ADDRESS			0x00020000
#define EFUSE_BASE_ADDRESS			0x00030000
#define FPGA_REG_BASE_ADDRESS			0x00039000
#define WLAN_UART2_BASE_ADDRESS			0x00054c00
#define CE_WRAPPER_BASE_ADDRESS			0x00057000
#define CE0_BASE_ADDRESS			0x00057400
#define CE1_BASE_ADDRESS			0x00057800
#define CE2_BASE_ADDRESS			0x00057c00
#define CE3_BASE_ADDRESS			0x00058000
#define CE4_BASE_ADDRESS			0x00058400
#define CE5_BASE_ADDRESS			0x00058800
#define CE6_BASE_ADDRESS			0x00058c00
#define CE7_BASE_ADDRESS			0x00059000
#define DBI_BASE_ADDRESS			0x00060000
#define WLAN_ANALOG_INTF_PCIE_BASE_ADDRESS	0x0006c000
#define PCIE_LOCAL_BASE_ADDRESS			0x00080000

#define SOC_RESET_CONTROL_OFFSET		0x00000000
#define SOC_RESET_CONTROL_SI0_RST_MASK		0x00000001
#define SOC_CPU_CLOCK_OFFSET			0x00000020
#define SOC_CPU_CLOCK_STANDARD_LSB		0
#define SOC_CPU_CLOCK_STANDARD_MASK		0x00000003
#define SOC_CLOCK_CONTROL_OFFSET		0x00000028
#define SOC_CLOCK_CONTROL_SI0_CLK_MASK		0x00000001
#define SOC_SYSTEM_SLEEP_OFFSET			0x000000c4
#define SOC_LPO_CAL_OFFSET			0x000000e0
#define SOC_LPO_CAL_ENABLE_LSB			20
#define SOC_LPO_CAL_ENABLE_MASK			0x00100000

#define SOC_CHIP_ID_ADDRESS			0x000000ec
#define SOC_CHIP_ID_REV_LSB			8
#define SOC_CHIP_ID_REV_MASK			0x00000f00

#define WLAN_RESET_CONTROL_COLD_RST_MASK	0x00000008
#define WLAN_RESET_CONTROL_WARM_RST_MASK	0x00000004
#define WLAN_SYSTEM_SLEEP_DISABLE_LSB		0
#define WLAN_SYSTEM_SLEEP_DISABLE_MASK		0x00000001

#define WLAN_GPIO_PIN0_ADDRESS			0x00000028
#define WLAN_GPIO_PIN0_CONFIG_MASK		0x00007800
#define WLAN_GPIO_PIN1_ADDRESS			0x0000002c
#define WLAN_GPIO_PIN1_CONFIG_MASK		0x00007800
#define WLAN_GPIO_PIN10_ADDRESS			0x00000050
#define WLAN_GPIO_PIN11_ADDRESS			0x00000054
#define WLAN_GPIO_PIN12_ADDRESS			0x00000058
#define WLAN_GPIO_PIN13_ADDRESS			0x0000005c

#define CLOCK_GPIO_OFFSET			0xffffffff
#define CLOCK_GPIO_BT_CLK_OUT_EN_LSB		0
#define CLOCK_GPIO_BT_CLK_OUT_EN_MASK		0

#define SI_CONFIG_OFFSET			0x00000000
#define SI_CONFIG_BIDIR_OD_DATA_LSB		18
#define SI_CONFIG_BIDIR_OD_DATA_MASK		0x00040000
#define SI_CONFIG_I2C_LSB			16
#define SI_CONFIG_I2C_MASK			0x00010000
#define SI_CONFIG_POS_SAMPLE_LSB		7
#define SI_CONFIG_POS_SAMPLE_MASK		0x00000080
#define SI_CONFIG_INACTIVE_DATA_LSB		5
#define SI_CONFIG_INACTIVE_DATA_MASK		0x00000020
#define SI_CONFIG_INACTIVE_CLK_LSB		4
#define SI_CONFIG_INACTIVE_CLK_MASK		0x00000010
#define SI_CONFIG_DIVIDER_LSB			0
#define SI_CONFIG_DIVIDER_MASK			0x0000000f
#define SI_CS_OFFSET				0x00000004
#define SI_CS_DONE_ERR_MASK			0x00000400
#define SI_CS_DONE_INT_MASK			0x00000200
#define SI_CS_START_LSB				8
#define SI_CS_START_MASK			0x00000100
#define SI_CS_RX_CNT_LSB			4
#define SI_CS_RX_CNT_MASK			0x000000f0
#define SI_CS_TX_CNT_LSB			0
#define SI_CS_TX_CNT_MASK			0x0000000f

#define SI_TX_DATA0_OFFSET			0x00000008
#define SI_TX_DATA1_OFFSET			0x0000000c
#define SI_RX_DATA0_OFFSET			0x00000010
#define SI_RX_DATA1_OFFSET			0x00000014

#define CORE_CTRL_CPU_INTR_MASK			0x00002000
#define CORE_CTRL_ADDRESS			0x0000
#define PCIE_INTR_ENABLE_ADDRESS		0x0008
#define PCIE_INTR_CLR_ADDRESS			0x0014
#define SCRATCH_3_ADDRESS			0x0030

/* Firmware indications to the Host via SCRATCH_3 register. */
#define FW_INDICATOR_ADDRESS	(SOC_CORE_BASE_ADDRESS + SCRATCH_3_ADDRESS)
#define FW_IND_EVENT_PENDING			1
#define FW_IND_INITIALIZED			2

/* HOST_REG interrupt from firmware */
#define PCIE_INTR_FIRMWARE_MASK			0x00000400
#define PCIE_INTR_CE_MASK_ALL			0x0007f800

#define DRAM_BASE_ADDRESS			0x00400000

#define MISSING 0

#define SYSTEM_SLEEP_OFFSET			SOC_SYSTEM_SLEEP_OFFSET
#define WLAN_SYSTEM_SLEEP_OFFSET		SOC_SYSTEM_SLEEP_OFFSET
#define WLAN_RESET_CONTROL_OFFSET		SOC_RESET_CONTROL_OFFSET
#define CLOCK_CONTROL_OFFSET			SOC_CLOCK_CONTROL_OFFSET
#define CLOCK_CONTROL_SI0_CLK_MASK		SOC_CLOCK_CONTROL_SI0_CLK_MASK
#define RESET_CONTROL_MBOX_RST_MASK		MISSING
#define RESET_CONTROL_SI0_RST_MASK		SOC_RESET_CONTROL_SI0_RST_MASK
#define GPIO_BASE_ADDRESS			WLAN_GPIO_BASE_ADDRESS
#define GPIO_PIN0_OFFSET			WLAN_GPIO_PIN0_ADDRESS
#define GPIO_PIN1_OFFSET			WLAN_GPIO_PIN1_ADDRESS
#define GPIO_PIN0_CONFIG_MASK			WLAN_GPIO_PIN0_CONFIG_MASK
#define GPIO_PIN1_CONFIG_MASK			WLAN_GPIO_PIN1_CONFIG_MASK
#define SI_BASE_ADDRESS				WLAN_SI_BASE_ADDRESS
#define SCRATCH_BASE_ADDRESS			SOC_CORE_BASE_ADDRESS
#define LOCAL_SCRATCH_OFFSET			0x18
#define CPU_CLOCK_OFFSET			SOC_CPU_CLOCK_OFFSET
#define LPO_CAL_OFFSET				SOC_LPO_CAL_OFFSET
#define GPIO_PIN10_OFFSET			WLAN_GPIO_PIN10_ADDRESS
#define GPIO_PIN11_OFFSET			WLAN_GPIO_PIN11_ADDRESS
#define GPIO_PIN12_OFFSET			WLAN_GPIO_PIN12_ADDRESS
#define GPIO_PIN13_OFFSET			WLAN_GPIO_PIN13_ADDRESS
#define CPU_CLOCK_STANDARD_LSB			SOC_CPU_CLOCK_STANDARD_LSB
#define CPU_CLOCK_STANDARD_MASK			SOC_CPU_CLOCK_STANDARD_MASK
#define LPO_CAL_ENABLE_LSB			SOC_LPO_CAL_ENABLE_LSB
#define LPO_CAL_ENABLE_MASK			SOC_LPO_CAL_ENABLE_MASK
#define ANALOG_INTF_BASE_ADDRESS		WLAN_ANALOG_INTF_BASE_ADDRESS
#define MBOX_BASE_ADDRESS			MISSING
#define INT_STATUS_ENABLE_ERROR_LSB		MISSING
#define INT_STATUS_ENABLE_ERROR_MASK		MISSING
#define INT_STATUS_ENABLE_CPU_LSB		MISSING
#define INT_STATUS_ENABLE_CPU_MASK		MISSING
#define INT_STATUS_ENABLE_COUNTER_LSB		MISSING
#define INT_STATUS_ENABLE_COUNTER_MASK		MISSING
#define INT_STATUS_ENABLE_MBOX_DATA_LSB		MISSING
#define INT_STATUS_ENABLE_MBOX_DATA_MASK	MISSING
#define ERROR_STATUS_ENABLE_RX_UNDERFLOW_LSB	MISSING
#define ERROR_STATUS_ENABLE_RX_UNDERFLOW_MASK	MISSING
#define ERROR_STATUS_ENABLE_TX_OVERFLOW_LSB	MISSING
#define ERROR_STATUS_ENABLE_TX_OVERFLOW_MASK	MISSING
#define COUNTER_INT_STATUS_ENABLE_BIT_LSB	MISSING
#define COUNTER_INT_STATUS_ENABLE_BIT_MASK	MISSING
#define INT_STATUS_ENABLE_ADDRESS		MISSING
#define CPU_INT_STATUS_ENABLE_BIT_LSB		MISSING
#define CPU_INT_STATUS_ENABLE_BIT_MASK		MISSING
#define HOST_INT_STATUS_ADDRESS			MISSING
#define CPU_INT_STATUS_ADDRESS			MISSING
#define ERROR_INT_STATUS_ADDRESS		MISSING
#define ERROR_INT_STATUS_WAKEUP_MASK		MISSING
#define ERROR_INT_STATUS_WAKEUP_LSB		MISSING
#define ERROR_INT_STATUS_RX_UNDERFLOW_MASK	MISSING
#define ERROR_INT_STATUS_RX_UNDERFLOW_LSB	MISSING
#define ERROR_INT_STATUS_TX_OVERFLOW_MASK	MISSING
#define ERROR_INT_STATUS_TX_OVERFLOW_LSB	MISSING
#define COUNT_DEC_ADDRESS			MISSING
#define HOST_INT_STATUS_CPU_MASK		MISSING
#define HOST_INT_STATUS_CPU_LSB			MISSING
#define HOST_INT_STATUS_ERROR_MASK		MISSING
#define HOST_INT_STATUS_ERROR_LSB		MISSING
#define HOST_INT_STATUS_COUNTER_MASK		MISSING
#define HOST_INT_STATUS_COUNTER_LSB		MISSING
#define RX_LOOKAHEAD_VALID_ADDRESS		MISSING
#define WINDOW_DATA_ADDRESS			MISSING
#define WINDOW_READ_ADDR_ADDRESS		MISSING
#define WINDOW_WRITE_ADDR_ADDRESS		MISSING

#define RTC_STATE_V_GET(x) (((x) & RTC_STATE_V_MASK) >> RTC_STATE_V_LSB)

#endif /* _HW_H_ */
