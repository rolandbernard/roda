// test: Nested functions, constants and globals can be shadow local variables

fn foo(): int {
    let a = 20;
    let b = 30;
    let fizz = 5;
    {
        const a: int = 5;
        static b: int = 10;
        fn fizz(): int {
            return a + b;
        }
        return fizz();
    }
}
const A: int = 10;

pub fn main(): int {
    if foo() != 15 {
        return 1;
    }
    return 0;
}

