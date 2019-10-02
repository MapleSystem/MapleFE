// This is to find all of the prime numbers between 2 and 100.
class PrimNum100 {
  public static void main(String args[]) {
    int n;
    n = 3;
    // use n%2, 3,5,7 to find out prime numbers 
    if((n%2)!=0 && (n%3)!=0 && (n%5)!=0 && (n%7)!=0 && n<=100)
      System.out.println( n + " is a prime number.");
    n++;
    if((n%2)!=0 && (n%3)!=0 && (n%5)!=0 && (n%7)!=0 && n<=100)
      System.out.println( n + " is a prime number.");
  }
}

