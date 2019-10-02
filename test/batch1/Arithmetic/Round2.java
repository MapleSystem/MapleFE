// Round a float number into integer number
import java.util.*;

public class Round2 {
  public static void main(String[] args) {
    Scanner in = new Scanner(System.in);
    System.out.print("Input a float number : ");
    float f = in.nextFloat();
    System.out.printf("The round number of %f is %.2f", f, round(f));
    System.out.printf("\n");
  }
  
  public static float round(float f) {
    float fn = (float)Math.floor(f);
    float cn = (float)Math.ceil(f);
    if ((f - fn) > (cn - f)) {
      return cn;
    } else if ((cn - f) > (f - fn)) {
      return fn;
    } else {
      return cn;
    }
  }
}
