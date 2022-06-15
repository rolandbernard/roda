// test: File should not compile with syntax error
// stderr: = error[E0001]: 6:1: syntax error, unexpected `}`, expecting `;`\n

pub fn foo(): int {
    return 0
}

