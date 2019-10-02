// Show the group of three indexes adds up to be total of 3
import java.util.ArrayList;
import java.util.List;
import java.util.*;

public class ArrCheck6 {
  public static void main(String[] args) {
    int[] array1 = {-1, 1, -5, 9, 0, 2};
    int total = 3;
    ArrCheck6 r = new ArrCheck6();
    System.out.println("Array 1 : " + Arrays.toString(array1));
    System.out.println("Which three indexes in Array 1 add up to be total of 3");
    System.out.println(r.Sum3(array1, total));
  }
  
  public List<List<Integer>> Sum3(int[] nums, int total) {
    List<List<Integer>> array2 = new ArrayList<List<Integer>>();
  
    for (int x = 0; x < nums.length; x++) {
      for (int y = x; y < nums.length; y++) {
        for (int z = y; z < nums.length; z++) {
          if (x != y && y != z && x != z && (nums[x] + nums[y] + nums[z] == total)) {
            List<Integer> List1 = new ArrayList<Integer>(3);
            List1.add(nums[x]);
            List1.add(nums[y]);
            List1.add(nums[z]);
            array2.add(List1);
          }
        }
      }
    }
    return array2;
  }
}
