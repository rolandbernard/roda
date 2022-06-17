// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:18-23: type error, expecting expression of type `int` but found array literal\n

pub fn foo() {
    let a: int = [0, 5];
}
