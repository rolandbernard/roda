// test: Test moving around tuples

type MyTuple = ([2]int, f32, [4]int);

fn foo(a: MyTuple, b: int, c: int): (int, f64) {
    a.0[c] += b;
    return (a.0[c], a.1 as f64);
}

pub fn main(): int {
    let a = ([1, 2], 2.25, [1, 2, 3, 4]);
    let b = foo(a, 5, 1);
    if b.0 != 7 || a.1 != 2.25 {
        return 1;
    }
    return 0;
}
