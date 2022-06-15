// test: File should not compile with syntax error
// stderr: = error[E0001]: 5:23: syntax error, unexpected `)`, expecting `;`\n

pub fn foo(): int {
    return (1 + 3) - 5);
}

