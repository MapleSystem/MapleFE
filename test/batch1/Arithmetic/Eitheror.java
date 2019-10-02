// Display true if the sum is equal to 18, returns false if not
import java.util.*;

public class Eitheror {
  public static void main(String[] args) {
    Scanner in = new Scanner(System.in);
    System.out.print("Input the 1st integer : ");
    int x = in.nextInt();
    System.out.print("Input the 2nd integer : ");
    int y = in.nextInt();
    System.out.print("Is the value or sum equal to 18\n");
    System.out.print("The resule is : " + calc(x, y));
    System.out.printf("\n");
  }
  
  public static boolean calc(int x, int y) {
    if (x == 18 || y == 18) 
      return true;

    return ((x + y) == 18 || Math.abs(x - y) == 18);
  }
}
