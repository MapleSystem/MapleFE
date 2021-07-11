class Klass {
  public delete(s: string) {
    console.log("delete " + s);
  }
}

var obj: Klass = new Klass();
obj.delete("key");
