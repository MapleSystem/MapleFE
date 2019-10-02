// Print reverse order of the given number

public class Reverse1 {
  public static void main(String[] args) {
    int n = 12345;
    int pos = 1;

    System.out.println("Original value : " + n);  
  
    if (n < 0) {
      pos = -1;
      n = -1 * n;
    }

    int s = 0;
    while (n > 0) {
      int r = n % 10;
   
      int maxD = Integer.MAX_VALUE - s * 10;
      if (s > Integer.MAX_VALUE / 10 || r > maxD)
        System.out.println("Wrong number");

      s = s * 10 + r;
      n /= 10;
    }
    System.out.println("Reversed value : " + pos * s);
  }
}
