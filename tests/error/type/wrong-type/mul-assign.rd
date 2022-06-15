// test: Compiler should find a type conflict
// stderr: = error[E0011]: 6:5: type error, incompatible type `bool` for mul assign expession, must be a numeric value\n

pub fn foo(a: bool) {
    let b = false;
    b *= a;
}
