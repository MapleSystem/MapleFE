// Sort EVEN numbers to the left, ODD numbers to the right
import java.util.Arrays;

public class ArrSort8 {
  public static void main(String[] args) {
    int arr1[] = {10, 8, 9, 15, 7, 20, 2, 0, 1};
    int res[];

    System.out.println("Array : ");
    System.out.println(Arrays.toString(arr1));

    res = sorted(arr1);

    System.out.print("After sorting : ");
    System.out.println(Arrays.toString(arr1));
  }

  static int [] sorted(int arr1[]) {
    int left = 0, right = arr1.length - 1;

    while (left < right) {
      while (arr1[left]%2 == 0 && left < right)
        left++;

      while (arr1[right]%2 == 1 && left < right)
        right--;

       if (left < right) {
         int tmp = arr1[left];
         arr1[left] = arr1[right];
         arr1[right] = tmp;
         left++;
         right--;
       }
    }
    return arr1;
  }
}
