// test: Compiler should find a type conflict
// stderr: = error[E0008]: 6:13: type error, incompatible type `bool` for multiply expession, must be a numeric value\n

pub fn foo(a: bool) {
    let b = false;
    let c = b * a;
}
