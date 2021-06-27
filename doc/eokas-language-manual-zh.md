# eokas 编程序言指南


## 代码注释

```eokas
// 这里是单行注释

/*
    多行注释
    多行注释
*/
```

## 基础类型与字面量

基础类型是预定义的，可以直接使用这些类型。

``` eokas
i8, i16, i32, i64,
u8, u16, u32, u64,
f32, f64,
bool,
string,
```

字面量是直接的值，无论什么类型都有属于自己的字面量。下面是 eokas 中一些基础类型的字面量：

```eokas
十进制整数: 1, 2, 3, ... 9, 10, ... 99, 100, ...
二进制整数: 0b0, 0b1, 0b1001, 0b0010
十六进制整数: 0x1, 0x2, ... 0x9, 0xA, ..., 0xF, ...
浮点数字: 3.14, 0.14e2
布尔值: true, false
字符串: "hello world."

空值: null
```

## 常量与变量

### 1、定义

常量在定义时同时被初始化，并在初始化之后就无法被重新赋值的量，定义常量使用关键字 ’val‘。变量在运行时可以被随意修改，定义变量使用关键字 ’var‘。

定义常量和变量的完整语法如下：
```eokas
// 定义常量
val pi : f64 = 3.141592653;
val sqrt2 f64 = 1.414;

// 定义变量
var a : u8 = 127;
val b : bool = false;
```

### 2、类型推导

使用编译器类型推导，可以不用显示指定常量或变量的类型，但必须给出初始值，编译器会根据初始值自动推导其类型。

```eokas
// 定义变量或常量时使用类型推导。
var x = 4;
val f = 1.414;
```

### 3、作用域

作用域是常量或变量的影响范围，用代码块 {} 来显示定义一个作用域（很多语句也会包含自身隐含的作用域）。常量和变量的生命周期是从定义时开始到所在的作用域结尾。因此内层作用域可以访问外层作用域定义的常量和变量，外层作用域无法访问内层作用域的常量和变量。

```eokas
val x = 10;
{
    val x = 9;
    {
        val x = 8;
        val y = 8;
    }
    val y = 9;
}
val y = 10;
```

## 运算符与表达式

### 1、数据访问

```eokas
// 变量访问
a

// 数组访问
a[0]

// 对象访问
a.name

// 函数调用
f(10)
```

### 2、算术运算

```eokas
// 算术运算
3.14159 * (5 + 1) * (5 + 1)
```

### 3、逻辑运算

```eokas
// 逻辑运算
a || b         // 如 a 为真就直接返回 a，否则返回 b。
a && b         // 如 a 为假就直接返回 a，否则返回 b。
a && b || c    // 如 a 为真就返回 b，否则返回 c。
!a             // 非 a
```

### 4、关系运算

```eokas
// 关系运算
a > b
a >= b
a < b
a <= b
a == b
a != b
```

### 3、位运算

```eokas
// 位运算
~a
a | b
a & b
a ^ b
a << 2
a >> 2
```

## 数组

数组类型描述了数据的连续存取规范。简单来讲数组描述了在内存中的一段连续的空间，这段空间被分成 n 个大小相等的被称为元素的格子，每个元素的按照设么类型进行读写。

```eokas
// 定义一个有十个元素数组，每个元素都是一个 int 类型的变量。
val list: int[10] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];

// 使用数组中的元素。
val a = list[9]; // a 的类型是 int，值是 9。
```

## 结构

描述了数据的结构化存取规范。使用结构类型可以给予数据更多符合人类认知方式的描述方法，让数据组织更加符合人脑的直观认知。结构是一种自定义的类型，一个结构可以包含多个成员（每个成员是一个常量或变量）。成员在被定义的时候可以为其赋值初始化。定义结构使用关键字 ‘struct’。

```eokas
struct O {
    val type: int = 1;
    var name: string;
    var code: int;
};
```

使用结构类型定义变量和常量，注意每个成员在定义时就制定了是常量还是变量，如果某个结构成员是一个常量，则必须在定义结构的时候就初始化，在定义变量时不能再对常量成员赋值了。

```eokas
val o = O {
    type = 0;  // error
    name = "";
    code = 0;
};
```

## 模式

模式是 eokas 类型定义需要遵循的规范。相当于其他语言的接口（interface）的概念。模式可以被结构或其他模式遵循，如果一个模式或结构类型遵循了一个模式，则这个模式或结构类型必须包含所遵循模式中的所有成员。模式成员仅仅作为其他模式或结构所遵循的规范，不能在定义时被赋值初始化。定义时不能被赋值初始化。定义模式使用关键字 ‘schema’。

```eokas
// 定义一个模式
schema I {
    var x: int;
};

// 定义一个遵循模式 I 的新模式。
schema A : I {
    var x: int;
    var y: int;
}

// 定义一个遵循模式 A 的结构。
struct B : I {
    var x: int = 0;
    var y: int = 0;
};
```

