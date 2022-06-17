// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:14: type error, incompatible type `f64` for not expession, must be an integer or boolean value\n

pub fn foo(a: f64) {
    let c = !a;
}
