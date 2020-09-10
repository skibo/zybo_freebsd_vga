// SPDX-License-Identifier: GPL-2.0+
/*
 * Driver for Xilinx frame buffer.
 *
 * Copyright (C) 2020 Thomas Skibo
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <video.h>
#include <asm/io.h>
#include <asm/cache.h>

#ifdef CONFIG_DM_VIDEO

DECLARE_GLOBAL_DATA_PTR;

/*
 * Xilinx Video Timing Controller (v6.1) registers.  See product guide,
 * PG016 November 18, 2015.
 */
#define XVTC_BASE		0x43c00000
#define XVTC_CTL		(XVTC_BASE + 0x0000)
#define   XVTC_CTL_SW_ENABLE			(1 << 0)
#define   XVTC_CTL_REG_UPDATE			(1 << 1)
#define   XVTC_CTL_GEN_ENABLE			(1 << 2)
#define   XVTC_CTL_DET_ENABLE			(1 << 3)
#define   XVTC_CTL_SYNC_ENABLE			(1 << 5)

#define   XVTC_CTL_FRAME_HSIZE_SRC		(1 << 8)
#define   XVTC_CTL_ACTIVE_HSIZE_SRC		(1 << 9)
#define   XVTC_CTL_HSYNC_START_SRC		(1 << 10)
#define   XVTC_CTL_HSYNC_END_SRC		(1 << 11)
#define   XVTC_CTL_FRAME_VSIZE_SRC		(1 << 13)
#define   XVTC_CTL_ACTIVE_VSIZE_SRC		(1 << 14)
#define   XVTC_CTL_VSYNC_START_SRC		(1 << 15)
#define   XVTC_CTL_END_SRC			(1 << 16)
#define   XVTC_CTL_VBLANK_HOFF_SRC		(1 << 17)
#define   XVTC_CTL_CHROMA_SRC			(1 << 18)
#define   XVTC_CTL_VBLANK_POL_SRC		(1 << 20)
#define   XVTC_CTL_HBLANK_POL_SRC		(1 << 21)
#define   XVTC_CTL_VSYNC_POL_SRC		(1 << 22)
#define   XVTC_CTL_HSYNC_POL_SRC		(1 << 23)
#define   XVTC_CTL_ACTIVE_VIDEO_POL_SRC		(1 << 24)
#define   XVTC_CTL_ACTIVE_CHROMA_POL_SRC	(1 << 25)

