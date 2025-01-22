long f(long a) {
    return a + 1;
}

int main() {    
    short s = 3;
    long x = s;
    return f(x) + f(x);
}
