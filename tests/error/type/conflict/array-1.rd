// test: Compiler should find a type conflict
// stderr: = error[E0008]: 5:21-29: type error, conflicting types `[2]int` and `[3]int`\n

pub fn foo(): int {
    let a: [2]int = [5, 6, 7];
    return 0;
}
