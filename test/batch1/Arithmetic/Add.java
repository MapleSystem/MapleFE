/*
  Simple Addition on two integers
*/
public class Add {
  public static int add (int p1, int p2) {
    int sum = p1 + p2;

    return sum;
  }

  public static void main(String[] args) {
    int x = 100;
    int y = 50;

    int sum = add(x, y);

    System.out.println(x + " + " + y + " = " + sum);
  }
}
