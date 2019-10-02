// Round a float number to 3 decimals
import java.lang.*;
import java.math.BigDecimal;

public class Round1 {
  public static void main(String[] args) {
    float x = 123.4f;
    BigDecimal res;
    int decimal = 3;
    BigDecimal bigdec = new BigDecimal(Float.toString(x));
    bigdec = bigdec.setScale(decimal, BigDecimal.ROUND_HALF_UP);
   
    System.out.printf("Origianl number : %.7f\n", x);
    System.out.println("Rounded up to 4 decimal : " + bigdec);
  }
}
