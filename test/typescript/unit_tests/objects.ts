// different ways of creating an object

function func() {}

let funcObj = function (name: string) {
  this._name = name;
};

class ObjClass {
  private _name: string;
  constructor(name: string) {
    this._name = name;
  }
}

let obj_a: Object = {};
let obj_b: Object = Object.create({});
let obj_c: Object = Object.create(null);
let obj_d: Object = new Object();
let obj_e: Object = new func();
let obj_f: Object = new funcObj("John");
let obj_g: Object = new ObjClass("John");
