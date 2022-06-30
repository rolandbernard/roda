// test: Test block expressions

pub fn main(): int {
    let a = {
        let b = 3;
        let c = 5;
        b + c
    };
    if a != 8 {
        return 1;
    }
    if { let a = 2*a; a } != 16 {
        return 2;
    }
    if { let a = 2*a; a } != { let a = 5*a; let b = a / 5; a - 3*b } {
        return 3;
    }
    return 0;
}
