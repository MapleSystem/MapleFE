/*
  Get the unicode point before the giving index
*/
public class UnicodePtB4  {
  public static void main(String[] args) {
    String str = "Huawei";
    System.out.println(str);

    int x = str.codePointBefore(1);
    int y = str.codePointBefore(3);
    int z = str.codePointBefore(5);

    System.out.println("Unicode point before Index 1 = " + x);
    System.out.println("Unicode point before Index 3 = " + y);
    System.out.println("Unicode point before Index 5 = " + z);
  }
}
