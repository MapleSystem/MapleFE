/*
  Use getBytes() to get the sequence of bytes of the given character.
  It is also called byte array
*/
public class GetBytesArray {
  public static void main(String args[]) {
    String x = "APPLE";
    byte[] y = x.getBytes();
   
    int i;
    for (i = 0; i < y.length; i++) {
      System.out.println(y[i]);
    }
  }
}
