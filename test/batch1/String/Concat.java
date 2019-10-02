/*
  Concatenate two words and convert them to uppercase letters
*/
public class Concat {
  public static String concat(boolean toappend, String p1, String p2) {
    String result = null;
    if (toappend) {
      result = p1 + " " + p2;
    }
    return result.toUpperCase();
  }

  public static void main(String[] args) {
    String output = concat(true, "heLlo", "wOrlD");
    System.out.println("Concatenate result is " + output);
  }
}
