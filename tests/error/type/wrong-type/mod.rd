// test: Compiler should find a type conflict
// stderr: = error[E0011]: 6:13: type error, incompatible type `f64` for modulo expession, must be an integer value\n

pub fn foo(a: f64) {
    let b = 5.5;
    let c = b % a;
}
