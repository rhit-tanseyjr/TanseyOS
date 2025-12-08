#include "kernel.h"

// Interrupt cause codes
#define CAUSE_SUPERVISOR_TIMER    5
#define CAUSE_SUPERVISOR_SOFTWARE 1
#define CAUSE_SUPERVISOR_EXTERNAL 9

// Exception cause codes
#define CAUSE_ILLEGAL_INSTRUCTION 2
#define CAUSE_BREAKPOINT         3
#define CAUSE_LOAD_ACCESS_FAULT  5
#define CAUSE_STORE_ACCESS_FAULT 7

// Trap frame - saved register state
struct trap_frame {
    uint32_t ra;   // return address
    uint32_t sp;   // stack pointer
    uint32_t gp;   // global pointer
    uint32_t tp;   // thread pointer
    uint32_t t0;   // temporaries
    uint32_t t1;
    uint32_t t2;
    uint32_t s0;   // saved registers
    uint32_t s1;
    uint32_t a0;   // function arguments
    uint32_t a1;
    uint32_t a2;
    uint32_t a3;
    uint32_t a4;
    uint32_t a5;
    uint32_t a6;
    uint32_t a7;
    uint32_t s2;   // more saved registers
    uint32_t s3;
    uint32_t s4;
    uint32_t s5;
    uint32_t s6;
    uint32_t s7;
    uint32_t s8;
    uint32_t s9;
    uint32_t s10;
    uint32_t s11;
    uint32_t t3;   // more temporaries
    uint32_t t4;
    uint32_t t5;
    uint32_t t6;
};

// Global timer tick counter
static uint32_t timer_ticks = 0;

// Timer interrupt handler
void handle_timer_interrupt(void) {
    timer_ticks++;
    
    // Print a message every 100 ticks (about 1 second at 10ms intervals)
    if (timer_ticks % 100 == 0) {
        printf("Timer tick: %d\n", timer_ticks);
    }
    
    // Acknowledge timer interrupt and set next timer
    // This tells OpenSBI to set the next timer interrupt
    sbi_call(10000000 / 100, 0, 0, 0, 0, 0, 0, 0x54494D45 /* Timer extension */);
}

// Main trap handler (called from assembly)
void handle_trap(struct trap_frame *frame) {
    uint32_t scause, stval, sepc;
    
    // Read trap cause and related registers
    __asm__ __volatile__("csrr %0, scause" : "=r"(scause));
    __asm__ __volatile__("csrr %0, stval" : "=r"(stval));
    __asm__ __volatile__("csrr %0, sepc" : "=r"(sepc));
    
    // Check if it's an interrupt (bit 31 set) or exception
    if (scause & (1u << 31)) {
        // It's an interrupt
        uint32_t cause = scause & 0x7fffffff;
        
        switch (cause) {
            case CAUSE_SUPERVISOR_TIMER:
                handle_timer_interrupt();
                break;
                
            case CAUSE_SUPERVISOR_SOFTWARE:
                printf("Software interrupt received\n");
                break;
                
            case CAUSE_SUPERVISOR_EXTERNAL:
                printf("External interrupt received\n");
                break;
                
            default:
                printf("Unknown interrupt: %d\n", cause);
                break;
        }
    } else {
        // It's an exception
        printf("Exception occurred!\n");
        printf("  Cause: %d\n", scause);
        printf("  Address: %x\n", stval);
        printf("  PC: %x\n", sepc);
        
        switch (scause) {
            case CAUSE_ILLEGAL_INSTRUCTION:
                printf("  Type: Illegal instruction\n");
                break;
            case CAUSE_BREAKPOINT:
                printf("  Type: Breakpoint\n");
                break;
            case CAUSE_LOAD_ACCESS_FAULT:
                printf("  Type: Load access fault\n");
                break;
            case CAUSE_STORE_ACCESS_FAULT:
                printf("  Type: Store access fault\n");
                break;
            default:
                printf("  Type: Unknown exception\n");
                break;
        }
        
        // Halt on exceptions
        for (;;) {
            __asm__ __volatile__("wfi");
        }
    }
}

// Assembly trap entry point - saves all registers and calls handle_trap
__attribute__((naked))
__attribute__((aligned(4)))
void kernel_trap(void) {
    __asm__ __volatile__(
        // Allocate space for trap frame (32 registers * 4 bytes)
        "addi sp, sp, -128\n"
        
        // Save all registers
        "sw ra, 0(sp)\n"
        "sw sp, 4(sp)\n"   // Note: saves old SP value
        "sw gp, 8(sp)\n"
        "sw tp, 12(sp)\n"
        "sw t0, 16(sp)\n"
        "sw t1, 20(sp)\n"
        "sw t2, 24(sp)\n"
        "sw s0, 28(sp)\n"
        "sw s1, 32(sp)\n"
        "sw a0, 36(sp)\n"
        "sw a1, 40(sp)\n"
        "sw a2, 44(sp)\n"
        "sw a3, 48(sp)\n"
        "sw a4, 52(sp)\n"
        "sw a5, 56(sp)\n"
        "sw a6, 60(sp)\n"
        "sw a7, 64(sp)\n"
        "sw s2, 68(sp)\n"
        "sw s3, 72(sp)\n"
        "sw s4, 76(sp)\n"
        "sw s5, 80(sp)\n"
        "sw s6, 84(sp)\n"
        "sw s7, 88(sp)\n"
        "sw s8, 92(sp)\n"
        "sw s9, 96(sp)\n"
        "sw s10, 100(sp)\n"
        "sw s11, 104(sp)\n"
        "sw t3, 108(sp)\n"
        "sw t4, 112(sp)\n"
        "sw t5, 116(sp)\n"
        "sw t6, 120(sp)\n"
        
        // Call C trap handler with trap frame pointer
        "mv a0, sp\n"
        "call handle_trap\n"
        
        // Restore all registers
        "lw ra, 0(sp)\n"
        "lw gp, 8(sp)\n"
        "lw tp, 12(sp)\n"
        "lw t0, 16(sp)\n"
        "lw t1, 20(sp)\n"
        "lw t2, 24(sp)\n"
        "lw s0, 28(sp)\n"
        "lw s1, 32(sp)\n"
        "lw a0, 36(sp)\n"
        "lw a1, 40(sp)\n"
        "lw a2, 44(sp)\n"
        "lw a3, 48(sp)\n"
        "lw a4, 52(sp)\n"
        "lw a5, 56(sp)\n"
        "lw a6, 60(sp)\n"
        "lw a7, 64(sp)\n"
        "lw s2, 68(sp)\n"
        "lw s3, 72(sp)\n"
        "lw s4, 76(sp)\n"
        "lw s5, 80(sp)\n"
        "lw s6, 84(sp)\n"
        "lw s7, 88(sp)\n"
        "lw s8, 92(sp)\n"
        "lw s9, 96(sp)\n"
        "lw s10, 100(sp)\n"
        "lw s11, 104(sp)\n"
        "lw t3, 108(sp)\n"
        "lw t4, 112(sp)\n"
        "lw t5, 116(sp)\n"
        "lw t6, 120(sp)\n"
        
        // Restore stack pointer and return
        "lw sp, 4(sp)\n"
        "sret\n"  // Return from trap
    );
}
