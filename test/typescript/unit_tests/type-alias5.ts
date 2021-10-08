type U<T> = T extends unknown ? unknown extends T ? T extends true ? true : false : false : false;
