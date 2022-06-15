// test: Should allow character literals

pub fn main(): int {
    if ('a' != 97) {
        return 1;
    }
    if ('\'' != 39) {
        return 2;
    }
    if 'ぁ' != 0x3041 {
        return 3;
    }
    if '\x41' != 0x41 {
        return 4;
    }
    if '\u3041' != 0x3041 {
        return 5;
    }
    if '\U12343041' != 0x12343041 {
        return 6;
    }
    if '\n' != 10 {
        return 7;
    }
    return 0;
}
