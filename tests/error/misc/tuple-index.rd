// test: Compiler should find a type conflict
// stderr: = error[E0021]: 6:15: no field at index `2` in tuple type `(int, f64)`\n

pub fn foo() {
    let a: (int, f64);
    let b = a.2;
}
