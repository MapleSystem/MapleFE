/*
  Count the number of Unicode point in specific text range
*/
public class UnicodePtRange {
  public static void main(String[] args) {
    String str = "Huawei";
    System.out.println(str);

    int range = str.codePointCount(1, 5);

    System.out.println("Unicode Point count = " + range);
  }
}
