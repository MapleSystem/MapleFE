// Define generic element pool using array
class Pool<T> {
  private _pool: T[] = [];

  public put(element: T) {
    this._pool.push(element);
  }
  public get(): T | undefined{
    return this._pool.pop();
  }
  public size(): number {
    return this._pool.length;
  }
}

class Foo {
  public _id: number;

  constructor(id: number) {
    this._id = id;
  }
}

// Create array of primary type values
const primPool = new Pool<number>();
primPool.put(10);
console.log(primPool.get());

// Create array of objects
const objPool = new Pool<Foo>();
objPool.put(new Foo(100));
console.log(objPool.get()?._id);
