// Find sorted count
import java.util.*;
import java.lang.*;
import java.io.*;

public class ArrSort3 {
  static int count_sort(int arr[], int len) {
    int min_value = arr[0];
    int min_index = -1;

    for (int x = 0; x < len; x++) {
      if (min_value > arr[x]) {
        min_value = arr[x];
        min_index = x;
      }
    }
    return min_index;
  }

  public static void main(String[] args) {
    int array1[] = {90, 80, 70, 10, 20, 30, 40, 50, 60};
    int array2[] = {80, 70, 10, 20, 30, 40, 50, 60};
    int array3[] = {70, 10, 20, 30, 40, 50, 60};

    int len1 = array1.length;
    int len2 = array2.length; 
    int len3 = array3.length;

    System.out.println("Array 1 : " + Arrays.toString(array1));
    System.out.println("Sorted : " + count_sort(array1, len1) + "\n");

    System.out.println("Array 2 : " + Arrays.toString(array2));
    System.out.println("Sorted : " + count_sort(array2, len2) + "\n");

    System.out.println("Array 3 : " + Arrays.toString(array3));
    System.out.println("Sorted : " + count_sort(array3, len3) + "\n");
  }
}
