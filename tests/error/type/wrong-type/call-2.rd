// test: Compiler should find a type conflict
// stderr: = error[E0008]: 10:5: type error, incompatible type `&fn (int, int, int): int` for call expession, must be a function\n

fn test(a: int, b: int, c: int): int {
    return a + b + c;
}

pub fn foo() {
    let a = &test;
    a(1, 2, 3);
}
