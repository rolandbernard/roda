// test: Conversions should be possible

pub fn main(): int {
    let a: u64 = 12;
    let b: f64 = 12.7;
    if a != b as u64 {
        return 1;
    }
    let c: i64 = -12;
    let d: f32 = -12.3;
    if c != d as i64 {
        return 2;
    }
    let e: u64 = 12;
    let f: f32 = 12.7;
    if e != f as u64 {
        return 3;
    }
    return 0;
}
