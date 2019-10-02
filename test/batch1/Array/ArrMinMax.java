// Display the minimum and maximum value of the numeric array
import java.util.Arrays;

public class ArrMinMax {
  static int min;
  static int max;

  public static void min_max(int arr[]) {
    min = arr[0];
    max = arr[0];
    int l = arr.length;
  
    for (int x = 1; x < l - 1; x = x + 2) {
      if (x + 1 > l) {
        if (arr[x] > max) {
          max = arr[x];
        }
        
        if (arr[x] < min) {
          min = arr[x];
        }
      }
   
      if (arr[x] > arr[x + 1]) {
        if (arr[x] > max) {
          max = arr[x];
        }
  
        if (arr[x + 1] < min) {
          min = arr[x + 1];
        }
      }
  
      if (arr[x] < arr[x + 1]) {
        if (arr[x] < min) {
          min = arr[x];
        }
    
        if (arr[x + 1] > max) {
          max = arr[x + 1];
        }
      }
    }
  }

  public static void main(String[] args) {
    int[] array1 = {2019, 2014, 2015, 2010, 2016,
                    2012, 2017, 2011, 2013, 2018, 2050};
  
    min_max(array1);

    System.out.println("Original : " + Arrays.toString(array1));
    System.out.println("Minimum value is : " + min);
    System.out.println("Maximum value is : " + max);
  }
}

