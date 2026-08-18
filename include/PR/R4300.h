#ifndef __R4300_H__
#define __R4300_H__

#include <PR/ultratypes.h>

/*
 * Segment base addresses and sizes
 */
#define	KUBASE		0
#define	KUSIZE		0x80000000
#define	K0BASE		0x80000000
#define	K0SIZE		0x20000000
#define	K1BASE		0xA0000000
#define	K1SIZE		0x20000000
#define	K2BASE		0xC0000000
#define	K2SIZE		0x20000000

/*
 * Exception vectors
 */
#define SIZE_EXCVEC	0x80			/* Size of an exc. vec */
#define	UT_VEC		K0BASE			/* utlbmiss vector */
#define	R_VEC		(K1BASE+0x1fc00000)	/* reset vector */
#define	XUT_VEC		(K0BASE+0x80)		/* extended address tlbmiss */
#define	ECC_VEC		(K0BASE+0x100)		/* Ecc exception vector */
#define	E_VEC		(K0BASE+0x180)		/* Gen. exception vector */

/*
 * Address conversion macros
 */
#ifdef _LANGUAGE_ASSEMBLY

#define	K0_TO_K1(x)	((x)|0xA0000000)	/* kseg0 to kseg1 */
#define	K1_TO_K0(x)	((x)&0x9FFFFFFF)	/* kseg1 to kseg0 */
#define	K0_TO_PHYS(x)	((x)&0x1FFFFFFF)	/* kseg0 to physical */
#define	K1_TO_PHYS(x)	((x)&0x1FFFFFFF)	/* kseg1 to physical */
#define	KDM_TO_PHYS(x)	((x)&0x1FFFFFFF)	/* direct mapped to physical */
#define	PHYS_TO_K0(x)	((x)|0x80000000)	/* physical to kseg0 */
#define	PHYS_TO_K1(x)	((x)|0xA0000000)	/* physical to kseg1 */

#else /* _LANGUAGE_C */

#if defined(TARGET_WEB)
#define	K0_TO_K1(x)	((uintptr_t)(x))
#define	K1_TO_K0(x)	((uintptr_t)(x))
#define	K0_TO_PHYS(x)	((uintptr_t)(x))
#define	K1_TO_PHYS(x)	((uintptr_t)(x))
#define	KDM_TO_PHYS(x)	((uintptr_t)(x))
#define	PHYS_TO_K0(x)	((uintptr_t)(x))
#define	PHYS_TO_K1(x)	((uintptr_t)(x))
#else
#define	K0_TO_K1(x)	((u32)(x)|0xA0000000)	/* kseg0 to kseg1 */
#define	K1_TO_K0(x)	((u32)(x)&0x9FFFFFFF)	/* kseg1 to kseg0 */
#define	K0_TO_PHYS(x)	((u32)(x)&0x1FFFFFFF)	/* kseg0 to physical */
#define	K1_TO_PHYS(x)	((u32)(x)&0x1FFFFFFF)	/* kseg1 to physical */
#define	KDM_TO_PHYS(x)	((u32)(x)&0x1FFFFFFF)	/* direct mapped to physical */
#define	PHYS_TO_K0(x)	((u32)(x)|0x80000000)	/* physical to kseg0 */
#define	PHYS_TO_K1(x)	((u32)(x)|0xA0000000)	/* physical to kseg1 */
#endif

#endif	/* _LANGUAGE_ASSEMBLY */

/*
 * Address predicates
 */
#define	IS_KSEG0(x)	((u32)(x) >= K0BASE && (u32)(x) < K1BASE)
#define	IS_KSEG1(x)	((u32)(x) >= K1BASE && (u32)(x) < K2BASE)
#define	IS_KSEGDM(x)	((u32)(x) >= K0BASE && (u32)(x) < K2BASE)
#define	IS_KSEG2(x)	((u32)(x) >= K2BASE && (u32)(x) < KPTE_SHDUBASE)
#define	IS_KPTESEG(x)	((u32)(x) >= KPTE_SHDUBASE)
#define	IS_KUSEG(x)	((u32)(x) < K0BASE)

/*
 * TLB size constants
 */

#define	NTLBENTRIES	31	/* entry 31 is reserved by rdb */

#define	TLBHI_VPN2MASK		0xffffe000
#define	TLBHI_VPN2SHIFT		13
#define	TLBHI_PIDMASK		0xff
#define	TLBHI_PIDSHIFT		0
#define	TLBHI_NPID		255

#define	TLBLO_PFNMASK		0x3fffffc0
#define	TLBLO_PFNSHIFT		6
#define	TLBLO_CACHMTRX		0x38
#define	TLBLO_UNCACHED		0x10
#define	TLBLO_NONCACHE		0x10
#define	TLBLO_CACHEABLE		0x18
#define	TLBLO_CACHMSK		0x38
#define	TLBLO_CSHIFT		3
#define	TLBLO_D			0x4
#define	TLBLO_V			0x2
#define	TLBLO_G			0x1

#define	TLBINX_PROBE		0x80000000
#define	TLBINX_INXMASK		0x3f
#define	TLBINX_INXSHIFT		0

#define	TLBRAND_RANDMASK	0x3f
#define	TLBRAND_RANDSHIFT	0

#define	TLBWIRED_WIREDMASK	0x3f
#define	TLBWIRED_WIREDSHIFT	0

#define	TLBCTXT_BASEMASK	0xff800000
#define	TLBCTXT_BASESHIFT	23
#define	TLBCTXT_BASEBITS	9

#define	TLBCTXT_VPN2MASK	0x007ffff0
#define	TLBCTXT_VPN2SHIFT	4
#define	TLBCTXT_VPN2BITS	19

#define	TLBPGMASK_4K		0x0
#define	TLBPGMASK_16K		0x6000
#define	TLBPGMASK_64K		0x1e000
#define	TLBPGMASK_256K		0x7e000
#define	TLBPGMASK_1M		0x1fe000
#define	TLBPGMASK_4M		0x7fe000
#define	TLBPGMASK_16M		0x1ffe000

#define	TLBPGMASK_MASK		0x1ffe000
#define	TLBPGMASK_SHIFT		13

#endif /* __R4300_H__ */