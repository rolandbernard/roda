// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:18-33: type error, expecting expression of type `int` but found struct literal\n

pub fn foo() {
    let a: int = (a = 0, c = 5.5);
}
