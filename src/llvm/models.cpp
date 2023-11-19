
#include "./models.h"

namespace eokas {
    llvm_type_t::llvm_type_t(llvm_module_t *module)
            : module(module), handle(nullptr) {
    }

    llvm_type_t::~llvm_type_t() {

    }

    bool llvm_type_t::is_struct_type() const {
        if(handle == nullptr)
            return false;
        return handle->isStructTy();
    }

    bool llvm_type_t::is_value_type() const {
        if(handle == nullptr)
            return false;
        return handle->isIntegerTy() || handle->isFloatingPointTy() || handle->isPointerTy();
    }

    bool llvm_type_t::is_reference_type() const {
        if(handle == nullptr)
            return false;
        return handle->isFunctionTy() || handle->isStructTy() || handle->isArrayTy();
    }

    llvm_type_struct_t::llvm_type_struct_t(llvm_module_t* module)
            : llvm_type_t(module), generics(), members() {
        this->handle = this->structHandle = llvm::StructType::create(module->context);
    }

    void llvm_type_struct_t::begin() {}

    void llvm_type_struct_t::body() {}

    void llvm_type_struct_t::end() {
        std::vector<llvm::Type*> structBody;
        for (auto& member: this->members) {
            structBody.push_back(member.type->handle);
        }
        this->structHandle->setBody(structBody);
    }

    bool llvm_type_struct_t::extends(const String& base) {
        auto type = module->get_type_symbol(base)->type;
        auto baseStruct = dynamic_cast<llvm_type_struct_t*>(type);
        if(baseStruct == nullptr)
            return false;
        return this->extends(baseStruct);
    }

    bool llvm_type_struct_t::extends(llvm_type_struct_t* base) {
        if (base == nullptr)
            return false;
        this->add_member("base", base);
        for (auto& m: base->members) {
            if (this->is_final_member(m.name))
                continue;
            if (this->add_member(&m) == nullptr)
                return false;
        }
        return true;
    }

    llvm_type_struct_t::member_t* llvm_type_struct_t::add_member(const String& name, llvm_type_t* type, llvm_value_t* value) {
        if (this->get_member(name) != nullptr)
            return nullptr;
        if (type == nullptr && value == nullptr)
            return nullptr;

        member_t& m = this->members.emplace_back();
        m.name = name;
        m.type = type;
        m.value = value;

        if (type == nullptr) {
            m.type = value->get_type();
        }

        return &m;
    }

    llvm_type_struct_t::member_t* llvm_type_struct_t::add_member(const String &name, llvm_value_t *value) {
        return this->add_member(name, nullptr, value);
    }

    llvm_type_struct_t::member_t* llvm_type_struct_t::add_member(const member_t* other) {
        return this->add_member(other->name, other->type, other->value);
    }

    llvm_type_struct_t::member_t* llvm_type_struct_t::get_member(const String& name) {
        for (auto& m: this->members) {
            if (m.name == name)
                return &m;
        }
        return nullptr;
    }

    llvm_type_struct_t::member_t* llvm_type_struct_t::get_member(size_t index) {
        if (index >= this->members.size())
            return nullptr;
        member_t& m = this->members.at(index);
        return &m;
    }

    size_t llvm_type_struct_t::get_member_index(const String& name) const {
        for (size_t index = 0; index < this->members.size(); index++) {
            if (this->members.at(index).name == name)
                return index;
        }
        return -1;
    }

    bool llvm_type_struct_t::is_final_member(const String& name) const {
        return name == "base";
    }

    void llvm_type_struct_t::resolve_generic_type(const std::vector<llvm::Type*>& args) {
        for (size_t index = 0; index < this->generics.size(); index++) {
            auto gen = llvm::cast<llvm::StructType>(this->generics[index]);
            auto arg = llvm::cast<llvm::StructType>(args[index]);
            this->resolve_opaque_type(gen, arg);
        }
    }

