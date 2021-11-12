enum ET {
    type = "type",
    with = "with"
}
abstract class Base<U extends ET, V extends ET> { f1: U | null = null; f2: V | null = null; }
declare class Klass extends Base<ET.type, ET.with> {}