#define   XVTC_CTL_FSYNC_RESET			(1 << 30)
#define   XVTC_CTL_SW_RESET			(1U << 31)
#define XVTC_STATS		(XVTC_BASE + 0x0004)
#define   XVTC_STATS_LOCK			(1 << 8)
#define   XVTC_STATS_LOCK_LOSS			(1 << 9)
#define   XVTC_STATS_DET_VBLANK			(1 << 10)
#define   XVTC_STATS_DET_ACTIVE_VIDEO		(1 << 11)
#define   XVTC_STATS_GEN_VBLANK			(1 << 12)
#define   XVTC_STATS_GEN_ACTIVE_VIDEO		(1 << 13)
#define   XVTC_STATS_FSYNC_MASK			(0xffffU << 16)
#define   XVTC_STATS_FSYNC_SHIFT		16
#define XVTC_ERROR		(XVTC_BASE + 0x0008)
#define   XVTC_ERROR_VBLANK_LOCK		(1 << 16)
#define   XVTC_ERROR_HBLANK_LOCK		(1 << 17)
#define   XVTC_ERROR_VSYNC_LOCK			(1 << 18)
#define   XVTC_ERROR_HSYNC_LOCK			(1 << 19)
#define   XVTC_ERROR_ACTIVE_VIDEO_LOCK		(1 << 20)
#define   XVTC_ERROR_ACTIVE_CHROMA_LOCK		(1 << 21)
#define XVTC_IER		(XVTC_BASE + 0x000c)	/* IRQ Enable. */
#define XVTC_VER		(XVTC_BASE + 0x0010)
#define XVTC_DASIZE		(XVTC_BASE + 0x0020)
#define   XVTC_DASIZE_VSIZE_MASK		(0x1fff << 16)
#define   XVTC_DASIZE_VSIZE_SHIFT		16
#define   XVTC_DASIZE_HSIZE_MASK		0x1fff
#define   XVTC_DASIZE_HSIZE_SHIFT		0
#define XVTC_DTSTAT		(XVTC_BASE + 0x0024)
#define XVTC_DFENC		(XVTC_BASE + 0x0028)
#define XVTC_DPOL		(XVTC_BASE + 0x002c)
#define XVTC_DHSIZE		(XVTC_BASE + 0x0030)
#define XVTC_DVSIZE		(XVTC_BASE + 0x0034)
#define XVTC_DHSYNC		(XVTC_BASE + 0x0038)
#define XVTC_DVBHOFF		(XVTC_BASE + 0x003c)
#define XVTC_DVSYNC		(XVTC_BASE + 0x0040)
#define XVTC_DVSHOFF		(XVTC_BASE + 0x0044)
#define XVTC_GASIZE		(XVTC_BASE + 0x0060)
#define   XVTC_GASIZE_VSIZE_MASK		(0x1fff << 16)
#define   XVTC_GASIZE_VSIZE_SHIFT		16
#define   XVTC_GASIZE_HSIZE_MASK		0x1fff
#define   XVTC_GASIZE_HSIZE_SHIFT		0
#define XVTC_GTSTAT		(XVTC_BASE + 0x0064)
#define XVTC_GFENC		(XVTC_BASE + 0x0068)
#define XVTC_GPOL		(XVTC_BASE + 0x006c)
#define XVTC_GHSIZE		(XVTC_BASE + 0x0070)
#define   XVTC_GHSIZE_MASK			0x1fff
#define   XVTC_GHSIZE_SHIFT			0
#define XVTC_GVSIZE		(XVTC_BASE + 0x0074)
#define   XVTC_GVSIZE_MASK			0x1fff
#define   XVTC_GVSIZE_SHIFT			0
#define XVTC_GHSYNC		(XVTC_BASE + 0x0078)
#define   XVTC_GHSYNC_END_MASK			(0x1fff << 16)
#define   XVTC_GHSYNC_END_SHIFT			16
#define   XVTC_GHSYNC_START_MASK		0x1fff
#define   XVTC_GHSYNC_START_SHIFT		0
#define XVTC_GVBHOFF		(XVTC_BASE + 0x007c)
#define XVTC_GVSYNC		(XVTC_BASE + 0x0080)
#define   XVTC_GVSYNC_END_MASK			(0x1fff << 16)
#define   XVTC_GVSYNC_END_SHIFT			16
#define   XVTC_GVSYNC_START_MASK		0x1fff
#define   XVTC_GVSYNC_START_SHIFT		0
#define XVTC_GVSHOFF		(XVTC_BASE + 0x0084)
#define XVTC_FS(x)		(XVTC_BASE + 0x0100 + 4 * (x))
#define XVTC_GGD		(XVTC_BASE + 0x0140)

/*
 * Xilinx AXI Video Direct Memory Access device (v6.2).  See LogiCORE IP
 * Product Guide PG020 November 18, 2015.
 */