    void llvm_type_struct_t::resolve_opaque_type(llvm::StructType* opaqueT, llvm::StructType* structT) {
        std::vector<llvm::Type*> body;
        for (uint32_t index = 0; index < structT->getNumElements(); index++) {
            auto* elementT = structT->getElementType(index);
            body.push_back(elementT);
        }
        opaqueT->setBody(body);
    }

    llvm_value_t::llvm_value_t(llvm_module_t* module, llvm::Value* value)
            : module(module), value(value) {

    }

    llvm_type_t* llvm_value_t::get_type() {
        if(this->value == nullptr) {
            return nullptr;
        }
        llvm::Type* handle = this->value->getType();
        return this->module->get_type_symbol(handle)->type;
    }

    llvm_function_t::llvm_function_t(
            llvm_module_t* module,
            const String& name,
            llvm::Type* retT,
            const std::vector<llvm::Type*> argsT,
            bool varg)
            : llvm_value_t(module), IR(module->context) {
        this->type = llvm::FunctionType::get(retT, argsT, varg);
        this->handle = llvm::Function::Create(
                this->type,
                llvm::Function::ExternalLinkage,
                name.cstr(),
                &module->module);

        llvm::AttributeList attrs;
        this->handle->setAttributes(attrs);
        this->handle->setCallingConv(llvm::CallingConv::C);

        this->value = this->handle;
    }

    void llvm_function_t::begin() {}

    void llvm_function_t::body() {}

    void llvm_function_t::end() {}

    /**
     * For ref-types, transform multi-level pointer to one-level pointer.
     * For val-types, transform multi-level pointer to real value.
     * */
    llvm::Value* llvm_function_t::get_value(llvm::Value* value) {
        llvm::Type* type = value->getType();
        while (type->isPointerTy()) {
            if (llvm::isa<llvm::Function>(value))
                break;
            if (type->getPointerElementType()->isFunctionTy())
                break;
            if (type->getPointerElementType()->isStructTy())
                break;
            if (type->getPointerElementType()->isArrayTy())
                break;
            value = IR.CreateLoad(value);
            type = value->getType();
        }

        return value;
    }

    /**
     * Transform the multi-level pointer value to one-level pointer type value.
     * Ignores literal values.
     * */
    llvm::Value* llvm_function_t::ref_value(llvm::Value* value) {
        llvm::Type* type = value->getType();

        while (type->isPointerTy() && type->getPointerElementType()->isPointerTy()) {
            value = IR.CreateLoad(value);
            type = value->getType();
        }

        return value;
    }

    llvm::BasicBlock* llvm_function_t::add_basic_block(const String& name) {
        llvm::BasicBlock* bb = llvm::BasicBlock::Create(module->context, name.cstr(), this->handle);
        return bb;
    }

    void llvm_function_t::add_tail_ret() {
        auto& lastOp = IR.GetInsertBlock()->back();
        if (!lastOp.isTerminator()) {
            auto retT = this->handle->getReturnType();
            if (retT->isVoidTy())
                IR.CreateRetVoid();
            else
                IR.CreateRet(module->get_default_value(retT));
        }
    }

    bool llvm_function_t::is_array_type(llvm::Type* type) {
        if (!type->isPointerTy())
            return false;
        type = type->getPointerElementType();
        if (!type->isStructTy() || type->getStructNumElements() != 2)
            return false;

        auto dataT = type->getStructElementType(0);
        auto countT = type->getStructElementType(1);

        return dataT->isPointerTy()
               && dataT->getPointerElementType()->isArrayTy()
               && countT->isIntegerTy(64);
    }

    llvm::Value* llvm_function_t::make(llvm::Type* type) {
        auto mallocF = module->module.getFunction("malloc");
        llvm::Constant* len = llvm::ConstantExpr::getSizeOf(type);
        llvm::Value* ptr = IR.CreateCall(mallocF, {len});
        llvm::Value* val = IR.CreateBitCast(ptr, type->getPointerTo());
        return val;
    }

