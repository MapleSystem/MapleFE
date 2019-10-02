@interface testInter {
    int id1();
    int id2();
    int id3();
}
 
public class Annot5 {

    @testInter(id1 = 123, id2 = 124, id3 = 125) public void m(){
    }

    public static void main(String argv[])   {
        System.out.println(run());
    }

    public static int run() {
        return 0;
    }
}

