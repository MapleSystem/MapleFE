/*
  Use toCharArray() to print a string into single character array
*/
public class toCharArray {
  public static void main(String args[]) {
    String x = "Hello World";
    char[] y = x.toCharArray();

    int i;  
    for (i = 0; i < y.length; i++) {
      System.out.println(y[i]);
    }
  }
}
