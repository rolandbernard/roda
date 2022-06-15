// test: Compiler should find a type conflict
// stderr: = error[E0011]: 5:14: type error, incompatible type `uint` for negative expession, must be a signed numeric value\n

pub fn foo(a: uint) {
    let c = -a;
}
