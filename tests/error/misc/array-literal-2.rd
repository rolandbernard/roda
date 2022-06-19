// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:21-26: type error, conflicting types `[3]int` and `[2]int`\n

pub fn foo() {
    let a: [3]int = [0, 5];
}
