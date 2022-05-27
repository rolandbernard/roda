
import fn print(text: *u8);

fn fma(a: uint, b: uint, c: uint): uint {
    return a * b + c;
}

export fn main(argc: int, argv: **u8): int {
    print("Hello \t \"world\"!");
    print(5.2E-2);
    return 0;
}