## 过程与函数

### 1、过程类型

过程（Process）描述了数据的变换规范。数据通过过程从一种状态映射到另一种状态，例如从用户点击操作映射到硬件机器的某个行为。过程类型主要是描述了这样的变换规范，但不包含具体的变换逻辑。

在 eokas 中是所有可调用对象的抽象。过程可以是一个函数（func）、一个任务（task）、一个重载了函数调用运算符 ’operator()‘ 的对象。使用关键字 ‘proc’ 来定义过程，具体格式如下：

```eokas
// 定义过程
proc Compute(a: int, b: int): int;
```

### 2、函数定义与调用

根据过程，我们可以定义具体的函数常量或变量（可以使用类型推导来直接定义函数）。函数定义使用关键字 ‘func’。

```eokas
// 定义符合过程类型约束的函数常量。
val add : Compute = func(a: int, b: int) : int {
    return a + b;
};

// 不显示指定类型，使用类型推导，直接定义函数常量。
val f = func(a: int, b: int): int {
    return a + b;
};

// 调用函数
val a = add(1, 2);
val b = f(1, 2);
var c = add(a, b);
```

### 3、递归

递归是函数自己调用自己，或者几个函数之间循环调用。self 关键字代表函数自己，这样能够非常方便的实现匿名函数递归。

```eokas
val fib = func(n: int): int {
		if(n == 1 || n == 2) return 1;
		return self(n-1) + self(n-2);
};
```

### 4、闭包

闭包可用让内存函数访问外层函数的局部变量。

```
val iter = func(list: List): func(): int {
    var index = 0;
    return func() {
        return list[index++];
    }
}
```

## 泛型

### 1、定义与使用

泛型是使用参数化的类型进行高度的抽象和代码复用的程序设计方法。在 eokas 中，泛型使用尖括号和类型参数来描述。具体定义方法如下：

```eokas
// 定义一个泛型模式
schema I<T> {
		var x : T;
};

// 定义一个泛型结构。
struct Params<T> {
    var a: T;
    var b: T;
};

// 定义一个泛型过程。
proc Compute<T>(a: T, b: T): T;
```

使用泛型定义常量、变量，是将具体的类型传递给泛型的类型参数。

```eokas
// 根据泛型结构定义一个变量。
var p = Params<float> {
    a = 1.0;
    b = 2.0;
};

// 根据过程类型定义一个泛型函数。
val add : Compute<T> = func<T>(a: T, b: T): float {
    return a + b;
};

// 调用泛型函数
val x = add<int>(1, 2); // x 是 int 类型。
val y = add<float>(1.0, 2.0); // y 是 float 类型。
```

### 2、泛型的类型约束

泛型的类型约束是指类型参数必须符合某种规范。在 eokas 中使用模式进行泛型的类型约束。

```eokas
// 定义一个模式 C。
schema C {
    var x: int;
};

// 定义一个泛型模式 A，使用 C 作为类型 T 的约束，T 必须遵循 C。
schema A<T: C> {
    var a: T;
};

// 定义一个泛型结构 B，使用 C 作为类型 T 的约束，T 必须遵循 C。
struct B<T: C> {
    var b: T;
};

// 定义一个遵循模式 A 的泛型结构 X，其中的类型参数 T 必须遵循 C。
// T 如果不遵循 C 会报错，因为在上面 A 的定义中，T 是遵循 C 的。
struct X<T: C> : A<T> {
    var a: T;
};
```

## 错误处理

eokas 使用 try...catch、throw 来进行错误处理。

```eokas
schema Error {
		var name: string;
		var code: int;
		var message: string;
};

struct RuntimeError : Error {
    var name: string = "RuntimeError";
    var code: int = 0;
    var message: string;
};

try {
    val a = doSomething();
    val b = doNothing();
    if(a == 0 || b == 0) {
        throw Error{code = 0xFF, message = 'exception'};
    }
}
catch (err: Error) {
    ...
}
```

## 模块

* 模块中可以封装常量、变量、类型。
* export 可以让外部代码访问到模块中的对象。没有被 export 的模块内部对象将不能被外部代码访问。

```eokas
module my.math {

    export val pi = 3.1415926;

    export type Vec3 = struct{
        var x: float;
        var y: float;
        var z: float;
    };

    export val add = func(a: Vec3, b: Vec3): Vec3 {
        return Vec3 {
            x = a.x + b.x,
            y = a.y + b.y,
            z = a.z + b.z,
        };
    };
}
```

* import 关键字可以导入一个模块中的对象。

```eokas
// 将 my.math 模块中的所有内容全部导入到当前模块
import my.math;

// 将 my.math 模块的所有内容导入到 math 模块中。
import math = my.math;
```
