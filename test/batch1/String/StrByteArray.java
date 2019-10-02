// Get the byte array of given string
public class StrByteArray {
  public static void main(String[] args) {
    String x = "Hello World.";
 
    // Get the byte array of the given string x
    byte[] byte_array = x.getBytes();

    // New string using the contents of the byte array
    String y = new String(byte_array);   

    System.out.println("New string is \"" + y + "\"" + "\n");
  }
}
