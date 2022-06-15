// test: Recursive fibonacci implementation

fn fib(n: uint): uint {
    if n <= 1 {
        return n;
    } else {
        return fib(n - 1) + fib(n - 2);
    }
}

pub fn main(): int {
    if fib(10) != 55 {
        return 1;
    }
    if fib(20) != 6765 {
        return 2;
    }
    if fib(30) != 832040 {
        return 3;
    }
    return 0;
}
