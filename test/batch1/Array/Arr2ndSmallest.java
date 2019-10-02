// Find the 2nd smallest value in the integer array
import java.util.Arrays;

public class Arr2ndSmallest {
  public static void main(String[] args) {
    int[] array1 = {2019, 2014, 2015, 2010, 2016,
                    2012, 2017, 2011, 2013, 2018};

    System.out.println("Original : " + Arrays.toString(array1));

    int smallest = Integer.MAX_VALUE;
    int two_smallest = Integer.MAX_VALUE;

    for (int x = 0; x < array1.length; x++) {
      if (array1[x] == smallest) {
        two_smallest = smallest;
      } else if (array1[x] < smallest) {
        two_smallest = smallest;
        smallest = array1[x];
      } else if (array1[x] < two_smallest) {
        two_smallest = array1[x];
      }
    }
    System.out.println("2nd smallest value : " + two_smallest);
  }
}
