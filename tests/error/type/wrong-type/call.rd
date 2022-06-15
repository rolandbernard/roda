// test: Compiler should find a type conflict
// stderr: = error[E0011]: 6:5: type error, incompatible type `int` for call expession, must be a function\n

pub fn foo() {
    let a: int;
    a(1, 2, 3);
}
