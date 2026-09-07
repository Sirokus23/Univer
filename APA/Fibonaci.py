import math
from decimal import Decimal

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

def fib4(n):
    root5 = Decimal(5).sqrt()
    fi = (Decimal(1) + root5)/Decimal(2)
    psi = (Decimal(1) - root5)/Decimal(2)
    return round((fi**n - psi**n)/root5)

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
        print(valoare, counter[0])
    elif alegere == 2:
        n = int(input("Al citalea nr fibbonaci: "))
        counter = [0]
        result, nop = fib2(n, counter)
        print(result, nop)
    elif alegere == 3:
        n = int(input("Al citalea nr fibbonaci: "))
        counter=[0]
        result, nop = fib3(n, counter)
        print(result, nop)
    elif alegere == 4:
        n = int(input("Al citalea nr fibbonaci: "))
        result = fib4(n)
        print(result)

main()