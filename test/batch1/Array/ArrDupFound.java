import java.util.Arrays;

public class ArrDupFound {
  public static void main(String[] args) {
    int[] array1 = {2019, 2014, 2015, 2010, 2016,
                    2012, 2017, 2011, 2014, 2018};

    System.out.println(Arrays.toString(array1));

    for (int x = 0; x < array1.length - 1; x++) {
      for (int y = x + 1; y < array1.length; y++) {
        if ((array1[x] == array1[y]) && (x != y)) {
          System.out.println("Duplicate value " + array1[y] + " found!");
        }
      }
    }
  }
}
