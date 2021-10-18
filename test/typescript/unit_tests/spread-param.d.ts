declare function func<T, U>( s1: T, s2: T | U, ...ss: Array< T | U | ((e: T | null) => void)>,): T;

