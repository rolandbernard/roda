// test: Conversions should be possible

pub fn main(): int {
    let a: u64 = 12;
    let b: u8 = 12;
    if a as u8 != b {
        return 1;
    }
    if a != b as u64 {
        return 2;
    }
    let c: i8 = -244;
    if b != c as u8 {
        return 3;
    }
    if b as i8 != c {
        return 3;
    }
    return 0;
}
