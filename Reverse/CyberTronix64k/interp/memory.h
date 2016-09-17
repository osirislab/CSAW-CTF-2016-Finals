#pragma once
#include <cstdio>
#include <atomic>

typedef uint16_t mem_t;

#define PUTC_LOC 0x200
#define GETC_LOC 0x201


class memory {
    public:
        memory() : mem{0} {};
        // access methods
        mem_t& operator[](size_t idx) {
            if (idx == PUTC_LOC) {
                must_print = true;
                return mem[idx];
            }
            if (idx == GETC_LOC) {
                mem[idx] = mem_t(getchar());
                return mem[idx];
            }
            return mem[idx];
        }

        void flush_changes() {
            if (must_print) {
                putchar(char(mem[PUTC_LOC]));
            must_print = false;
        }
    }

    bool must_print;
    mem_t mem[1<<16] = {0}; // lookups that don't hit mmapped regions hit this
};
