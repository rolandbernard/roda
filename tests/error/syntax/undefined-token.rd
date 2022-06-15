// test: File should not compile with syntax error
// stderr: = error[E0001]: 5:16: syntax error, unexpected invalid token\n

pub fn foo(): int {
    return 0 + $;
}

