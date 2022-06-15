// test: File should not compile with syntax error
// stderr: = error[E0001]: 5:18: syntax error, unexpected `;`\nerror[E0001]: 6:23: syntax error, unexpected `)`\n

pub fn foo(): int {
    let a = 12 + ;
    let b = (12 + 4 - );
}
