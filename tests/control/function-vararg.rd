// test: Should compile variadic function

fn bar(a: int, b: int, ..): int {
    return a + b;
}

pub fn main(): bool {
    return bar(1, 2, 3, 4, "Hello", 5.6, true) != 3;
}

