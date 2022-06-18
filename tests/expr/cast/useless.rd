// test: Conversions should be possible

pub fn main(): int {
    let a: [2]int = [1, 2];
    let b: [2]int = a as [2]int;
    if b[0] != 1 || b[1] != 2 {
        return 1;
    }
    return 0;
}
