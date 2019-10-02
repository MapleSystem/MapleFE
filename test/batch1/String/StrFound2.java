/*
  Whether original string contains the given string/world
*/
public class StrFound2 {
  public static void main(String[] args) {
    String x = "What a wonderful world";
    String y = "world";

    System.out.println("String : " + x);
    System.out.println("Does \"" + x + "\" contain \"" + y + "\"?");
    System.out.println("Result : " + x.contains(y));
  }
}
