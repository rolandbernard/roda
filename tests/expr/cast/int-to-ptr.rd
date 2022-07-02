// test: Conversions should be possible

pub fn main(): int {
    let a = 0 as &();
    let b = 0 as &();
    if a != b {
        return 1;
    }
    let c = 42 as &();
    let d = 42 as &();
    if c != d {
        return 2;
    }
    return 0;
}
