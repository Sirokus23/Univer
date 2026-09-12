import math

def power_log_n(a, n, counter, iteratii): 
    rezultat = 1
    baza = a
    while n > 0:
        iteratii[0] += 1
        counter[0] += 1  
        counter[0] += 1  
        if n % 2 == 1:
            rezultat *= baza
            counter[0] += 1  

        baza *= baza          
        n //= 2                
        counter[0] += 2
    counter[0] += 1  
    return rezultat


def fib1(n, counter, apeluri):  
    apeluri[0] += 1
    if n < 2:
        counter[0] += 1
        return n
    else:
        counter[0] += 4
        return fib1(n - 1, counter, apeluri) + fib1(n - 2, counter, apeluri)


def fib2(n, counter, iteratii):  
    i = 1
    j = 0
    for _ in range(0, n):
        j = i + j
        i = j - i
        counter[0] += 4
        iteratii[0] += 1
    counter[0] += 1
    return j, counter[0]

def fib3(n, counter, iteratii):  
    i = 1
    j = 0
    k = 0
    h = 1
    while n > 0:
        iteratii[0] += 1
        counter[0] += 1 
        counter[0] += 1  
        if n % 2 != 0:
            t = j * h
            j = i * h + j * k + t
            i = i * k + t
            counter[0] += 7

        t = h ** 2
        h = 2 * k * h + t
        k = k ** 2 + t
        n = n // 2
        counter[0] += 6
    counter[0] += 1  
    return j, counter[0]

def fib4(n, counter, iteratii):  
    radical5 = math.sqrt(5)
    counter[0] += 1  

    fi = (1 + radical5) / 2
    psi = (1 - radical5) / 2
    counter[0] += 4  

    pow_fi = power_log_n(fi, n, counter, iteratii)
    pow_psi = power_log_n(psi, n, counter, iteratii)

    diff = (pow_fi - pow_psi) / radical5
    counter[0] += 2  

    rezultat = round(diff)

    return rezultat, counter[0]


def main():
    print("====Meniu====")
    print("1. Fibonacci recursiv")
    print("2. Fibonacci iterativ")
    print("3. Fibonacci dublare rapidă")
    print("4. Formula lui Binet")
    alegere = int(input("Introduceti nr: "))

    if alegere == 1:
        n = int(input("Al câtelea nr Fibonacci: "))
        counter = [0]
        apeluri = [0]
        valoare = fib1(n, counter, apeluri)
        print(f"n={n}  Apeluri={apeluri[0]}  Nop={counter[0]}  Fibonacci(n)={valoare}")

    elif alegere == 2:
        n = int(input("Al câtelea nr Fibonacci: "))
        counter = [0]
        iteratii = [0]
        result, nop = fib2(n, counter, iteratii)
        print(f"n={n}  Iterații={iteratii[0]}  Nop={nop}  Fibonacci(n)={result}")

    elif alegere == 3:
        n = int(input("Al câtelea nr Fibonacci: "))
        counter = [0]
        iteratii = [0]
        result, nop = fib3(n, counter, iteratii)
        print(f"n={n}  Iterații={iteratii[0]}  Nop={nop}  Fibonacci(n)={result}")

    elif alegere == 4:
        n = int(input("Al câtelea nr Fibonacci: "))
        counter = [0]
        iteratii = [0]
        result, nop = fib4(n, counter, iteratii)
        print(f"n={n}  Iterații={iteratii[0]}  Nop={nop}  Fibonacci(n)={result}")


main()