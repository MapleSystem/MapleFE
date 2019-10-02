// Sort the numberic and string array

import java.util.Arrays;

public class ArrSort {
  public static void main(String[] args) {
    int[] array1 = {2019, 2014, 2015, 2010, 2016,
                    2012, 2017, 2011, 2013, 2018};

    String[] array2 = {"Test1", "Test6", "Test3", "Test8",
                       "Test5", "Test2", "Test7", "Test4"};

    System.out.println("Original array 1 : " + Arrays.toString(array1));
    Arrays.sort(array1);
    System.out.println("Sorted array 1 : " + Arrays.toString(array1));

    System.out.println("Original array 2 : " + Arrays.toString(array2));
    Arrays.sort(array2);
    System.out.println("Sorted array 2 : " + Arrays.toString(array2));
  }  
}
