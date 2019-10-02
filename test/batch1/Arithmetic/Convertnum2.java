// Convert a float number to an absolute number
import java.util.*;

public class Convertnum2 {
  public static void main(String[] args) {
    Scanner in = new Scanner(System.in);
    System.out.print("Input a float number : ");
    float f = in.nextFloat();
    System.out.printf("The absolute value of %.2f is %.2f", f, convert(f));
    System.out.println("\n");
  }
  
  public static float convert(float f) {
    float abs = (f >= 0) ? f : -f;
    return abs;
  }  
}
