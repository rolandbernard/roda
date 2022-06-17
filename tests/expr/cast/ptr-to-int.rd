// test: Conversions should be possible

pub fn main(): int {
    let a = 12;
    let b: usize = *a as usize;
    if *a as usize != b {
        return 1;
    }
    let c: [2]int;
    let d = *c[1] as usize - *c[0] as usize;
    if d != sizeof int {
        return d as int;
    }
    return 0;
}
