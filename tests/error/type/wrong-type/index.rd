// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:5: type error, incompatible type `f64` for index expession, must be an array or pointer\n

pub fn foo(a: f64) {
    a[0];
}
