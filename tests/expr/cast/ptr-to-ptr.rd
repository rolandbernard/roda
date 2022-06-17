// test: Conversions should be possible

pub fn main(): int {
    let a: [2][2]int = [[1, 2], [3, 4]];
    let b = *a;
    let c = b as *int;
    if c[0] != 1 || c[1] != 2 || c[2] != 3 || c[3] != 4 {
        return 1;
    }
    return 0;
}