    llvm::Value* llvm_function_t::make(llvm::Type* type, llvm::Value* count) {
        auto mallocF = module->module.getFunction("malloc");
        llvm::Constant* stride = llvm::ConstantExpr::getSizeOf(type);
        llvm::Value* len = IR.CreateMul(stride, count);
        llvm::Value* ptr = IR.CreateCall(mallocF, {len});
        llvm::Value* val = IR.CreateBitCast(ptr, type->getPointerTo());
        return val;
    }

    llvm::Value* llvm_function_t::make(llvm_type_t* type) {
        auto ptr = this->make(type->handle);
        if(type->is_struct_type()) {
            auto struct_type = dynamic_cast<llvm_type_struct_t*>(type);
            for (size_t i = 0; i < struct_type->members.size(); i++) {
                auto mem = struct_type->get_member(i);
                auto p = IR.CreateStructGEP(ptr, i);
                auto v = mem->value->value;
                IR.CreateStore(v, p);
            }
        }
        return ptr;
    }

    void llvm_function_t::free(llvm::Value* ptr) {
        auto freeF = module->module.getFunction("free");
        IR.CreateCall(freeF, {ptr});
    }

    llvm::Value* llvm_function_t::array_set(llvm::Value* array, const llvm::ArrayRef<llvm::Value*>& elements) {
        auto elementT = elements[0]->getType();
        auto arrayT = llvm::ArrayType::get(elementT, elements.size());

        auto* dataP = IR.CreateStructGEP(array, 0);
        auto* dataV = this->make(arrayT);
        for (size_t i = 0; i < elements.size(); i++) {
            auto elementP = IR.CreateConstGEP2_64(dataV, 0, i);
            auto elementV = elements[i];
            IR.CreateStore(elementV, elementP);
        }
        IR.CreateStore(dataV, dataP);

        auto countP = IR.CreateStructGEP(array, 1);
        auto countV = IR.getInt64(elements.size());
        IR.CreateStore(countV, countP);

        return array;
    }

    llvm::Value* llvm_function_t::array_get(llvm::Value* array, llvm::Value* index) {
        auto dataP = IR.CreateStructGEP(array, 0);
        auto dataV = IR.CreateLoad(dataP);
        auto val = IR.CreateInBoundsGEP(dataV, {IR.getInt64(0), index});
        return val;
    }

    llvm::Value* llvm_function_t::cstr_len(llvm::Value* val) {
        llvm::Value* cstr = this->cstr_from_value(val);
        auto pfn = module->module.getFunction("strlen");
        llvm::Value* ret = IR.CreateCall(pfn, {cstr});

        return ret;
    }

    llvm::Value* llvm_function_t::cstr_from_value(llvm::Value* val) {
        llvm::Type* vt = val->getType();

        if (vt == module->type_cstr)
            return val;

        if (vt == module->type_string_ptr)
            return this->cstr_from_string(val);

        if (vt == module->type_bool)
            return this->cstr_from_bool(val);

        return this->cstr_from_number(val);
    }

    llvm::Value* llvm_function_t::cstr_from_string(llvm::Value* str) {
        llvm::Value* ptr = IR.CreateStructGEP(module->type_string, str, 0);
        llvm::Value* cstr = IR.CreateLoad(ptr);
        return cstr;
    }

    llvm::Value* llvm_function_t::cstr_from_number(llvm::Value* val) {
        llvm::Type* vt = val->getType();

        llvm::Value* buf = IR.CreateAlloca(llvm::ArrayType::get(module->type_i8, 64));

        llvm::StringRef vf = "%x";
        if (vt->isIntegerTy())
            vf = vt->getIntegerBitWidth() == 8 ? "%c" : "%d";
        else if (vt->isFloatingPointTy())
            vf = "%f";

        llvm::Value* fmt = IR.CreateGlobalString(vf);

        auto sprintf = module->module.getFunction("sprintf");
        IR.CreateCall(sprintf, {buf, fmt, val});

        return buf;
    }

