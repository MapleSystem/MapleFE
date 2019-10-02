// Look for a specific value in the array
// If found then return true, else, return false
import java.util.Arrays;

public class ArrSearch {
  public static boolean found(int[] arr, int y) {
    for (int z : arr) {
      if (y == z) {
        return true;
      }
    }
    return false;
  }

  public static void main(String[] args) {
    int[] array1 = {2019, 2014, 2015, 2010, 2016,
                    2012, 2017, 2011, 2013, 2018};

    System.out.println(Arrays.toString(array1));

    // Pass a value to found function, if value found in array then return true
    // else return false
    System.out.println("2010 found? " + found(array1, 2010));
    System.out.println("2020 found? " + found(array1, 2020));
    System.out.println("2017 found? " + found(array1, 2017));
  }
}
