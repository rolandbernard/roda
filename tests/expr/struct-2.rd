// test: Test moving around structs

type MyStruct = (a: [2]int, b: f32);

fn foo(a: MyStruct, b: int, c: int): (a: int, b: f64) {
    a.a[c] += b;
    let d;
    d.a = a.a[c];
    d.b = a.b as f64;
    return d;
}

pub fn main(): int {
    let a;
    a.a = [1, 2];
    a.b = 2.25;
    let b = foo(a, 5, 1).b;
    if b != 2.25 {
        return 1;
    }
    return 0;
}
