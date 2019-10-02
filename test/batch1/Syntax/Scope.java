/*
  Demonstrate block scope
*/
public class Scope {
  public static void main(String args[]) {
    int x = 10;
  
    if (x == 10) {
      int y = 20;

      System.out.println("x is " + x + " and y is " + y);
      x = y * 2;
    }
    // y = ?;   It will error out because Y is unknown outside if statement

    System.out.println("new value of x is " + x);
  }
}
