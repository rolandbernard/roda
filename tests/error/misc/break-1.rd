// test: Break is not allowed here
// stderr: = error[E0022]: 8:5-9: no target for break expression\n

pub fn foo(a: int): int {
    while a < 10 {
        a += 1;
    }
    break;
    while a < 100 {
        a += 1;
    }
    return 0;
}
