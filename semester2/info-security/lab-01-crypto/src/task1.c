#include <stdio.h>
#include <string.h>

int main() {
    char s[1000];
    int k;
    char lower[] = "абвгдеёжзийклмнопрстуфхцчшщъыьэюя";
    char upper[] = "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ";
    int size = 33;

    fgets(s, sizeof(s), stdin);
    s[strcspn(s, "\n")] = 0;
    scanf("%d", &k);
    k = ((k % size) + size) % size;

    for (int i = 0; s[i]; i++) {
        char *p;
        if ((p = strchr(lower, s[i]))) {
            int idx = p - lower;
            putchar(lower[(idx + k) % size]);
        } else if ((p = strchr(upper, s[i]))) {
            int idx = p - upper;
            putchar(upper[(idx + k) % size]);
        } else {
            putchar(s[i]);
        }
    }
    putchar('\n');
    return 0;
}