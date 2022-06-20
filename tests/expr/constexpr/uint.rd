// test: Test constant expressions with booleans

pub fn main(): int {
    let a: [3]int;
    let b: [1 as uint + 2]int = a;
    let c: [7 as uint - 4]int = a;
    let d: [7 as uint * 4 / 8]int = a;
    let e: [7 as uint % 4]int = a;
    let f: [1 as uint | 2]int = a;
    let g: [3 as uint & 7]int = a;
    let h: [19 as uint ^ 16]int = a;
    return 0;
}