#define XVDMA_BASE			0x43000000
#define XVDMA_MM2S_CTRL			(XVDMA_BASE + 0x0000)
#define   XVDMA_MM2S_CTRL_IRQ_DLY_CT_MASK	(0xffU << 24)
#define   XVDMA_MM2S_CTRL_IRQ_DLY_CT_SHIFT	24
#define   XVDMA_MM2S_CTRL_IRQ_FRM_CT_MASK	(0xff << 16)
#define   XVDMA_MM2S_CTRL_IRQ_FRM_CT_SHIFT	16
#define   XVDMA_MM2S_CTRL_REPEAT_EN		(1 << 15)
#define   XVDMA_MM2S_CTRL_ERR_IRQ_EN		(1 << 14)
#define   XVDMA_MM2S_CTRL_DLY_CT_IRQ_EN		(1 << 13)
#define   XVDMA_MM2S_CTRL_FRM_CT_IRQ_EN		(1 << 12)
#define   XVDMA_MM2S_CTRL_RD_PTR_NUM_MASK	(0xf << 8)
#define   XVDMA_MM2S_CTRL_RD_PTR_NUM_SHIFT	8
#define   XVDMA_MM2S_CTRL_GENLOCK_SRC		(1 << 7)
#define   XVDMA_MM2S_CTRL_FRM_CT_EN		(1 << 4)
#define   XVDMA_MM2S_CTRL_GENLOCK_EN		(1 << 3)
#define   XVDMA_MM2S_CTRL_RESET			(1 << 2)
#define   XVDMA_MM2S_CTRL_CIRC			(1 << 1)	/* vs. PARK */
#define   XVDMA_MM2S_CTRL_RUN			(1 << 0)	/* vs. STOP */
#define XVDMA_MM2S_STAT			(XVDMA_BASE + 0x0004)
#define   XVDMA_MM2S_STAT_IRQ_DLY_CT_MASK	(0xffU << 24)
#define   XVDMA_MM2S_STAT_IRQ_DLY_CT_SHIFT	24
#define   XVDMA_MM2S_STAT_IRQ_FRM_CT_MASK	(0xff << 16)
#define   XVDMA_MM2S_STAT_IRQ_FRM_CT_SHIFT	16
#define   XVDMA_MM2S_STAT_ERR_IRQ		(1 << 14)
#define   XVDMA_MM2S_STAT_DLY_CT_IRQ		(1 << 13)
#define   XVDMA_MM2S_STAT_FRM_CT_IRQ		(1 << 12)
#define   XVDMA_MM2S_STAT_SOF_EARLY_ERR		(1 << 7)
#define   XVDMA_MM2S_STAT_VDMA_DEC_ERR		(1 << 6)
#define   XVDMA_MM2S_STAT_VDMA_SLV_ERR		(1 << 5)
#define   XVDMA_MM2S_STAT_VDMA_INTERNAL_ERR	(1 << 4)
#define   XVDMA_MM2S_STAT_HALTED		(1 << 0)
#define XVDMA_MM2S_REG_INDEX		(XVDMA_BASE + 0x0014)
#define   XVDMA_MM2S_REG_INDEX_BIT		1
#define XVDMA_PARK_PTR			(XVDMA_BASE + 0x0028)
#define   XVDMA_PARK_PTR_WR_FRM_STORE_MASK	(0x1f << 24)
#define   XVDMA_PARK_PTR_WR_FRM_STORE_SHIFT	24
#define   XVDMA_PARK_PTR_RD_FRM_STORE_MASK	(0x1f << 16)
#define   XVDMA_PARK_PTR_RD_FRM_STORE_SHIFT	16
#define   XVDMA_PARK_PTR_WR_FRM_PTR_REF_MASK	(0x1f << 8)
#define   XVDMA_PARK_PTR_WR_FRM_PTR_REF_SHIFT	8
#define   XVDMA_PARK_PTR_RD_FRM_PTR_REF_MASK	0x1f
#define   XVDMA_PARK_PTR_RD_FRM_PTR_REF_SHIFT	0
#define XVDMA_VERSION			(XVDMA_BASE + 0x002c)
#define XVDMA_S2MM_CTRL			(XVDMA_BASE + 0x0030)
#define   XVDMA_S2MM_CTRL_IRQ_DLY_CT_MASK	(0xffU << 24)
#define   XVDMA_S2MM_CTRL_IRQ_DLY_CT_SHIFT	24
#define   XVDMA_S2MM_CTRL_IRQ_FRM_CT_MASK	(0xff << 16)
#define   XVDMA_S2MM_CTRL_IRQ_FRM_CT_SHIFT	16
#define   XVDMA_S2MM_CTRL_REPEAT_EN		(1 << 15)
#define   XVDMA_S2MM_CTRL_ERR_IRQ_EN		(1 << 14)
#define   XVDMA_S2MM_CTRL_DLY_CT_IRQ_EN		(1 << 13)
#define   XVDMA_S2MM_CTRL_FRM_CT_IRQ_EN		(1 << 12)
#define   XVDMA_S2MM_CTRL_WR_PTR_NUM_MASK	(0xf << 8)
#define   XVDMA_S2MM_CTRL_WR_PTR_NUM_SHIFT	8
#define   XVDMA_S2MM_CTRL_GENLOCK_SRC		(1 << 7)
#define   XVDMA_S2MM_CTRL_FRM_CT_EN		(1 << 4)
#define   XVDMA_S2MM_CTRL_GENLOCK_EN		(1 << 3)
#define   XVDMA_S2MM_CTRL_RESET			(1 << 2)
#define   XVDMA_S2MM_CTRL_CIRC			(1 << 1)	/* vs. PARK */
#define   XVDMA_S2MM_CTRL_RUN			(1 << 0)	/* vs. STOP */
#define XVDMA_S2MM_STAT			(XVDMA_BASE + 0x0034)
#define   XVDMA_S2MM_STAT_IRQ_DLY_CT_MASK	(0xffU << 24)
#define   XVDMA_S2MM_STAT_IRQ_DLY_CT_SHIFT	24
#define   XVDMA_S2MM_STAT_IRQ_FRM_CT_MASK	(0xff << 16)
#define   XVDMA_S2MM_STAT_IRQ_FRM_CT_SHIFT	16
#define   XVDMA_S2MM_STAT_EOL_LATE_ERR		(1 << 15)
#define   XVDMA_S2MM_STAT_ERR_IRQ		(1 << 14)
#define   XVDMA_S2MM_STAT_DLY_CT_IRQ		(1 << 13)
#define   XVDMA_S2MM_STAT_FRM_CT_IRQ		(1 << 12)
#define   XVDMA_S2MM_STAT_SOF_LATE_ERR		(1 << 11)
#define   XVDMA_S2MM_STAT_EOL_EARLY_ERR		(1 << 8)
#define   XVDMA_S2MM_STAT_SOF_EARLY_ERR		(1 << 7)
#define   XVDMA_S2MM_STAT_VDMA_DEC_ERR		(1 << 6)
#define   XVDMA_S2MM_STAT_VDMA_SLV_ERR		(1 << 5)
#define   XVDMA_S2MM_STAT_VDMA_INTERNAL_ERR	(1 << 4)
#define   XVDMA_S2MM_STAT_HALTED		(1 << 0)
#define XVDMA_S2MM_IRQ_MASK		(XVDMA_BASE + 0x003c)
#define   XVDMA_S2MM_IRQ_MASK_EOL_LATE_ERR	(1 << 3)	/* 1=masked */
#define   XVDMA_S2MM_IRQ_MASK_SOF_LATE_ERR	(1 << 2)
#define   XVDMA_S2MM_IRQ_MASK_EOL_EARLY_ERR	(1 << 1)
#define   XVDMA_S2MM_IRQ_MASK_SOF_EARLY_ERR	(1 << 0)
#define XVDMA_S2MM_REG_INDEX		(XVDMA_BASE + 0x0044)
#define   XVDMA_S2MM_REG_INDEX_BIT		1
#define XVDMA_MM2S_VSIZE		(XVDMA_BASE + 0x0050)
#define XVDMA_MM2S_HSIZE		(XVDMA_BASE + 0x0054)
#define XVDMA_MM2S_FRMDLY_STRIDE	(XVDMA_BASE + 0x0058)
#define   XVDMA_MM2S_FRMDLY_STRIDE_FMRDLY_MASK	(0x1f << 24)
#define   XVDMA_MM2S_FRMDLY_STRIDE_FMRDLY_SHIFT	24
#define   XVDMA_MM2S_FRMDLY_STRIDE_STRIDE_MASK	0xffff
#define XVDMA_MM2S_START_ADDR(x)	(XVDMA_BASE + 0x005c + 4 * (x))
#define XVDMA_S2MM_VSIZE		(XVDMA_BASE + 0x00a0)
#define XVDMA_S2MM_HSIZE		(XVDMA_BASE + 0x00a4)
#define XVDMA_S2MM_FRMDLY_STRIDE	(XVDMA_BASE + 0x00a8)
#define   XVDMA_S2MM_FRMDLY_STRIDE_FMRDLY_MASK	(0x1f << 24)
#define   XVDMA_S2MM_FRMDLY_STRIDE_FMRDLY_SHIFT	24
#define   XVDMA_S2MM_FRMDLY_STRIDE_STRIDE_MASK	0xffff
#define XVDMA_S2MM_START_ADDR(x)	(XVDMA_BASE + 0x00ac + 4 * (x))

