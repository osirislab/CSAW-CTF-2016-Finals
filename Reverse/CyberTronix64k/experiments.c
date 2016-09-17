#include <stdio.h>
#include <stdint.h>

uint16_t cksum(uint16_t* data, uint16_t nbytes) {
    uint16_t cks1 = 0, cks2 = 0, idx = 0;
    while (idx < nbytes) {
        uint16_t dp = data[idx];
        cks1 += dp;
        cks2 += cks1 + ((dp & 0xFF00)>>8);
        idx++;
    }
    cks1 &= 0xFF;
    cks2 &= 0xFF;
    return (cks1 << 8) | cks2;
}

// swizzle data of len 25
uint16_t* swizzle(uint16_t* data) {
    uint16_t * end = data+25;
    while (data != end) {
        uint16_t dp = *data;
        uint16_t val = 0;
        val |= (dp & 0b1100000000000000) >> 14; // 0 -> 7
        val |= (dp & 0b11000000000000) << 2; // 1 -> 0
        val |= (dp & 0b110000000000) >> 2; // 2 -> 3
        val |= (dp & 0b1100000000) << 4; // 3 -> 1
        val |= (dp & 0b11000000) << 4; // 4 -> 2
        val |= (dp & 0b110000) >> 2; // 5 -> 6 
        val |= (dp & 0b1100) << 4; // 6 -> 4
        val |= (dp & 0b11) << 4; // 7 -> 5
        *data = val;
        data++;
    }
    return data-25;
}

void divmod(uint16_t a, uint16_t b, uint16_t* div, uint16_t* mod) {
    uint16_t num = 0;
    while (a > b) {
        a -= b;
        num++;
    }
    *div = num;
    *mod = a;
}

int main() {
    uint16_t data[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
        'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
        'Y'
    };
    swizzle(data);
    for (size_t i = 0; i < 25; i++) {
        printf("0x%04x\n", data[i]);
    }
    printf("\n");

}
