// test: Break is not allowed here
// stderr: = error[E0022]: 5:13-17: no target for break expression\n

pub fn foo(a: int): int {
    while { break; a < 100 } {
        a += 1;
    }
    return 0;
}
