#include <stdio.h>
#include <stdint.h>

void replacing_fourth_byte(long long num){
    signed char min_char1 = (signed char)(1 << 7);  // 10000000₂ = -128
    unsigned char *ptr = (unsigned char*)&num;
    ptr[3] = (unsigned char)min_char1;  // 0x80
    
    printf("\n%lld\n", num);
}

long long clear_bits_in_third_bytes(long long num) {
    // Маска для обнуления четных битов (0, 2, 4, 6): 0xAA = 10101010
    unsigned char mask = 0xAA;
    
    unsigned char *bytes = (unsigned char*)&num;
    
    // Обрабатываем каждый третий байт (индексы 0, 3, 6)
    bytes[0] = bytes[0] & mask;  // Байт 0
    bytes[3] = bytes[3] & mask;  // Байт 3
    bytes[6] = bytes[6] & mask;  // Байт 6
    
    printf("\n%lld\n", num);
}

int main() {
    long long num;
    scanf("%lld", &num);
    replacing_fourth_byte(num);
    clear_bits_in_third_bytes(num);
    return 0;
}