// test: Test moving around structs

pub fn main(): int {
    let a: (a: i8, b: f64,) = (b = 4.25, a = 5);
    if a.a != 5 || a.b != 4.25 {
        return 1;
    }
    let b: (a: i8, b: f64) = (a = 5, b = 4.25,);
    if b.a != 5 || b.b != 4.25 {
        return 2;
    }
    let c: (b: i8, a: f64,) = (a = 4.25, b = 5,);
    if c.b != 5 || c.a != 4.25 {
        return 3;
    }
    let d: (b: i8, a: f64) = (b = 5, a = 4.25);
    if d.b != 5 || d.a != 4.25 {
        return 4;
    }
    let e: (a: f64, b: i8,) = (b = 5, a = 4.25,);
    if e.b != 5 || e.a != 4.25 {
        return 5;
    }
    return 0;
}
