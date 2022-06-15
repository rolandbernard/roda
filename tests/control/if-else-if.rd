// test: Allow chaining of if-else expressions

pub fn main(): int {
    if false {
        return 3;
    } else if true {
        if false {
            return 2;
        } else if false {
            return 4;
        } else {
            return 0;
        }
    } else {
        return 1;
    }
}
