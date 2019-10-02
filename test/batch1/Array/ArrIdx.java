// Display the index of the value
// If found display then index value starts from 0, else, -1
import java.util.Arrays;

public class ArrIdx {
  public static int idx (int[] arr, int x) {
    if (arr == null) return -1;
    
    int l = arr.length;
    int y = 0;

    while (y < l) {
      if (arr[y] == x) {
        return y;
      } else {
        y = y + 1;
      }
    }
    return -1;
  }
  
  public static void main(String[] args) {
    int[] array1 = {2019, 2014, 2015, 2010, 2016,
                    2012, 2017, 2011, 2013, 2018};

    System.out.println(Arrays.toString(array1));
    System.out.println("Index of 2015 is : " + idx(array1, 2015));
    System.out.println("Index of 2019 is : " + idx(array1, 2019));
    System.out.println("Index of 2020 is : " + idx(array1, 2020));
    System.out.println("Index of 2011 is : " + idx(array1, 2011));
  }
}
