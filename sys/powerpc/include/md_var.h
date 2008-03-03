/*-
 * Copyright (c) 1998 Doug Rabson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#ifndef _MACHINE_MD_VAR_H_
#define	_MACHINE_MD_VAR_H_

/*
 * Miscellaneous machine-dependent declarations.
 */

extern	char	sigcode[];
extern	char	esigcode[];
extern	int	szsigcode;
extern	long	Maxmem;
extern	int	busdma_swi_pending;

extern	vm_offset_t	kstack0;
extern	vm_offset_t	kstack0_phys;

extern	u_long	ns_per_tick;

#if defined(_KERNEL) || defined(_STANDALONE)
#define	CACHELINESIZE	32
#endif

void	__syncicache(void *, int);

void	busdma_swi(void);
int	is_physical_memory(vm_offset_t addr);
int	mem_valid(vm_offset_t addr, int len);

void	decr_config(unsigned long);
void	decr_init(void);
void	decr_tc_init(void);

void	cpu_setup(u_int);

struct trapframe;
void	powerpc_interrupt(struct trapframe *);

#endif /* !_MACHINE_MD_VAR_H_ */