    llvm::Value* llvm_function_t::cstr_from_bool(llvm::Value* val) {
        llvm::BasicBlock* branch_true = this->add_basic_block("branch.true");
        llvm::BasicBlock* branch_false = this->add_basic_block("branch.false");
        llvm::BasicBlock* branch_end = this->add_basic_block("branch.end");

        IR.CreateCondBr(val, branch_true, branch_false);

        IR.SetInsertPoint(branch_true);
        auto val_true = IR.CreateGlobalStringPtr("true");
        IR.CreateBr(branch_end);

        IR.SetInsertPoint(branch_false);
        auto val_false = IR.CreateGlobalStringPtr("false");
        IR.CreateBr(branch_end);

        IR.SetInsertPoint(branch_end);
        llvm::PHINode* phi = IR.CreatePHI(module->type_cstr, 2);
        phi->addIncoming(val_true, branch_true);
        phi->addIncoming(val_false, branch_false);

        return phi;
    }

    llvm::Value* llvm_function_t::string_make(const char* cstr) {
        auto str = this->make(module->type_string);

        auto dataP = IR.CreateStructGEP(str, 0);
        auto dataV = IR.CreateGlobalString(cstr);
        IR.CreateStore(dataV, dataP);

        auto lengthP = IR.CreateStructGEP(str, 1);
        auto lengthV = IR.getInt32(std::strlen(cstr));
        IR.CreateStore(lengthV, lengthP);

        return str;
    }

    llvm::Value* llvm_function_t::string_from_cstr(llvm::Value* cstr) {
        llvm::Value* str = this->make(module->type_string);

        llvm::Value* dataP = IR.CreateStructGEP(str, 0);
        IR.CreateStore(cstr, dataP);

        auto lengthP = IR.CreateStructGEP(str, 1);
        auto lengthV = this->cstr_len(cstr);
        IR.CreateStore(lengthV, lengthP);

        return str;
    }

    llvm::Value* llvm_function_t::string_from_value(llvm::Value* val) {
        llvm::Type* vt = val->getType();

        if (vt == module->type_string_ptr)
            return val;

        llvm::Value* cstr = nullptr;
        if (vt == module->type_cstr)
            cstr = val;
        else if (vt == module->type_bool)
            cstr = this->cstr_from_bool(val);
        else
            cstr = this->cstr_from_number(val);

        return this->string_from_cstr(cstr);
    }

    llvm::Value* llvm_function_t::string_get_char(llvm::Value* str, llvm::Value* index) {
        auto dataPtr = IR.CreateStructGEP(str, 0);
        return IR.CreateGEP(dataPtr, index);
    }

    llvm::Value* llvm_function_t::string_concat(llvm::Value* str1, llvm::Value* str2) {
        return nullptr;
    }

    llvm::Value* llvm_function_t::print(const std::vector<llvm::Value*>& args) {
        llvm::Value* val = args[0];
        llvm::Value* fmt = IR.CreateGlobalString("%s ");
        llvm::Value* cstr = this->cstr_from_value(val);
        auto pfn = module->module.getFunction("printf");
        llvm::Value* ret = IR.CreateCall(pfn, {fmt, cstr});

        return ret;
    }

