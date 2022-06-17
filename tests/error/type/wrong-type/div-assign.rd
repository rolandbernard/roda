// test: Compiler should find a type conflict
// stderr: = error[E0008]: 6:5: type error, incompatible type `bool` for div assign expession, must be a numeric value\n

pub fn foo(a: bool) {
    let b = false;
    b /= a;
}
