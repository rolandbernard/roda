// test: Run while block until condition is false
// stdout: = 10

extern fn printf(fmt: &u8, ..);

pub fn main(): bool {
    let i = 0;
    while i < 10 {
        i += 1;
    }
    printf("%i", i as i32);
    return i != 10;
}
