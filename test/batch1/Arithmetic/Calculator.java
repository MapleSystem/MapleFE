/*
  Subtraction and Division on two integers
*/
public class Calculator {
  public static int sub(int p1, int p2) {
    return p1 - p2;
  }

  // Division for integer values
  public static int intdiv(int p1, int p2) {
    if (p2 == 0) {
      throw new IllegalArgumentException("Cannot divide by 0.");
    }
    return p1 / p2;
  }

  public static void main(String[] args) {
    int x, y, sum_sub, sum_div;
    x = 100;
    y = 50;

    sum_sub = sub(x, y);
    System.out.println(x + " - " + y + " = " + sum_sub);

    sum_div = intdiv(x, y);
    System.out.println(x + " / " + y + " = " + sum_div);
  }
}
