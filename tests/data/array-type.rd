// test: Test moving around array type data

fn foo(a: [5]int, b: int, c: int): [5]int {
    a[c] += b;
    return a;
}

pub fn main(): int {
    let x: [0]int = ();
    let a: [5]int;
    a[0] = 1;
    a[1] = 2;
    a[2] = 3;
    a[3] = 4;
    a[4] = 5;
    let b = a[3];
    let c = a[1];
    let d = foo(a, b, c);
    if d[0] != 1 || d[1] != 2 || d[2] != 7 || d[3] != 4 || d[4] != 5 {
        return 1;
    } else {
        return 0;
    }
}