    llvm_module_t::llvm_module_t(llvm::LLVMContext& context, const String& name)
            : context(context), module(name.cstr(), context), scope(new llvm_scope_t(nullptr, nullptr)), usings() {

        type_void = llvm::Type::getVoidTy(context);
        type_i8 = llvm::Type::getInt8Ty(context);
        type_i16 = llvm::Type::getInt16Ty(context);
        type_i32 = llvm::Type::getInt32Ty(context);
        type_i64 = llvm::Type::getInt64Ty(context);
        type_u8 = llvm::Type::getInt8Ty(context);
        type_u16 = llvm::Type::getInt16Ty(context);
        type_u32 = llvm::Type::getInt32Ty(context);
        type_u64 = llvm::Type::getInt64Ty(context);
        type_f32 = llvm::Type::getFloatTy(context);
        type_f64 = llvm::Type::getDoubleTy(context);
        type_bool = llvm::Type::getInt1Ty(context);
        type_cstr = type_i8->getPointerTo();
        type_void_ptr = type_void->getPointerTo();
    }

    llvm_module_t::~llvm_module_t() {
        _DeletePointer(scope);
    }

    void llvm_module_t::begin() {

    }

    void llvm_module_t::body() {}

    void llvm_module_t::end() {}

    void llvm_module_t::using_module(llvm_module_t* other) {
        if (other == this)
            return;
        for (auto* m: this->usings) {
            if (m == other)
                return;
        }
        this->usings.push_back(other);
    }

    bool llvm_module_t::add_type_symbol(const String& name, struct llvm_type_t* type) {
        if (type == nullptr || type->module != this)
            return false;
        if (!this->scope->add_type_symbol(name, type))
            return false;
        return type;
    }

    llvm_type_symbol_t* llvm_module_t::get_type_symbol(const String& name) {
        return this->scope->get_type_symbol(name, true);
    }

    llvm_type_symbol_t* llvm_module_t::get_type_symbol(llvm::Type *handle) {
        return this->scope->get_type_symbol(handle, true);
    }

    bool llvm_module_t::add_value_symbol(const String& name, struct llvm_value_t* value) {
        if (value == nullptr)
            return false;
        if (!this->scope->add_value_symbol(name, value))
            return false;
        return true;
    }

    llvm_value_symbol_t* llvm_module_t::get_value_symbol(const String& name) {
        return this->scope->get_value_symbol(name, true);
    }

    String llvm_module_t::get_type_name(llvm::Type* type) {
        if (type == type_i8) return "i8";
        if (type == type_i16) return "i16";
        if (type == type_i32) return "i32";
        if (type == type_i64) return "i64";

        if (type == type_u8) return "u8";
        if (type == type_u16) return "u16";
        if (type == type_u32) return "u32";
        if (type == type_u64) return "u64";

        if (type == type_f32) return "f32";
        if (type == type_f64) return "f64";

        if (type == type_bool) return "bool";
        if (type == type_cstr) return "cstr";

        if (type->isStructTy())
            return type->getStructName().data();

        if (type->isArrayTy())
            return String::format("Array<%s>", this->get_type_name(type->getArrayElementType()).cstr());

        if (type->isPointerTy())
            return String::format("Ptr<%s>", this->get_type_name(type->getPointerElementType()).cstr());

        if (type->isFunctionTy()) {
            auto* funcType = llvm::cast<llvm::FunctionType>(type);

            String params = "";
            uint32_t count = funcType->getFunctionNumParams();
            for (uint32_t index = 0; index < count; index++) {
                if (index == 0) {
                    params += ",";
                }
                auto ptype = funcType->getFunctionParamType(index);
                params += this->get_type_name(ptype);
            }

            String ret = this->get_type_name(funcType->getReturnType());

            return String::format("func(%s):%s", params.cstr(), ret.cstr());
        }

        return "";
    }

    llvm::Value* llvm_module_t::get_default_value(llvm::Type* type) {
        if (type->isIntegerTy()) {
            auto bits = type->getIntegerBitWidth();
            return llvm::ConstantInt::get(context, llvm::APInt(bits, 0));
        }
        if (type->isFloatingPointTy()) {
            return llvm::ConstantFP::get(context, llvm::APFloat(0.0f));
        }
        return llvm::ConstantPointerNull::get(llvm::Type::getVoidTy(context)->getPointerTo());
    }

}
