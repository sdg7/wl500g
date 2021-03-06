#
# Makefile for the Linux/MIPS kernel.
#

extra-y		:= head.o init_task.o vmlinux.lds

obj-y		+= cpu-probe.o branch.o entry.o genex.o irq.o process.o \
		   ptrace.o reset.o semaphore.o setup.o signal.o syscall.o \
		   time.o topology.o traps.o unaligned.o vdso.o \
		   user_backtrace.o

obj-$(CONFIG_STACKTRACE)	+= stacktrace.o
obj-$(CONFIG_MODULES)		+= mips_ksyms.o module.o

obj-$(CONFIG_CPU_R3000)		+= r2300_fpu.o r2300_switch.o
obj-$(CONFIG_CPU_TX39XX)	+= r2300_fpu.o r2300_switch.o
obj-$(CONFIG_CPU_TX49XX)	+= r4k_fpu.o r4k_switch.o
obj-$(CONFIG_CPU_R4000)		+= r4k_fpu.o r4k_switch.o
obj-$(CONFIG_CPU_VR41XX)	+= r4k_fpu.o r4k_switch.o
obj-$(CONFIG_CPU_R4300)		+= r4k_fpu.o r4k_switch.o
obj-$(CONFIG_CPU_R4X00)		+= r4k_fpu.o r4k_switch.o
obj-$(CONFIG_CPU_R5000)		+= r4k_fpu.o r4k_switch.o
obj-$(CONFIG_CPU_R5432)		+= r4k_fpu.o r4k_switch.o
obj-$(CONFIG_CPU_R8000)		+= r4k_fpu.o r4k_switch.o
obj-$(CONFIG_CPU_RM7000)	+= r4k_fpu.o r4k_switch.o
obj-$(CONFIG_CPU_RM9000)	+= r4k_fpu.o r4k_switch.o
obj-$(CONFIG_CPU_NEVADA)	+= r4k_fpu.o r4k_switch.o
obj-$(CONFIG_CPU_R10000)	+= r4k_fpu.o r4k_switch.o
obj-$(CONFIG_CPU_SB1)		+= r4k_fpu.o r4k_switch.o
obj-$(CONFIG_CPU_MIPS32)	+= r4k_fpu.o r4k_switch.o
obj-$(CONFIG_CPU_MIPS64)	+= r4k_fpu.o r4k_switch.o
obj-$(CONFIG_CPU_R6000)		+= r6000_fpu.o r4k_switch.o

obj-$(CONFIG_SMP)		+= smp.o

obj-$(CONFIG_MIPS_MT)		+= mips-mt.o
obj-$(CONFIG_MIPS_MT_SMTC)	+= smtc.o smtc-asm.o smtc-proc.o
obj-$(CONFIG_MIPS_MT_SMP)	+= smp-mt.o

obj-$(CONFIG_MIPS_VPE_LOADER)	+= vpe.o
obj-$(CONFIG_MIPS_VPE_APSP_API)	+= rtlx.o
obj-$(CONFIG_MIPS_APSP_KSPD)	+= kspd.o

obj-$(CONFIG_I8259)		+= i8259.o
obj-$(CONFIG_IRQ_CPU)		+= irq_cpu.o
obj-$(CONFIG_IRQ_CPU_RM7K)	+= irq-rm7000.o
obj-$(CONFIG_IRQ_CPU_RM9K)	+= irq-rm9000.o
obj-$(CONFIG_IRQ_MV64340)	+= irq-mv6434x.o
obj-$(CONFIG_MIPS_BOARDS_GEN)	+= irq-msc01.o

obj-$(CONFIG_32BIT)		+= scall32-o32.o
obj-$(CONFIG_64BIT)		+= scall64-64.o
obj-$(CONFIG_MIPS32_COMPAT)	+= linux32.o ptrace32.o signal32.o
obj-$(CONFIG_MIPS32_N32)	+= binfmt_elfn32.o scall64-n32.o signal_n32.o
obj-$(CONFIG_MIPS32_O32)	+= binfmt_elfo32.o scall64-o32.o

obj-$(CONFIG_KGDB)		+= gdb-low.o gdb-stub.o
obj-$(CONFIG_PROC_FS)		+= proc.o

obj-$(CONFIG_64BIT)		+= cpu-bugs64.o

obj-$(CONFIG_I8253)		+= i8253.o

obj-$(CONFIG_KEXEC)		+= machine_kexec.o relocate_kernel.o
obj-$(CONFIG_EARLY_PRINTK)	+= early_printk.o

CFLAGS_cpu-bugs64.o	= $(shell if $(CC) $(CFLAGS) -Wa,-mdaddi -c -o /dev/null -x c /dev/null >/dev/null 2>&1; then echo "-DHAVE_AS_SET_DADDI"; fi)

ifneq ($(CONFIG_PRINT_SYSCALLS),)
$(src)/syscall.o: $(src)/syscalls.h

clean-files := syscalls.h

quiet_cmd_syscall_names = GEN     $@
      cmd_syscall_names = $(AWK) -f $(src)/syscalls.awk $< >$@

# Only O32 ABI supported for now
$(src)/syscalls.h: $(src)/scall32-o32.S $(src)/syscalls.awk
	$(call cmd,syscall_names)
endif
