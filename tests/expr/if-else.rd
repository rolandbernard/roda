// test: Test block expressions

pub fn main(): int {
    let a = if false {
        let b = 3;
        let c = 5;
        b + c
    } else {
        55
    };
    if a != 55 {
        return 1;
    }
    if if a == 55 { let a = 2*a; a } else {  let a = 10*a; a - 5 } != 110 {
        return 2;
    }
    if if a == 55 { 5 } else { 0 } != if a == 50 { 0 } else { 5 } {
        return 3;
    }
    return 0;
}
