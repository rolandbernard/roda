// test: FizzBuzz should be printed
// stdout: != 

extern fn printf(fmt: &u8, ..);

pub fn main(): int {
    let i = 1;
    while i <= 100 {
        if i % 15 == 0 {
            printf("FizzBuzz\n");
        } else if i % 3 == 0 {
            printf("Fizz\n");
        } else if i % 5 == 0 {
            printf("Buzz\n");
        } else {
            printf("%li\n", i);
        }
        i += 1;
    }
    return 0;
}

