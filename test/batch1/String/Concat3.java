/*
  Concatenate two strings
*/
public class Concat3 {
  public static void main(String[] args) {
    String x = "Huawei";
    String y = "#1";

    System.out.println(x);
    System.out.println(y);

    String z = x.concat(' ' + y);

    System.out.println("Concatenated result : " + z);
  }
}
