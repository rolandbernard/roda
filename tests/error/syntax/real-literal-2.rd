// test: File should not compile with syntax error
// stderr: = error[E0001]: 5:17-18: syntax error, unexpected identifier, expecting `;`\n

pub fn foo(): int {
    return 0.1e1a2;
}
