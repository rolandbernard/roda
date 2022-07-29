// test: Nested functions can not access outer function variables
// stderr: = error[E0009]: 6:22: variable expression not allowed in constant expressions\n

pub fn foo(): int {
    let a = 100;
    const bar: int = a;
    return bar;
}
