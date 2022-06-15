// test: Compiler should find a type conflict
// stderr: = error[E0011]: 6:13: type error, incompatible type `[2]int` for equal expession, must be a numeric value, boolean or pointer\n

pub fn foo(a: [2]int) {
    let b = a;
    let c = b == a;
}
