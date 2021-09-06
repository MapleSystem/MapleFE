type Constructor<T = {}> = new (...args: any[]) => T;
interface someStandardInterface {}
function ClassFactory<TBase>(
  base: Constructor<TBase>
): Constructor<TBase & someStandardInterface> {
  class GeneratedClass extends (base as unknown as any) {}
  return GeneratedClass as unknown as any;
}
class someBaseClass {}
class newClassWithStandardInterface extends ClassFactory(someBaseClass) {}
