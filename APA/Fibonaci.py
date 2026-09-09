import math

def power_log_n(a, n, counter): #Functie ajutatoare pentru a calcula costul functiei pow in spatiul O(log n)
    rezultat = 1
    baza = a
    counter[0]+=1
    while n > 0:
        # Dacă ultimul bit al lui n este 1 (n este impar)
        counter[0]+=1 
        counter[0]+=1 
        if n % 2 == 1:
            rezultat *= baza
            counter[0]+=1 
        
        baza *= baza  # Ridicăm baza la pătrat
        n //= 2       # Împărțim n la 2 (sau n >>= 1)
        counter[0]+=2
    counter[0]+=1 
    return rezultat

def fib1(n, counter):  #recursiv
    if n < 2:
        counter[0]+=1
        return n
    else:
        counter[0]+=4
        return fib1(n-1, counter) + fib1(n-2, counter)

def fib2(n, counter):    #iterativ
    i = 1
    j = 0
    for _ in range(0,n):
        j = i + j
        i = j - i
        counter[0] += 4
    counter[0] += 1
    return j, counter[0]

def fib3(n, counter):
    i = 1
    j = 0
    k = 0
    h = 1
    counter[0] += 1
    while n > 0:
        counter[0]+=1
        counter[0]+=1
        if n % 2 != 0:
            t = j*h
            j = i * h + j*k + t
            i = i*k + t
            counter[0] += 7

        t = h**2
        h = 2 * k * h  + t
        k = k**2 + t
        n = n // 2
        counter[0] += 6
    counter[0]+=1
    return j, counter[0]

import math

def fib4(n, counter):
    radical5 = math.sqrt(5)
    counter[0]+=1
    fi = (1 + radical5) / 2
    psi = (1 - radical5) / 2
    counter[0]+=4
    pow_fi = power_log_n(fi, n, counter)
    pow_psi = power_log_n(psi, n, counter)

    diff = (pow_fi - pow_psi)/radical5
    counter[0]+=1

    rezultat = round(diff)
    counter[0]+=1

    return rezultat
    


def main():
    print("====Meniu====")
    print("1. Fibbonaci recursiv")
    print("2. Fibbonaci iterativ")
    print("3. Fibonacci dublare rapida")
    print("4. Formula lui Binet")
    alegere = int(input("Introduceti nr: "))

    if alegere == 1:
        n = int(input("Al citalea nr fibbonaci: "))
        counter = [0]
        valoare = fib1(n, counter)
        print(f"Nr Fibbonaci:{valoare} Nop: {counter[0]}")
    elif alegere == 2:
        n = int(input("Al citalea nr fibbonaci: "))
        counter = [0]
        result, nop = fib2(n, counter)
        print(f"Nr Fibbonaci:{result} Nop: {nop}")
    elif alegere == 3:
        n = int(input("Al citalea nr fibbonaci: "))
        counter=[0]
        result, nop = fib3(n, counter)
        print(f"Nr Fibbonaci:{result} Nop: {nop}")
    elif alegere == 4:
        n = int(input("Al citalea nr fibbonaci: "))
        counter = [0]
        result = fib4(n, counter)
        print(f"Nr Fibbonaci:{result} Nop: {counter[0]}")

main()