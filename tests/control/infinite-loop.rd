// test: An infinite loop should never return
// timesout: true
// time: < 100ms

pub fn main(): int {
    let i = 0;
    while true {
        i += 1;
    }
    return 1;
}
