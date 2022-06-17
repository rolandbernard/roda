// test: Compiler should find a type conflict
// stderr: = error[E0008]: 6:13: type error, incompatible type `f64` for shift right expession, must be an integer value\n

pub fn foo(a: f64) {
    let b = 5.5;
    let c = b >> a;
}
