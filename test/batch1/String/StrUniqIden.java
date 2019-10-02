// Create a unique identifier of a given string
public class StrUniqIden {
  public static void main(String[] args) {
    String x = "Hello World.";
   
    // Get the hash code of the given string x
    int hash_code = x.hashCode();
  
    System.out.println("The hash code for \"" + x + "\"" + " is " + hash_code);
  }
}
