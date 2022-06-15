// test: Compiler should find a type conflict
// stderr: = error[E0011]: 6:5: type error, incompatible type `f64` for binary xor assign expession, must be an integer value\n

pub fn foo(a: f64) {
    let b = 5.5;
    b ^= a;
}
