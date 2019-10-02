// Find the 2nd largest value in the integer string
import java.util.Arrays;

public class Arr2ndLargest {
  public static void main(String[] args) {
    int[] array1 = {2019, 2014, 2015, 2010, 2016,
                    2012, 2017, 2011, 2013, 2018};

    System.out.println("Original : " + Arrays.toString(array1));
    Arrays.sort(array1);
    int idx = array1.length - 1;
    while (array1[idx] == array1[array1.length - 1]) {
      idx--;
    }
    System.out.println("2nd largest value : " + array1[idx]);
  }
}
