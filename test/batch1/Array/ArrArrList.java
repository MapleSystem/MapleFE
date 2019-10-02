// Make an array to an ArrayList
import java.util.ArrayList;
import java.util.Arrays;

public class ArrArrList {
  public static void main(String[] args) {
    String[] array1 = {"Test1", "Test6", "Test3", "Test8",
                       "Test5", "Test2", "Test7", "Test4"};

    ArrayList<String> l = new ArrayList<String>(Arrays.asList(array1));

    System.out.println(l);
  }
}
