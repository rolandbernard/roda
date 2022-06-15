// test: File should not compile with syntax error
// stderr: = error[E0001]: 5:17-19: syntax error, unexpected integer, expecting `;`\n

pub fn foo(): int {
    return 0b101201;
}
