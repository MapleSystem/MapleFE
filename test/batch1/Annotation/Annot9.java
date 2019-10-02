@interface testMarker {}

@testMarker interface testInter {
}

@testMarker class testClass {
    @testMarker public int i;
    @testMarker int meth(){
        return 0;
    }
    @testMarker int methP(int i) {
        return i;
    }
    @testMarker testClass() {
    }
    @testMarker class testInnClass {
    }
    static @testMarker String str1 = "1", str2 = "2", str3 = "3";
    @testMarker enum testEnum {str1, str2, str3}
}

public class Annot9 {

    public static void main(String argv[])   {
        System.out.println(run());
    }

    public static int run() {
        return 0;
    }
}



