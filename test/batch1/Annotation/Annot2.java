@interface testInter {
    String str();
}

public class Annot2 {

    @testInter(str = "null") void m() {
    }

    public static void main(String argv[])   {
        System.out.println(run());
    }

    public static int run() {
        return 0;
    }
}

