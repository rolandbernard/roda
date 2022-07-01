// test: Test constant block expressions

pub fn main(): int {
    let a: [3]int;
    let b: [{1 + 2}]int = a;
    let c: [{1 + 5; 5 - 2; {5 * 3; 7 - 4}}]int = a;
    let d: [{1; {2; {7 * 4 / 8}}}]int = a;
    return 0;
}
