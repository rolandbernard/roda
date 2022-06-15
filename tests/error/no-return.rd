// test: Functions must return in all branches
// stderr: = error[E0016]: 4:1-1: function body reaches end of control flow without expected return\n

pub fn foo(a: int): int {
    if a == 0 {
        return 1;
    } else if a == 1 {
        a += 1;
    } else if a == 2 {
        return 2;
    } else {
        return 3;
    }
}
