// Remove the index element from array
import java.util.Arrays;

public class ArrRemoveItem {
  public static void main(String[] args) {
    int[] array1 = {2019, 2014, 2015, 2010, 2016,
                    2012, 2017, 2011, 2013, 2018};

    System.out.println(Arrays.toString(array1));

    int idxr = 0;

    for (int x = idxr; x < array1.length -1; x++) {
      array1[x] = array1[x + 1];
    }
    
    System.out.println("After removing index 7 : " + Arrays.toString(array1));
  }
}
