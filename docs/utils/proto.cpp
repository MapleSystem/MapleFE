#include "../../ast2cpp/runtime/include/ts2cpp.h"

using namespace t2crt;

class Ctor_Vehicle;
class Ctor_Car;
class Ctor_MyCar;

class Vehicle;
class Car;
class MyCar;

extern Ctor_Vehicle  Vehicle_ctor;
extern Ctor_Car      Car_ctor;
extern Ctor_MyCar    MyCar_ctor;

class Vehicle : public Object {
  public:
    Vehicle(Function* ctor, Object* proto): Object(ctor, proto) {}
    // Vehicle.prototype props (use static)

  public:
    // Vehicle instance props
    std::string name;
};

class Car : public Vehicle {
  public:
    Car(Function* ctor, Object* proto): Vehicle(ctor, proto) {}
    // Car.prototype props (use static)

  public:
    // Car instance props
};

// C++ Class def for instance props and prototype props
class MyCar : public Car {
  public:
    MyCar(Function* ctor, Object* proto): Car(ctor, proto) {}
    // MyCar.prototype props (use static)

  public:
    // MyCar instance props
};

// Class def for function constructors

class Ctor_Vehicle : public Function {
  public:
    Ctor_Vehicle(Function* ctor, Object* proto, Object* prototype) : Function(ctor, proto, prototype) {}

    Vehicle* _new() {
      return  new Vehicle(this, this->prototype);
    }

    void operator()(Vehicle* obj , std::string name) {
      // add instance props to instance prop list
      ClassFld<std::string Vehicle::*> field(&Vehicle::name);
      obj->AddProp("name", field.NewProp(TY_String));
      // init instance props
      obj->name = name;
    }
};

class Ctor_Car : public Ctor_Vehicle {
  public:
    Ctor_Car(Function* ctor, Object* proto, Object* prototype) : Ctor_Vehicle(ctor, proto, prototype) {}

    Car* _new() {
      return new Car(this, this->prototype);
    }

    void operator()(Car* obj , std::string name) {
      Vehicle_ctor(obj, name);
    }
};

class Ctor_MyCar : public Ctor_Car {
  public:
    Ctor_MyCar(Function* ctor, Object* proto, Object* prototype) : Ctor_Car(ctor, proto, prototype) {}

    MyCar* _new() {
      return new MyCar(this, this->prototype);
    }

    void operator()(MyCar* obj , std::string name) {
      Car_ctor(obj, name);
    }
};

// Function constructors
Ctor_MyCar    MyCar_ctor   (&Function_ctor, &Car_ctor,               Car_ctor.prototype);
Ctor_Car      Car_ctor     (&Function_ctor, &Vehicle_ctor,           Vehicle_ctor.prototype);
Ctor_Vehicle  Vehicle_ctor (&Function_ctor, Function_ctor.prototype, Object_ctor.prototype);

// Object instances
Car* car    = Car_ctor._new();
MyCar* mycar= MyCar_ctor._new();
Array* arr  = Array_ctor._new();
Object* obj = Object_ctor._new();

int main(int argc, char* argv[]) {
  // InitAllProtoTypeProps();

  Car_ctor(car, "Tesla");
  MyCar_ctor(mycar, "Tesla Model X");
  std::vector<Object*> objs = {obj, arr, &Array_ctor, &Vehicle_ctor, &Car_ctor, &MyCar_ctor, car, mycar, &Function_ctor, &Object_ctor};
  std::vector<std::string> names = {"obj", "arr", "Array", "Vehicle", "Car", "MyCar", "car", "mycar", "Function", "Object"};
  GenerateDOTGraph(objs, names);
  return 0;
}

