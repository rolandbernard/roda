// test: File should not compile with syntax error
// stderr: = error[E0001]: 5:13-17: syntax error, unexpected identifier, expecting `;`\n

pub fn foo(): int {
    return 0a2e12;
}
