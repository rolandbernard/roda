// test: File should not compile with syntax error
// stderr: = error[E0001]: 5:25: syntax error, unexpected `;`\n

pub fn foo(): int {
    return ((1 + 3) - (5;
}

