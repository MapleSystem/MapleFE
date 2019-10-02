// Compare two arrays whether they are the same or not
import java.util.Arrays;

public class ArrSame {
  static void value_same(int[] arr1, int[] arr2) {
    boolean same = true;

    if (arr1.length == arr2.length) {
      for (int x = 0; x < arr1.length; x++) {
        if (arr1[x] != arr2[x]) {
          same = false;
        }
      }
    } else {
      same = false;
    }
 
    if (same) {
      System.out.println("===== They are the same! =====" + "\n");
    } else {
      System.out.println("===== They are different! =====" + "\n");
    }
  }

  public static void main(String[] args) {
    int[] array1 = {2019, 2014, 2015, 2010, 2016};
    int[] array2 = {2019, 2014, 2018, 2010, 2016};
    int[] array3 = {2019, 2014, 2015, 2010, 2016};
    int[] array4 = {2019, 2014, 2018, 2010, 2016};
    int[] array5 = {2019, 2014, 2018, 2010};

    System.out.println(Arrays.toString(array1));
    System.out.println(Arrays.toString(array2));
    value_same(array1, array2);

    System.out.println(Arrays.toString(array1));
    System.out.println(Arrays.toString(array3));
    value_same(array1, array3);

    System.out.println(Arrays.toString(array2));
    System.out.println(Arrays.toString(array4));
    value_same(array2, array4);

    System.out.println(Arrays.toString(array2));
    System.out.println(Arrays.toString(array5));
    value_same(array2, array5);
  }
}
