
## Eokas 开发日志


## 2024/05/15
#### 当前状态
* 使用 using/public/private 来实现模块成员的导入导出可以比 import/export/public/private 节省一个关键字.
* 每个模块的 root-scope 下定义两个新的 scope： 
  * exports-scope：如果本模块中某个类型和变量对外部可见，就将其放入 exports-scope 中。
  * imports-scope：导入一个模块，就将其 exports-scope 中的所有类型和值放入本模块的 imports-scope 中。
> 目前还无法确认将其他模块的符号导入本模块之后，在链接阶段是否会引起错误。
* parser 中 parse_import 与 parse_export 需要仔细思考，在模块导入导出方案确定之后，这个需要优先实现。
* x-module-coder 中，还没有实现模块导入导出方案。
* struc 类型的 resolve 还没有实现。
#### 后续工作
* 将 i32 这样的与定义类型定义到 core 模块中，需要考虑如何定义 i32 等类型为外部可访问。
* 通过引入core （core 是 coder 默认引入的），在测试代码模块中，能够正确搜索到 i32 的类型定义。


## 2024/03/01
* 使用 omis（Object Model Interation System，对象模型交互系统）来抽象低层对象，可以让 coder 的代码更加通用。
* 将 llvm 指令限定到 bridge 中，可以更加方便的升级 llvm 版本或替换掉 llvm 而使用其他低层后端框架。
