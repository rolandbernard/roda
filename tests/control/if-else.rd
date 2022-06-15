// test: Run if block if condition is true, otherwise the else block

pub fn main(): int {
    if true {
        if false {
            return 2;
        } else {
            return 0;
        }
    } else {
        return 1;
    }
}
