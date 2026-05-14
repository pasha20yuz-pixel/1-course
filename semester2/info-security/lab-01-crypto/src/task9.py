import random
import sys
import io
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

# ---------- Расширенный алгоритм Евклида ----------
def egcd(a, b):
    if b == 0:
        return (a, 1, 0)
    g, x1, y1 = egcd(b, a % b)
    return (g, y1, x1 - (a // b) * y1)

def modinv(a, m):
    g, x, _ = egcd(a, m)
    if g != 1:
        raise ValueError("Нет обратного элемента")
    return x % m

# ---------- Генерация ключей ----------
def generate_keys(block_size=8):
    priv = []
    total = 0
    for i in range(block_size):
        if i == 0:
            val = random.randint(1, 10)
        else:
            val = random.randint(total + 1, total * 2)
        priv.append(val)
        total += val

    m = total + random.randint(1, 100)          # модуль
    t = random.randint(2, m-1)
    while egcd(t, m)[0] != 1:
        t = random.randint(2, m-1)

    pub = [(t * w) % m for w in priv]
    return priv, pub, m, t

# ---------- Шифрование ----------
def encrypt(plain_text, pub_key):
    bits = []
    for ch in plain_text:
        byte = ord(ch)
        for i in range(7, -1, -1):
            bits.append((byte >> i) & 1)

    k = len(pub_key)
    while len(bits) % k != 0:
        bits.append(0)

    cipher = []
    for i in range(0, len(bits), k):
        block = bits[i:i+k]
        s = 0
        for j in range(k):
            if block[j]:
                s += pub_key[j]
        cipher.append(s)
    return cipher

# ---------- Дешифрование ----------
def decrypt(cipher, priv_key, m, t):
    t_inv = modinv(t, m)
    k = len(priv_key)
    bits = []

    for c in cipher:
        cp = (c * t_inv) % m
        block_bits = [0] * k
        remainder = cp
        for j in range(k-1, -1, -1):
            if remainder >= priv_key[j]:
                block_bits[j] = 1
                remainder -= priv_key[j]
        bits.extend(block_bits)

    bytes_list = []
    for i in range(0, len(bits), 8):
        if i + 8 > len(bits):
            break
        byte = 0
        for j in range(8):
            byte = (byte << 1) | bits[i + j]
        bytes_list.append(byte)
    return ''.join(chr(b) for b in bytes_list)

# ---------- Демонстрация ----------
priv, pub, m, t = generate_keys(8)
print("Закрытый ключ (сверхвозрастающая последовательность):", priv)
print("Открытый ключ:", pub)
print("Модуль m =", m)
print("Множитель t =", t)

message = "Hello, world!"
print("\nИсходное сообщение:", message)

cipher = encrypt(message, pub)
print("Зашифрованные числа:", cipher)

decrypted = decrypt(cipher, priv, m, t)
print("Расшифрованное сообщение:", decrypted)