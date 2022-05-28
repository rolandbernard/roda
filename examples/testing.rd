
import fn print(text: *u8);

fn fma(a: uint, b: uint, c: uint): uint {
    while a < b {
        b += 1;
        a *= 2;
    }
    return a * b + c;
}

export fn main(argc: int, argv: **u8): int {
    print("Hello \t \"world\"!");
    print("😀");
    print(5.2E-2);
    print("\U0001F600");
    print(829343.545);
    print(829343);
    return 0;
}

