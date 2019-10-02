// Display the Prime number of the input integer number
import java.util.*;

public class Primenum {
  public static void main(String[] args) {
    Scanner in = new Scanner(System.in);
    System.out.print("Input an integer : ");
    int x = in.nextInt();
    System.out.print("Prime number : " + Primes(x));
    System.out.printf("\n");
  }

  public static int Primes(int x) {
    if (x <= 0 || x ==1 || x ==2) {
      return 0;
    } else if (x == 3) {
      return 1;
    }

    BitSet set = new BitSet();
    x = x - 1;
    int y = (int)Math.sqrt(x);
    int mid = x;
    for (int z = 2; z <= y; z++) {
      if (!set.get(z)) {
        for (int a = 2; (z * a) <= x; a++) {
          if (!set.get(z * a)) {
            mid--;
            set.set(z * a);
          }
        }
      }
    }
    return mid - 1;
  }
}
