/*
  Get the Unicode point of given index
*/
public class UnicodePt {
  public static void main(String[] args) {
    String str = "Huawei";
    System.out.println(str);
   
    int x = str.codePointAt(1);
    int y = str.codePointAt(3);
    int z = str.codePointAt(5);

    System.out.println("Unicode point at Index 1 = " + x);
    System.out.println("Unicode point at Index 3 = " + y);
    System.out.println("Unicode point at Index 5 = " + z);
  }
}