static int xlnx_fb_probe(struct udevice *dev)
{
	struct video_uc_platdata *uc_plat = dev_get_uclass_platdata(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	uint32_t active;
	int width;
	int height;

	debug("%s: base=%lx\n", __func__, uc_plat->base);

	/* Get width and height from timing controller. */
	active = readl(XVTC_GASIZE);
	width = (active & XVTC_GASIZE_HSIZE_MASK) >> XVTC_GASIZE_HSIZE_SHIFT;
	height = (active & XVTC_GASIZE_VSIZE_MASK) >> XVTC_GASIZE_VSIZE_SHIFT;

	/* Initialize timing controller. */
	writel(~0, XVTC_STATS);
	writel(~0, XVTC_ERROR);
	writel(0, XVTC_IER);
	writel(XVTC_CTL_GEN_ENABLE |
		XVTC_CTL_FRAME_HSIZE_SRC |
		XVTC_CTL_ACTIVE_HSIZE_SRC |
		XVTC_CTL_HSYNC_START_SRC |
		XVTC_CTL_HSYNC_END_SRC |
		XVTC_CTL_FRAME_VSIZE_SRC |
		XVTC_CTL_ACTIVE_VSIZE_SRC |
		XVTC_CTL_VSYNC_START_SRC |
		XVTC_CTL_END_SRC |
		XVTC_CTL_VBLANK_HOFF_SRC |
		XVTC_CTL_CHROMA_SRC |
		XVTC_CTL_VBLANK_POL_SRC |
		XVTC_CTL_HBLANK_POL_SRC |
		XVTC_CTL_VSYNC_POL_SRC |
		XVTC_CTL_HSYNC_POL_SRC |
		XVTC_CTL_ACTIVE_VIDEO_POL_SRC |
		XVTC_CTL_ACTIVE_CHROMA_POL_SRC,
		XVTC_CTL);

	/* Initialize video DMA (MM2S) controller. */
	writel(XVDMA_MM2S_CTRL_RUN, XVDMA_MM2S_CTRL);
	writel(uc_plat->base, XVDMA_MM2S_START_ADDR(0));
	writel(width * 4, XVDMA_MM2S_FRMDLY_STRIDE);
	writel(width * 4, XVDMA_MM2S_HSIZE);
	writel(height, XVDMA_MM2S_VSIZE);

	mmu_set_region_dcache_behaviour(uc_plat->base, uc_plat->size,
					DCACHE_WRITETHROUGH);

	uc_priv->xsize = width;
	uc_priv->ysize = height;
	uc_priv->bpix = VIDEO_BPP32;

	return 0;
}

static int xlnx_fb_bind(struct udevice *dev)
{
	struct video_uc_platdata *uc_plat = dev_get_uclass_platdata(dev);
	uint32_t active;
	int width;
	int height;

	/* Get width and height from timing controller. */
	active = readl(XVTC_GASIZE);
	width = (active & XVTC_GASIZE_HSIZE_MASK) >> XVTC_GASIZE_HSIZE_SHIFT;
	height = (active & XVTC_GASIZE_VSIZE_MASK) >> XVTC_GASIZE_VSIZE_SHIFT;

	uc_plat->size = width * height * 4;

	debug("%s: width %d height %d size %d\n", __func__, width, height,
		uc_plat->size);

	return 0;
}

static const struct udevice_id xlnx_fb_ids[] = {
	{ .compatible = "xlnx,xlnx-fb" },
	{ }
};

U_BOOT_DRIVER(atmel_fb) = {
	.name	= "xlnx_fb",
	.id	= UCLASS_VIDEO,
	.of_match = xlnx_fb_ids,
	.bind = xlnx_fb_bind,
	.probe	= xlnx_fb_probe,
};

#endif /* CONFIG_DM_VIDEO */
