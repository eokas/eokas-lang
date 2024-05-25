
## Eokas 开发日志

## 2024/05/16
#### 当前进展
* Scanner 解析 import/export 关键字。
* Parser 将 import/export 解析为 ast。
* omis_module_coder 将 import/export 从 ast 节点翻译成 omis 实现。
* omis_module 的 using_module 将依赖项导入到 usings 中，并将依赖中的 exports scope 中的符号插入到本模块的 imports scope 中。
* omis_module 的 get_type_symbol 方法提供了从 imports scope 中查找符号。

#### 注意事项
* 目前 export 只支持导出 type symbol，但 import 已经支持将 dependencty 中的 type-symbol 与 value-symbol 一起导入。
* 目前 module 的 name 从哪里来，目前有两种方案：
  * 使用文件名称作为 module name。这样的问题是一个文件只能定义一个 module，一个 module 只能定义在一个文件中。import 后面接文件路径。
  * 设计专门的 module 语法结构。文件名称和 module 名称可以不一样，一个文件可以定义多个 module。也可以实现将一个 module 定义到多个文件中（类似于 C# 的 partial class）。

#### 后续计划
* 测试当前版本下的 import/export 特性。
* 支持更加优化的 import/export 语法。


## 2024/05/15
#### 当前进展
* 使用 using/public/private 来实现模块成员的导入导出可以比 import/export/public/private 节省一个关键字.
* 每个模块的 root-scope 下定义两个新的 scope： 
  * exports-scope：如果本模块中某个类型和变量对外部可见，就将其放入 exports-scope 中。
  * imports-scope：导入一个模块，就将其 exports-scope 中的所有类型和值放入本模块的 imports-scope 中。
> 目前还无法确认将其他模块的符号导入本模块之后，在链接阶段是否会引起错误。
* parser 中 parse_import 与 parse_export 需要仔细思考，在模块导入导出方案确定之后，这个需要优先实现。
* x-module-coder 中，还没有实现模块导入导出方案。
* struc 类型的 resolve 还没有实现。
#### 后续计划
* 将 i32 这样的与定义类型定义到 core 模块中，需要考虑如何定义 i32 等类型为外部可访问。
* 通过引入core （core 是 coder 默认引入的），在测试代码模块中，能够正确搜索到 i32 的类型定义。


## 2024/03/01
* 使用 omis（Object Model Interation System，对象模型交互系统）来抽象低层对象，可以让 coder 的代码更加通用。
* 将 llvm 指令限定到 bridge 中，可以更加方便的升级 llvm 版本或替换掉 llvm 而使用其他低层后端框架。
