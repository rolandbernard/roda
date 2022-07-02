// test: Break is not allowed here
// stderr: = error[E0022]: 5:13-20: no target for continue expression\n

pub fn foo(a: int): int {
    while { continue; a < 100 } {
        a += 1;
    }
    return 0;
}
