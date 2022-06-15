// test:

pub fn main(): int {
    if false != false {
        return 1;
    }
    if !(true != false) {
        return 2;
    }
    if !(false != true) {
        return 3;
    }
    if true != true {
        return 4;
    }
    return 0;
}
