// Using radix sort to sort the given array from low to high indexes
import java.util.Arrays;

public class Sort3 {
  public static void sort(int[] arr1) {
    Sort3.sort(arr1, 10);
  } 

  public static void sort(int[] arr1, int radix) {
    if (arr1.length == 0) {
      return;
    }
   
    int minVal = arr1[0];
    int maxVal = arr1[0];
    for (int x = 1; x < arr1.length; x++) {
      if (arr1[x] < minVal) {
        minVal = arr1[x];
      } else if (arr1[x] > maxVal) {
        maxVal = arr1[x];
      }
    }

    int exp = 1;
    while ((maxVal - minVal) / exp >= 1) {
      Sort3.countingSort(arr1, radix, exp, minVal);
      exp *= radix;
    }
  } 
 
  private static void countingSort(int[] arr1, int radix, int exp, int minVal) {
    int bucketIdx;
    int[] buckets = new int[radix];
    int[] out = new int[arr1.length];

    for (int x = 0; x < radix; x++) {
      buckets[x] = 0;
    }

    for (int x = 0; x < arr1.length; x++) {
      bucketIdx = (int)(((arr1[x] - minVal) / exp) % radix);
      buckets[bucketIdx]++;
    }

    for (int x = 1; x < radix; x++) {
      buckets[x] += buckets[x - 1];
    }
  
    for (int x = arr1.length -1; x >=0; x--) {
      bucketIdx = (int)(((arr1[x] - minVal) / exp) % radix);
      out[--buckets[bucketIdx]] = arr1[x];
    }
  
    for (int x =0; x < arr1.length; x++) {
      arr1[x] = out[x];
    }
  }

  public static void main(String args[]) {
    Sort3 s = new Sort3();
    int nums[] = {6, -4, 2, 1, 0, 44};
    System.out.println("Array 1 : ");
    System.out.println(Arrays.toString(nums));
    s.sort(nums);
    System.out.println("Sorted : ");
    System.out.println(Arrays.toString(nums));
  }
}
