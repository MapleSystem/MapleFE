// Find how many even numbers and odd numbers
import java.util.Arrays;

public class ArrEvenOdd {
  public static void main(String[] args) {
    int[] array1 = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    System.out.println("Original : " + Arrays.toString(array1));
  
    int cnt = 0;
    for (int x = 0; x < array1.length; x++) {
      if (array1[x] % 2 == 0) {
        cnt++;
      }
    } 
    System.out.println("How many EVEN numbers? " + cnt);
    System.out.println("How many ODD numbers? " + (array1.length - cnt)); 
  }
}
