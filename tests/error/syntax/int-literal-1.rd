// test: File should not compile with syntax error
// stderr: = error[E0001]: 5:16-18: syntax error, unexpected identifier, expecting `;`\n

pub fn foo(): int {
    return 0x1az23;
}
