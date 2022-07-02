// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:14: type error, incompatible type `int` for dereference expession, must be a pointer\n

pub fn foo(a: int) {
    let b = *a;
}
